#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <concurrent_priority_queue.h>
#include <queue>
#include <chrono>
#include <cmath>
#include <algorithm>

#include "..\..\shared\protocol.hpp"
#include "server_config.hpp"
#include "world_map.hpp"

using namespace std;

#include "include/lua.hpp"
#include <concurrent_unordered_map.h>

#include <sqlext.h>  
#include <locale.h>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "lua54.lib")

// 전역변수들 ========================================================

HANDLE g_hiocp;
SOCKET gServerSocket, gCliSocket;

// SESSION 부분 ============================================================

enum COMP_TYPE {
	OP_ACCEPT, OP_RECV, OP_SEND, OP_PLAYER_MOVE, OP_NPC_RUN, OP_LOGIN_OK, OP_LOGIN_FAIL,
	OP_MONSTER_MOVE, OP_MONSTER_CHASE, OP_MONSTER_ATTACK_TO_USER, OP_USER_ATTACK_TO_MONSTER
};
enum STATE { ST_FREE, ST_ALLOC, ST_INGAME };

class OVER_EXP {
public:
	WSAOVERLAPPED		_over;
	WSABUF				_wasBuf;
	char				_sendBuf[BUF_SIZE];

	COMP_TYPE			_compType;
	int					_playerId;				// npc가 반응할 대상(플레이어) id 저장용

	OVER_EXP()
	{
		_wasBuf.len = BUF_SIZE;
		_wasBuf.buf = _sendBuf;
		_compType = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}

	OVER_EXP(char* packet)
	{
		_wasBuf.len = packet[0];
		_wasBuf.buf = _sendBuf;
		ZeroMemory(&_over, sizeof(_over));
		_compType = OP_SEND;
		memcpy(_sendBuf, packet, _wasBuf.len);
	}
};

OVER_EXP gOverExp;

//////////////////////////////// a*알고리즘 

struct Node {
	int x, y;
	int gCost, hCost;
	Node* parent;

	Node(int x_, int y_, int g_, int h_, Node* p)
		: x(x_), y(y_), gCost(g_), hCost(h_), parent(p) {}

	int fCost() const { return gCost + hCost; }
	bool operator>(const Node& other) const { return fCost() > other.fCost(); }
};

struct NodeComp {
	bool operator()(const Node* a, const Node* b) const {
		return a->fCost() > b->fCost();
	}
};

bool isValid(int x, int y) {
	return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT && map_data[y][x] != 3;
}

int heuristic(int x1, int y1, int x2, int y2) {	// 휴리스틱 
	return abs(x1 - x2) + abs(y1 - y2);
}

std::vector<std::pair<int, int>> findPathAStar(int startX, int startY, int goalX, int goalY) {
	std::priority_queue<Node*, std::vector<Node*>, NodeComp> openList;
	std::unordered_set<long long> closedSet;

	auto hash = [](int x, int y) -> long long { return ((long long)y << 32) | x; };

	Node* start = new Node(startX, startY, 0, heuristic(startX, startY, goalX, goalY), nullptr);
	openList.push(start);

	while (!openList.empty()) {
		Node* current = openList.top();
		openList.pop();

		if (current->x == goalX && current->y == goalY) {
			std::vector<std::pair<int, int>> path;
			while (current) {
				path.emplace_back(current->x, current->y);
				current = current->parent;
			}
			std::reverse(path.begin(), path.end());
			return path;
		}

		closedSet.insert(hash(current->x, current->y));

		const int dx[4] = { 0, 0, -1, 1 };
		const int dy[4] = { -1, 1, 0, 0 };

		for (int dir = 0; dir < 4; ++dir) {
			int nx = current->x + dx[dir];
			int ny = current->y + dy[dir];

			if (!isValid(nx, ny)) continue;
			if (closedSet.count(hash(nx, ny))) continue;

			int gCost = current->gCost + 1;
			int hCost = heuristic(nx, ny, goalX, goalY);
			Node* neighbor = new Node(nx, ny, gCost, hCost, current);
			openList.push(neighbor);
		}
	}
	return {};
}


class GAMESESSION {
public:
	OVER_EXP			_recvOver;
	SOCKET				_socket;

	int					_id;
	STATE				_state;
	short				_x, _y;
	short				_hp, _maxHp, _level, _exp, _seedNum; // hp, 최대hp, 레벨, 경험치
	short				_damage;
	char				_name[NAME_SIZE];
	int					_prevRemainData;

	unordered_set<int>	_viewList;	// monster는 viewlist 안가진다
	mutex				_vlL;

	lua_State* _L;	// 각 session 객체가 들고있는 _L 이라는 자기만의 루아상태를 가짐
	// 이러면 각 npc든 플레이어든.. 자기만의 _L가짐
	// 그러면 npc마다 다른 행동패턴을 가지게 할 수 있음..!
	// npc.lua에서 처음에 제일 상단에 있는.. 전역변수들로 선언된거 가짐
	// move_cnt, dir, myid 다 따로 가지게되능거ㅇㅇ
	mutex				_ll;

	// monster 관련
	atomic<bool>		_isActive;
	atomic<bool>		_isMonsterChase;
	atomic<bool>		_isDie;
	int					_monType; // 0 이면 플레이어고.. .1,2면 몬스터중 하나
	short				_nextMonX, _nextMonY;

	int					_userInputId;
public:
	GAMESESSION()
	{
		_id = -1;
		_socket = 0;
		_x = _y = 0;
		_name[0] = 0;
		_state = ST_FREE;
		_prevRemainData = 0;
	}

	~GAMESESSION() {};

	void registerRecv();
	void registerSend(void*);

	void sendSessionLoginPacket();
	void sendSessioLoginOkPacket();
	void sendMovePacket(int);
	void sendAddPlayerPacket(int);
	void sendRemoveSessionPacket(int);
	void sendChatPacket(int, const char*);
	void sendAttackPacket_PtoN(int, int);		// PlayertoMon
	void sendAttackPacket_NtoP(int);		// MontoPlayer
	void sendMonsterDiePacket(int);
	void sendMonsterNearPacket(nearMonsterInfo*, int);
	void sendMapInfo(int, int, int);
	void sendStatChange(int);

	// sector
	void updateSector(short, short);
	std::unordered_set<int> getNearBySessions();
};

concurrency::concurrent_unordered_map<long long, std::shared_ptr<GAMESESSION>> clients;

// ============================================================

bool isPlayer(int sessionId) {
	return sessionId < MAX_USER;
}

bool isMonster(int sessionId) {
	return !isPlayer(sessionId);
}

int getNewPlayerId() {
	for (int i = 0; i < MAX_USER; ++i) {
		auto it = clients.find(i);
		if (it == clients.end())  // i번 슬롯이 비어있으면 사용 가능
			return i;

		std::shared_ptr<GAMESESSION> new_user = it->second;
		if (new_user && new_user->_state == ST_FREE)
			return i;
	}
	return -1;
}

bool isCanSee(int from, int to)
{
	auto from_it = clients.find(from);
	std::shared_ptr<GAMESESSION> fromUser = from_it->second;

	auto to_it = clients.find(to);
	std::shared_ptr<GAMESESSION> toUser = to_it->second;

	if (abs(fromUser->_x - toUser->_x) > VIEW_RANGE)
		return false;
	return abs(fromUser->_y - toUser->_y) <= VIEW_RANGE;
}

bool isMonSee(int from, int to)
{
	auto from_it = clients.find(from);
	std::shared_ptr<GAMESESSION> fromUser = from_it->second;

	auto to_it = clients.find(to);
	std::shared_ptr<GAMESESSION> toUser = to_it->second;

	if (abs(fromUser->_x - toUser->_x) > 5)
		return false;
	return abs(fromUser->_y - toUser->_y) <= 5;
}


void initializeMap() {
	std::cout << " 지형정보 업뎃중.. " << std::endl;
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			int temp = rand() % 100;
			if (temp >= 0 and temp <= 35) map_data[y][x] = 0;		// 풀
			else if (temp > 35 and temp <= 87) map_data[y][x] = 1;	// 땅
			else if (temp > 87 and temp <= 88) map_data[y][x] = 2;  // 꽃
			else if (temp > 88 and temp <= 94) map_data[y][x] = 3;	// 장애물
			else if (temp > 94 and temp <= 97) map_data[y][x] = 4;	// 씨앗
			else if (temp > 97 and temp <= 98) map_data[y][x] = 5;	// 똥
			else if (temp > 98 and temp < 100) map_data[y][x] = 6;	// hp포션

			if (x == 0)map_data[y][x] = 7;
			if (y == 0)map_data[y][x] = 10;
			if (x == MAP_WIDTH - 1) map_data[y][x] = 12;
			if (y == MAP_HEIGHT - 1) map_data[y][x] = 14;
		}
	}
	// 맵 상하좌우끝부분
	map_data[0][0] = 9;
	map_data[0][MAP_WIDTH - 1] = 11;
	map_data[MAP_WIDTH - 1][0] = 13;
	map_data[MAP_WIDTH - 1][MAP_WIDTH - 1] = 15;

	std::cout << " 지형정보 업뎃 완료!! " << std::endl;
}


// timer ===============================================================
enum EVENT_TYPE { EV_RANDOM_MOVE, EV_RUN, EV_CHASE };	// EV_RUN -> 이게 NPC가 3칸이동하게하는 

struct TIMER_EVENT {
	int obj_id;				// 이 타이머 이벤트 수행할 객체ID(player또는 npc)
	chrono::high_resolution_clock::time_point wakeup_time;	// 이벤트 실행되는 시간
	EVENT_TYPE event_id;		// 이 이벤트 종류
	int target_id;				// 이벤트 대상 id

	constexpr bool operator < (const TIMER_EVENT& L) const
	{
		return (wakeup_time > L.wakeup_time);
	}
};

std::priority_queue<TIMER_EVENT> timerQueues[TIMER_THREAD_COUNT];
std::mutex timerLocks[TIMER_THREAD_COUNT];

int get_timer_queue_index(int id) {
	return id % TIMER_THREAD_COUNT;
}

// sector
std::unordered_set<int> sectors[MAX_SECTOR_X][MAX_SECTOR_Y];
std::mutex sectorLock[MAX_SECTOR_X][MAX_SECTOR_Y];

std::pair<int, int> getSectorLocation(int x, int y) {
	return { x / SECTOR_SIZE, y / SECTOR_SIZE };
}

// DB용 =============================================================
SQLHENV henv = nullptr;
extern SQLHDBC hdbc = nullptr;
void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];

	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

bool initializeDB() {
	SQLRETURN ret;

	ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) return false;

	ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) return false;

	ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) return false;

	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

	ret = SQLConnect(hdbc, (SQLWCHAR*)L"2025_gameserver_2023192003", SQL_NTS, NULL, 0, NULL, 0);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) return false;

	//std::cout << "[DB] 연결 성공!" << std::endl;
	return true;
}

void finalizeDB() {
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

// 로그인할때 정보 확인용
bool get_DB_login_info(int& db_id, int& db_user_x, int& db_user_y, int& db_user_hp,
	int& db_user_level, int& db_user_exp, int& db_user_seed_num, int& db_user_maxHp, int& db_user_damage) {
	SQLHSTMT hstmt = 0;
	SQLRETURN ret;
	SQLINTEGER user_id, user_x, user_y, user_hp, user_level, user_exp, user_seed, user_maxHp, user_damage;
	SQLLEN cb1 = 0, cb2 = 0, cb3 = 0, cb4 = 0, cb5 = 0, cb6 = 0, cb7 = 0, cb8 = 0, cb9 = 0;

	ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	ret = SQLPrepare(hstmt, (SQLWCHAR*)L"EXEC get_user_info ?", SQL_NTS);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		//finalizeDB();
		return false;
	}
	ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &db_id, 0, NULL);
	ret = SQLExecute(hstmt);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		//finalizeDB();
		return false;
	}

	SQLBindCol(hstmt, 1, SQL_C_LONG, &user_id, 0, &cb1);
	SQLBindCol(hstmt, 2, SQL_C_LONG, &user_x, 0, &cb2);
	SQLBindCol(hstmt, 3, SQL_C_LONG, &user_y, 0, &cb3);
	SQLBindCol(hstmt, 4, SQL_C_LONG, &user_hp, 0, &cb4);
	SQLBindCol(hstmt, 5, SQL_C_LONG, &user_level, 0, &cb5);
	SQLBindCol(hstmt, 6, SQL_C_LONG, &user_exp, 0, &cb6);
	SQLBindCol(hstmt, 7, SQL_C_LONG, &user_seed, 0, &cb7);
	SQLBindCol(hstmt, 8, SQL_C_LONG, &user_maxHp, 0, &cb8);
	SQLBindCol(hstmt, 9, SQL_C_LONG, &user_damage, 0, &cb9);


	if (SQLFetch(hstmt) == SQL_SUCCESS) {
		db_id = user_id;			// db에 저장된걸 꺼내서 저장해준다..!
		db_user_x = user_x;
		db_user_y = user_y;
		db_user_hp = user_hp;
		db_user_level = user_level;
		db_user_exp = user_exp;
		db_user_seed_num = user_seed;
		db_user_maxHp = user_maxHp;
		db_user_damage = user_damage;

		/*printf("ID: %d 접속! 위치: (%d, %d) HP : %d, Level : %d, Exp : %d, SeedNum : %d, maxHp : %d, damage : %d\n",
			user_id, user_x, user_y, user_hp, user_level, user_exp, user_seed, user_maxHp, user_damage);*/
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return true;
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return false;
}

// db업뎃용
bool update_user_info(int id, int x, int y, int seed, int hp, int level, int exp, int userMaxHp, int damage) {
	SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK);  // 이전 트랜잭션 실패 가능성 대비

	SQLHSTMT hstmt;
	SQLRETURN ret;

	// 1. Statement 핸들 할당
	ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);


	ret = SQLPrepare(hstmt, (SQLWCHAR*)L"UPDATE SERVER_TABLE1 SET USER_X = ?, USER_Y = ? , USER_SEED_NUM = ?, USER_HP = ?, USER_LEVEL = ?, USER_EXP = ?, USER_MAX_HP = ? WHERE USER_ID = ?", SQL_NTS);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		std::cout << "[DB] SQLPrepare 실패!" << std::endl;
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, ret);  // 에러 로그 추가!
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	// 3. 파라미터 바인딩
	ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &x, 0, NULL);
	ret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &y, 0, NULL);
	ret = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &seed, 0, NULL);
	ret = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &hp, 0, NULL);
	ret = SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &level, 0, NULL);
	ret = SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &exp, 0, NULL);
	ret = SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userMaxHp, 0, NULL);
	ret = SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &id, 0, NULL);


	// 4. 실행
	ret = SQLExecute(hstmt);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		std::cout << "[DB] SQLExecute 실패!" << std::endl;
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, ret);  // 에러 로그 추가!
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

//	std::cout << "유저 정보 업데이트 완료: ID=" << id << " → (" << x << "," << y << ") seed: " << seed << std::endl;

	// 5. 커밋 (안 해도 기본 auto-commit라면 필요 X, 하지만 넣어주자)
	SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);

	// 6. 해제
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return true;
}

bool insert_new_user(int userId, const char* name) {
	SQLHSTMT hstmt;
	SQLRETURN ret;

	int init_x = rand() % 2000;
	int init_y = rand() % 2000;
	int init_hp = rand() % 10 + 20;
	int init_level = 1;
	int init_exp = 0;
	int init_seed = 22;
	int init_maxHp = 30;
	int init_damage = 1;

	ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) return false;

	ret = SQLPrepare(hstmt, (SQLWCHAR*)L"INSERT INTO SERVER_TABLE1 (USER_ID, USER_X, USER_Y, USER_HP, USER_LEVEL, USER_EXP, USER_SEED_NUM, USER_MAX_HP, USER_DAMAGE) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", SQL_NTS);

	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userId, 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_x, 0, NULL);
	SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_y, 0, NULL);
	SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_hp, 0, NULL);
	SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_level, 0, NULL);
	SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_exp, 0, NULL);
	SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_seed, 0, NULL);
	SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_maxHp, 0, NULL);
	SQLBindParameter(hstmt, 9, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_damage, 0, NULL);

	ret = SQLExecute(hstmt);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, ret);  // 디버깅용 로그
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	std::cout << "[DB] 새 유저 추가 성공! ID:" << userId << std::endl;
	SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return true;
}

void GAMESESSION::registerRecv() {
	DWORD recv_flag = 0;
	memset(&_recvOver._over, 0, sizeof(_recvOver._over));
	_recvOver._wasBuf.len = BUF_SIZE - _prevRemainData;
	_recvOver._wasBuf.buf = _recvOver._sendBuf + _prevRemainData;
	WSARecv(_socket, &_recvOver._wasBuf, 1, 0, &recv_flag, &_recvOver._over, 0);
}

void GAMESESSION::registerSend(void* packet) {
	OVER_EXP* sessionData = new OVER_EXP{ reinterpret_cast<char*>(packet)};
	WSASend(_socket, &sessionData->_wasBuf, 1, 0, 0, &sessionData->_over, 0);
}

void GAMESESSION::sendSessionLoginPacket() {
	sc_packet_avatar_info p;
	p.id = _id;
	p.size = sizeof(sc_packet_avatar_info);
	p.type = S2C_P_AVATAR_INFO;
	p.x = _x;
	p.y = _y;
	p.hp = _hp;
	p.level = _level;
	p.exp = _exp;
	p.max_hp = _maxHp;
	p.seedNum = _seedNum;
	registerSend(&p);
}

void GAMESESSION::sendSessioLoginOkPacket() {
	sc_packet_avatar_login_ok p;
	p.id = _id;
	p.size = sizeof(sc_packet_avatar_login_ok);
	p.type = S2C_P_AVATAR_LOGIN_OK;
	registerSend(&p);
}

void GAMESESSION::sendMovePacket(int sessionId) {
	auto it = clients.find(sessionId);
	std::shared_ptr<GAMESESSION> user = it->second;

	sc_packet_move p;
	p.id = sessionId;
	p.size = sizeof(sc_packet_move);
	p.type = S2C_P_MOVE;
	p.x = user->_x;
	p.y = user->_y;
	registerSend(&p);
}

void GAMESESSION::sendAddPlayerPacket(int sessionId) {
	auto it = clients.find(sessionId);
	std::shared_ptr<GAMESESSION> user = it->second;

	sc_packet_enter p;
	p.id = sessionId;
	sprintf_s(p.name, NAME_SIZE, "%s", user->_name);

	p.size = sizeof(p);
	p.type = S2C_P_ENTER;
	p.monType = user->_monType; 
	p.x = user->_x;
	p.y = user->_y;
	_vlL.lock();
	_viewList.insert(sessionId);
	_vlL.unlock();
	registerSend(&p);
}

void GAMESESSION::sendRemoveSessionPacket(int sessionId) {
	auto it = clients.find(sessionId);
	std::shared_ptr<GAMESESSION> user = it->second;

	_vlL.lock();
	if (_viewList.count(sessionId))
		_viewList.erase(sessionId);
	else {
		_vlL.unlock();
		return;
	}
	_vlL.unlock();

	sc_packet_leave p;
	p.id = sessionId;
	p.size = sizeof(p);
	p.type = S2C_P_LEAVE;
	registerSend(&p);
}

void GAMESESSION::sendChatPacket(int sessionId, const char* message) {
	sc_packet_chat p;
	p.id = sessionId;
	p.size = sizeof(p);
	p.type = S2C_P_CHAT;
	//strcpy_s(p.message, message);
	
	strncpy_s(p.message, message, MAX_CHAT_LENGTH - 1);
	p.message[MAX_CHAT_LENGTH - 1] = '\0';
	registerSend(&p);
}

void GAMESESSION::sendAttackPacket_PtoN(int targetId, int targetNewHp) {
	auto it = clients.find(targetId);
	std::shared_ptr<GAMESESSION> monster = it->second;

	sc_packet_attack_success p;
	p.size = sizeof(p);
	p.type = S2C_P_ATTACK_PtoN;

	p.targetId = targetId;
	p.targetHp = targetNewHp;

	p.attackerId = _id;
	p.seedNum = _seedNum;
	registerSend(&p);
}

void GAMESESSION::sendAttackPacket_NtoP(int sessionId) {
	auto it = clients.find(sessionId);
	std::shared_ptr<GAMESESSION> session = it->second;
}

void GAMESESSION::sendMonsterDiePacket(int targetId) {
	sc_packet_monster_die p;
	p.size = sizeof(p);
	p.type = S2C_P_MONSTER_DIE;
	p.id = targetId;
	registerSend(&p);
}

void GAMESESSION::sendMonsterNearPacket(nearMonsterInfo* infos, int cnt) {
	sc_packet_monster_near p;
	p.type = S2C_P_MONSTER_NEAR;
	p.monCount = cnt;
	p.monType = _monType;
	memcpy(p.monsters, infos, sizeof(nearMonsterInfo) * cnt);
	p.size = sizeof(p) - sizeof(nearMonsterInfo) * (5 - cnt);  // 정확한 size 계산
	registerSend(&p);
}

void GAMESESSION::sendMapInfo(int player_id, int x, int y) {
	int left_x = x - SCREEN_WIDTH / 2;
	int top_y = y - SCREEN_HEIGHT / 2;

	const int MAP_ROW_CHUNK = 4;
	for (int chunk_y = 0; chunk_y < SCREEN_HEIGHT; chunk_y += MAP_ROW_CHUNK) {
		sc_packet_mapInfo* p = new sc_packet_mapInfo;
		p->type = S2C_P_MAPINFO;
		p->top_y = top_y + chunk_y;

		for (int j = 0; j < MAP_ROW_CHUNK; ++j) {
			for (int i = 0; i < SCREEN_WIDTH; ++i) {
				int real_x = left_x + i;
				int real_y = top_y + chunk_y + j;

				if (real_x >= 0 && real_x < MAP_WIDTH && real_y >= 0 && real_y < MAP_HEIGHT)
					p->tiles[j][i] = (char)map_data[real_y][real_x];
				else
					p->tiles[j][i] = 8;
			}
		}

		p->size = sizeof(sc_packet_mapInfo);
		registerSend(p);
	}
}

void GAMESESSION::sendStatChange(int t) {
	sc_packet_stat_change p;
	p.type = S2C_P_STAT_CHANGE;
	p.size = sizeof(p);
	p.hp = _hp;
	p.exp = _exp;
	p.level = _level;
	p.seedNum = _seedNum;
	p.mapType = t;
	registerSend(&p);
}

void GAMESESSION::updateSector(short oldX, short oldY) {
	auto [old_sx, old_sy] = getSectorLocation(oldX, oldY);
	auto [new_sx, new_sy] = getSectorLocation(_x, _y);

	if (old_sx == new_sx && old_sy == new_sy) return;
	{
		std::scoped_lock lock(sectorLock[old_sx][old_sy], sectorLock[new_sx][new_sy]);
		sectors[old_sx][old_sy].erase(_id);
		sectors[new_sx][new_sy].insert(_id);
	}
}

std::unordered_set<int> GAMESESSION::getNearBySessions() {
	std::unordered_set<int> result;
	auto [sx, sy] = getSectorLocation(_x, _y);
	if (sx < 0 || sx >= MAX_SECTOR_X || sy < 0 || sy >= MAX_SECTOR_Y) {
		std::cout << "[ERROR] 잘못된 섹터 인덱스! x: " << _x << " y: " << _y
			<< " sx: " << sx << ", sy: " << sy << '\n';
		__debugbreak();  // 디버거 중단
		return {};
	}
	int range = (VIEW_RANGE + SECTOR_SIZE - 1) / SECTOR_SIZE;  // 시야 범위만큼 커버하려면 몇섹터?가 필요한지

	for (int dx = -range; dx <= range; ++dx) {
		for (int dy = -range; dy <= range; ++dy) {
			int nx = sx + dx;
			int ny = sy + dy;
			if (nx < 0 || ny < 0 || nx >= MAX_SECTOR_X || ny >= MAX_SECTOR_Y) {
				/*std::cout << "[SECTOR BOUNDS ERROR] nx=" << nx << " ny=" << ny
					<< " from sx=" << sx << " sy=" << sy
					<< " dx=" << dx << " dy=" << dy << '\n';*/
				continue;
			}

			std::lock_guard<std::mutex> lock(sectorLock[nx][ny]);
			for (int id : sectors[nx][ny]) {
				if (id == _id) continue;

				auto it = clients.find(id);
				if (it == clients.end()) continue;

				auto other = it->second;
				if (!other || other->_state != ST_INGAME) continue;
				if (other->_isDie) continue;

				if (abs(other->_x - _x) > VIEW_RANGE) continue;
				if (abs(other->_y - _y) > VIEW_RANGE) continue;

				result.insert(id);
			}
		}
	}
	return result;
}

// NPC + lua =============================================================

int API_get_x(lua_State* L)
{
	int user_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);

	auto it = clients.find(user_id);
	std::shared_ptr<GAMESESSION> user = it->second;

	int x = user->_x;
	lua_pushnumber(L, x);
	return 1;
}

int API_get_y(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);

	auto it = clients.find(user_id);
	std::shared_ptr<GAMESESSION> user = it->second;

	int y = user->_y;
	lua_pushnumber(L, y);
	return 1;
}

int API_SendMessage(lua_State* L)
{
	//cout << "API_SendMessage 호출" << endl;
	int my_id = (int)lua_tointeger(L, -3);
	int user_id = (int)lua_tointeger(L, -2);
	char* mess = (char*)lua_tostring(L, -1);

	lua_pop(L, 4);

	auto it = clients.find(user_id);
	std::shared_ptr<GAMESESSION> user = it->second;

	user->sendChatPacket(my_id, mess);
	return 0;
}

void wakeUpMonster(int monId, int playerId) {
	//cout << "montser 깨워지냐 " << endl;
	auto it = clients.find(monId);
	std::shared_ptr<GAMESESSION> monster = it->second;

	OVER_EXP* exover = new OVER_EXP;
	exover->_compType = OP_PLAYER_MOVE;
	exover->_playerId = playerId;

	PostQueuedCompletionStatus(g_hiocp, 1, monId, &exover->_over);
	if (monster->_isActive.exchange(true) == false) {
		int idx = get_timer_queue_index(monster->_id);
		if (idx < 0 || idx >= TIMER_THREAD_COUNT) {
			cout << "[ERROR] 잘못된 타이머 큐 인덱스: " << idx << endl;
			return;
		}
		std::lock_guard<std::mutex> guard(timerLocks[idx]);
		timerQueues[idx].emplace(TIMER_EVENT{ monId, chrono::high_resolution_clock::now() + 1s, EV_RANDOM_MOVE, 0 });
		//cout << " 큐 등록 끝 " << endl;
	}
}

void trySleepMonster(std::shared_ptr<GAMESESSION> monster) {
	auto _near = monster->getNearBySessions();
	if (_near.size() == 0) return;

	for (auto& c : _near) {
		auto near_it = clients.find(c);
		std::shared_ptr<GAMESESSION> user = near_it->second;
		if (!isPlayer(user->_id)) continue;
		if (user->_state != ST_INGAME) continue;

		if (isCanSee(monster->_id, user->_id)) {
			return; // 누군가에게라도 보이면 깨운 상태 유지
		}
	}
	monster->_isActive = false;
}

void doMonsterRandomMoving(int monId) {
	auto it = clients.find(monId);
	std::shared_ptr<GAMESESSION> monster = it->second;

	if (isPlayer(monId)) return;
	if (monster->_isActive == false) return;
	if (monster->_isMonsterChase) return;	

	unordered_set<int> old_vl;
	{
		auto nearby_ids = monster->getNearBySessions();
		for (int id : nearby_ids) {
			if (isMonster(id)) continue;
			if (!isCanSee(monster->_id, id)) continue;
			old_vl.insert(id);
		}
	}
	short x = monster->_x;
	short y = monster->_y;
	short old_x = x;
	short old_y = y;
	switch (rand() % 4) {
	case 0: if (x < (MAP_WIDTH - 1)) x++; break;
	case 1: if (x > 0) x--; break;
	case 2: if (y < (MAP_HEIGHT - 1)) y++; break;
	case 3:if (y > 0) y--; break;
	}
	monster->_x = x;
	monster->_y = y;

	monster->updateSector(old_x, old_y);

	unordered_set<int> new_vl;	// 이동후 새 뷰리스트..

	{
		auto nearby_ids = monster->getNearBySessions(); // monster->x, monster->y는 이미 이동된 상태
		for (int id : nearby_ids) {
			if (isMonster(id)) continue;
			if (!isCanSee(monster->_id, id)) continue;
			new_vl.insert(id);
		}
	}

	for (auto pl : new_vl) {
		auto it = clients.find(pl);
		std::shared_ptr<GAMESESSION> vl_other = it->second;
		if (0 == old_vl.count(pl)) {		// 이동해서 새로 보이는애라면, 
			// 플레이어의 시야에 등장
			vl_other->sendAddPlayerPacket(monster->_id);	// 걔한테 나(monster) 추가
		}
		else {
			vl_other->sendMovePacket(monster->_id);	// 그게 아니라면.. 단순히 위치만 업뎃
		}
	}

	for (auto pl : old_vl) {			// 이동 전 뷰리 보면서...
		auto it = clients.find(pl);
		std::shared_ptr<GAMESESSION> vl_other = it->second;
		if (0 == new_vl.count(pl)) {	// 새 뷰리에 없다면?
			vl_other->_vlL.lock();
			if (0 != vl_other->_viewList.count(monster->_id)) {	// 만약.. 클라의 뷰리에. 나(monster) 존재한다면
				vl_other->_vlL.unlock();
				vl_other->sendRemoveSessionPacket(monster->_id);	// 걔한테 나(monster) 지우라고 알리기
			}
			else {
				vl_other->_vlL.unlock();
			}
		}
	}

	using namespace chrono;
	long long current_time = duration_cast<milliseconds>
		(system_clock::now().time_since_epoch()).count();
	if (MAX_USER == monId) {	// 이건 시간출력하는.. 이동 시간 저장용
		//std::cout << "MOVE : " << current_time - monster->last_move_time << "ms.\n";
	}

	//monster->last_move_time = current_time;

	trySleepMonster(monster);
}

void monsterInit()
{
	cout << "monster initialize begin.\n";
	for (int i = MAX_USER; i < MAX_USER + NUM_MONSTER; ++i) {
		auto newMonster = std::make_shared<GAMESESSION>();

		newMonster->_x = rand() % MAP_WIDTH;
		newMonster->_y = rand() % MAP_HEIGHT;
		newMonster->_hp = rand() % 5 + 5;

		newMonster->_monType = rand() % 2 + 1;	// 1이면 일반, 2면 훈스터
		newMonster->_id = i;
		newMonster->_state = ST_INGAME;

		newMonster->_isActive = false;
		if (newMonster->_monType == 1)
			sprintf_s(newMonster->_name, "HAM");
		if (newMonster->_monType == 2) 
			sprintf_s(newMonster->_name, "HUN");

		clients[i] = newMonster;

		auto L = newMonster->_L = luaL_newstate();	// LUA 상태 초기화...
		luaL_openlibs(L);							// 표준 라이브러리 로딩하는거..
		luaL_loadfile(L, "npc.lua");				// npc.lua 스크립트 파일 로드
		lua_pcall(L, 0, 0, 0);						// 스크립트 실행

		lua_getglobal(L, "set_uid");		// lua함수 set_uid()를 호출하며
		lua_pushnumber(L, i);				// i 라는 아이디를 lua에 전달..
		lua_pcall(L, 1, 0, 0);

		lua_register(L, "API_SendMessage", API_SendMessage);
		lua_register(L, "API_get_x", API_get_x);
		lua_register(L, "API_get_y", API_get_y);

		// 딱히 필요없지 않나..?
		auto [sx, sy] = getSectorLocation(newMonster->_x, newMonster->_y);
		{
			std::lock_guard<std::mutex> lock(sectorLock[sx][sy]);
			sectors[sx][sy].insert(i);
		}
	}
	cout << "monster initialize end.\n";
}

// timer 함수 =================================================================

void doTimer(int timerIdx) {
	using namespace chrono;
	do {
		do {
			std::lock_guard<std::mutex> lock(timerLocks[timerIdx]);
			if (timerQueues[timerIdx].empty()) break;

			auto& k = timerQueues[timerIdx].top();	// top에서 꺼냈는데, 
			if (k.wakeup_time > high_resolution_clock::now()) {	// 걔가 아직 시간이 안됐으면..
				break;
			}

			// timer에서 꺼냈을 때
			if (isMonster(k.obj_id)) {
				auto npc_it = clients.find(k.obj_id);
				if (npc_it != clients.end()) {
					std::shared_ptr<GAMESESSION> npc = npc_it->second;
					//std::lock_guard<std::mutex> lock(timer_locks[idx]);
					if (npc->_isActive == false) {
						timerQueues[timerIdx].pop(); // 그냥 버림
						continue;
					}
				}
			}

			// 시간이 된 이벤트들을 처리하는..
			switch (k.event_id) {			// 시간이됐다면,여기로넘어왔을테니..
			case EV_RANDOM_MOVE: {
				OVER_EXP* o = new OVER_EXP;
				o->_compType = OP_MONSTER_MOVE;	// npc움직였다고 해주기
				PostQueuedCompletionStatus(g_hiocp, 1, k.obj_id, &o->_over);
				// 이거 처리해줘~라고 직접!!이벤트 던지는 함수임 
				break;
			}
			case EV_RUN: {
				OVER_EXP* ov = new OVER_EXP;
				ov->_compType = OP_NPC_RUN;
				ov->_playerId = k.target_id;
				PostQueuedCompletionStatus(g_hiocp, 1, k.obj_id, &ov->_over);
				break;
			}
			case EV_CHASE: {
				cout << "ev_chase도착" << endl;
				OVER_EXP* ov = new OVER_EXP;
				ov->_compType = OP_MONSTER_CHASE;	
				ov->_playerId = k.target_id;
				PostQueuedCompletionStatus(g_hiocp, 1, k.obj_id, &ov->_over);
				break;
			}
			}
			timerQueues[timerIdx].pop();			// 이벤트 처리했으면 큐에서 제거..

		} while (true);
		this_thread::sleep_for(chrono::milliseconds(10));	// 할수있는건 다 처리하고, 10ms마다 쉰다..
	} while (true);
}

// 작동 부분 =============================================================

void disconnect(int sessionId) {
	auto it = clients.find(sessionId);
	std::shared_ptr<GAMESESSION> session = it->second;

	session->_vlL.lock();
	unordered_set <int> vl = session->_viewList;
	session->_vlL.unlock();
	for (auto& p_id : vl) {
		if (isMonster(p_id)) continue;
		auto& pl = clients[p_id];
		std::shared_ptr<GAMESESSION> vl_other = pl;
		if (ST_INGAME != vl_other->_state) continue;

		if (vl_other->_id == sessionId) continue;
		vl_other->sendRemoveSessionPacket(sessionId);
	}
	closesocket(session->_socket);
	session->_state = ST_FREE;
	if (update_user_info(session->_userInputId, session->_x, session->_y, session->_seedNum,
		session->_hp, session->_level, session->_exp, session->_maxHp, session->_damage)) {
		//cout << "없뎃성공" << endl;
	}
	else cout << "업뎃실패. " << endl;
}

// 받았으면 보낼 준비
void processPacket(int sessionId, char* packetFromC) {
	//cout << "날아온 아이디 확인용 :" << sessionId << endl;
	auto it = clients.find(sessionId);
	std::shared_ptr<GAMESESSION> session = it->second;

	switch (packetFromC[1]) {
	case C2S_P_LOGIN:
	{	// 로그인 패킷이 날아온거라면 DB 확인
		cs_packet_login* p = reinterpret_cast<cs_packet_login*>(packetFromC);

		int dbUserX = 0, dbUserY = 0, dbUserHp = 0, dbUserLevel = 0,
			dbUserExp = 0, dbUserSeedNum = 0, dbUserMaxHp = 0, dbUserDamage = 0;
		int dbId = p->inputId;

		bool loginResult = get_DB_login_info(
			dbId, dbUserX, dbUserY, dbUserHp, dbUserLevel,
			dbUserExp, dbUserSeedNum, dbUserMaxHp, dbUserDamage
		);



		if (!loginResult) {
			cout << "신규유저" << dbId <<"아이디생성" <<endl;
			if (!insert_new_user(dbId, p->name)) {
				std::cout << "신규 유저 삽입 실패" << std::endl;
				sc_packet_login_fail fail;
				fail.size = sizeof(fail);
				fail.size = sizeof(fail);
				fail.type = S2C_P_LOGIN_FAIL;
				session->registerSend(&fail);
				disconnect(sessionId);
				break;
			}

			cout << "신규유저 삽입됨" << endl;
			if (!get_DB_login_info(
				dbId, dbUserX, dbUserY, dbUserHp, dbUserLevel,
				dbUserExp, dbUserSeedNum, dbUserMaxHp, dbUserDamage)) {
				std::cout << "삽입 후 정보 조회 실패" << std::endl;
				sc_packet_login_fail fail;
				fail.size = sizeof(fail);
				fail.type = S2C_P_LOGIN_FAIL;
				session->registerSend(&fail);
				disconnect(sessionId);
				break;
			}
		}

		session->_userInputId = dbId;
		session->_x = dbUserX;
		session->_y = dbUserY;
		session->_hp = dbUserHp;
		session->_level = dbUserLevel;
		session->_exp = dbUserExp;
		session->_seedNum = dbUserSeedNum;
		session->_maxHp = dbUserMaxHp;
		session->_damage = dbUserDamage;
		session->_monType = 0;

		strcpy_s(session->_name, p->name);
		//session->sendMapInfo(session->_id, session->_x, session->_y);

		OVER_EXP* ov = new OVER_EXP;
		ov->_compType = OP_LOGIN_OK;
		PostQueuedCompletionStatus(g_hiocp, 1, sessionId, &ov->_over);

		if (update_user_info(session->_userInputId, session->_x, session->_y, session->_seedNum,
			session->_hp, session->_level, session->_exp, session->_maxHp, session->_damage)) {
			//cout << "없뎃성공" << endl;
		}
		else cout << "업뎃실패. " << endl;
	}
	break;
	case C2S_P_MOVE:
	{
		cs_packet_move* p = reinterpret_cast<cs_packet_move*>(packetFromC);

		short oldX = session->_x;
		short oldY = session->_y;
		short x = session->_x;
		short y = session->_y;

		switch (p->direction) {	// 이건 이동방향 계산..
		case 0: if (y > 0) y--; break;
		case 1: if (y < MAP_HEIGHT - 1) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < MAP_WIDTH - 1) x++; break;
		}

		if (playerCanMove(x, y)) {
			switch (map_data[y][x]) {
			case 4:
				cout << "seed Get" << endl;
				session->_seedNum++;
				session->sendStatChange(4);
				map_data[y][x] = rand() % 2;
				break;
			case 5:
				cout << "똥 밟음 -HP" << endl;
				if (session->_hp > 0)
					session->_hp--;
				session->sendStatChange(5);
				map_data[y][x] = rand() % 2;
				break;
			case 6:
				cout << "potion Get" << endl;
				if (session->_hp < session->_maxHp)
					session->_hp++;
				session->sendStatChange(6);
				map_data[y][x] = rand() % 2;
				break;
			}
			session->_x = x;
			session->_y = y;
		}
		else {
			//cout << " 이동 불가.. " << endl;
			session->_x = oldX;
			session->_y = oldY;
		}

		session->updateSector(oldX, oldY);
		unordered_set<int> _near_vlist = session->getNearBySessions();
		unordered_set<int> nearMonsterList = _near_vlist;

		session->_vlL.lock();
		unordered_set<int> _oldVl = session->_viewList;	// 지금 유저의. 이전 시야를 백업해둔다..
		session->_vlL.unlock();

		session->sendMovePacket(sessionId);	// 나한테 나 움직였다고 알리기
		session->sendMapInfo(session->_id, session->_x, session->_y);

		for (auto& pl : _near_vlist) {	// nearlist돌면서... (이거 상대방기준임****)
			auto it = clients.find(pl);
			std::shared_ptr<GAMESESSION> near_other = it->second;
			if (isPlayer(pl)) {			// 플레이어라면...
				near_other->_vlL.lock();
				if (near_other->_viewList.count(sessionId)) {	// 만약에.. 전에도 있던애라면? 이미 상대방이 나를 보고있던 거라면?
					near_other->_vlL.unlock();
					near_other->sendMovePacket(sessionId);	// 움직임 전송
				}
				else {									// 그게 아니라.. 전에 없던애.. 내가 움직여서 상대방이 내가 새로 보인다면?
					near_other->_vlL.unlock();
					near_other->sendAddPlayerPacket(sessionId);	// 새로등장한거니까 움직임전송x -> 상대한테 나 추가하기!!
				}
			}
			else if (isMonster(pl)) {						// npc라면?
				wakeUpMonster(near_other->_id, session->_id);	// 깨운다(왜냐면 npc니까.. 얼려진상태로 있는게 디폴트임)
			}

			if (_oldVl.count(pl) == 0)			// 만약에.. 내가 움직여서.내가 새롭게 본 상대가 있다?
				session->sendAddPlayerPacket(pl);	// 걔도 추가해준담..
		}

		for (auto& pl : _oldVl) {			// oldvlist 돌면서...
			auto it = clients.find(pl);
			std::shared_ptr<GAMESESSION> old_other = it->second;
			if (0 == _near_vlist.count(pl)) {				// 만약에.. 지금도 보이는게 아니라면?
				session->sendRemoveSessionPacket(pl);	// 나한테 걔를 지워주자..
				if (isPlayer(pl))									// 상대가 플레이어라면
					old_other->sendRemoveSessionPacket(sessionId);// 상대에서 나를 지워주자..
				if (isMonster(pl))
					old_other->_isActive = false;
			}
		}

		vector<shared_ptr<GAMESESSION>> targets;
		int myX = session->_x;
		int myY = session->_y;
		for (int id : nearMonsterList) {
			if (isPlayer(id)) continue;

			auto it = clients.find(id);
			if (it == clients.end()) continue;

			std::shared_ptr<GAMESESSION> monster = it->second;

			int dx = abs(monster->_x - myX);
			int dy = abs(monster->_y - myY);

			if (dx <= 2 and dy <= 2) targets.push_back(monster);
		}

		//cout << " 공격가능 몬스터 수 : " << targets.size() << endl;
		int tempCnt = 0;
		int cnt = targets.size();
		nearMonsterInfo nearMons[5];
		for (auto& target : targets) {
			nearMons[tempCnt].monHp = target->_hp;
			nearMons[tempCnt].monId = target->_id;
			nearMons[tempCnt].monType = target->_monType;
			tempCnt++;
		}
		session->sendMonsterNearPacket(nearMons, cnt);

	
	}
	break;
	case C2S_P_ATTACK:
	{
		cout << "seed : " << session->_seedNum << endl;
		unordered_set<int> viewList = session->getNearBySessions();
		vector<shared_ptr<GAMESESSION>> targets;

		int myX = session->_x;
		int myY = session->_y;

		for (int id : viewList) {
			if (isPlayer(id)) continue;

			auto it = clients.find(id);
			if (it == clients.end()) continue;

			std::shared_ptr<GAMESESSION> monster = it->second;
			int dx = abs(monster->_x - myX);
			int dy = abs(monster->_y - myY);

			if (dx <= 2 and dy <= 2) targets.push_back(monster);	// 범위 설정
		}

		//cout << "주변 몬스터 수 : " << targets.size() << endl;
		for (auto& target : targets) {
			target->_hp -= session->_damage;	// 데미지 깎고..
			session->_seedNum--;				// seed 갯수 줄이기
			if (target->_hp <= 0) {
				cout << target->_id << " 사망 " << endl;
				target->_hp = 0;
				target->_isDie = true;
				target->_isActive = false;
				session->sendMonsterDiePacket(target->_id);
				session->sendAttackPacket_PtoN(target->_id, target->_hp);
			}
			else {
				session->sendAttackPacket_PtoN(target->_id, target->_hp);
				auto path = findPathAStar(target->_x, target->_y, session->_x, session->_y);
				if (!path.empty() && path.size() > 1) {
					// path[0] = 현재 위치, path[1] = 다음 목적지
					int next_x = path[1].first;
					int next_y = path[1].second;
					target->_nextMonX = next_x;
					target->_nextMonY = next_y;
					session->sendMovePacket(target->_id);	
					target->_isMonsterChase = true;

					// 타이머에 chase 등록
					int idx = target->_id % TIMER_THREAD_COUNT;
					std::lock_guard<std::mutex> guard(timerLocks[idx]);
					timerQueues[idx].emplace(TIMER_EVENT{ target->_id, 
						chrono::high_resolution_clock::now() + 100ms, EV_CHASE, session->_id });
				}
			}
		}

		int tempCnt = 0;
		int cnt = targets.size();
		nearMonsterInfo nearMons[5];
		for (auto& target : targets) {
			nearMons[tempCnt].monHp = target->_hp;
			nearMons[tempCnt].monId = target->_id;
			nearMons[tempCnt].monType = target->_monType;
			tempCnt++;
		}
		session->sendMonsterNearPacket(nearMons, cnt);
		if (update_user_info(session->_userInputId, session->_x, session->_y, session->_seedNum,
			session->_hp, session->_level, session->_exp, session->_maxHp, session->_damage)) {
			//cout << "없뎃성공" << endl;
		}
		else cout << "업뎃실패. " << endl;
	}
	break;
	case C2S_P_CHAT:
	{
		cout << " 채팅 받음" << endl;
		cs_packet_chat* p = reinterpret_cast<cs_packet_chat*>(packetFromC);

		char* messs = p->message;
		cout << p->id << "로부터 온 도착 메시지 :" << p->message << endl;

		for (auto& [id, client] : clients) {
			if (client && client->_state == ST_INGAME) {
				client->sendChatPacket(sessionId, messs);
			}
		}
	}
	break;
	case C2S_P_TELEPORT:
	{

	}
	break;
	}
}

void workerThread(HANDLE iocp) {
	while (true) {
		DWORD numBytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(iocp, &numBytes, &key, &over, INFINITE);
		if (over == nullptr) {
			cout << "[ERROR] GetQueuedCompletionStatus returned null overlapped\n";
			continue;
		}

		OVER_EXP* overEx = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (overEx->_compType == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << " GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (overEx->_compType == OP_SEND)
					delete overEx;
				continue;
			}
		}

		if ((0 == numBytes) && ((overEx->_compType == OP_RECV) || (overEx->_compType == OP_SEND))) {
			disconnect(static_cast<int>(key));
			if (overEx->_compType == OP_SEND)
				delete overEx;
			continue;
		}

		switch (overEx->_compType) {
		case OP_ACCEPT:
		{
			int clientId = getNewPlayerId();
			if (clientId != -1) {
				auto newPlayer = std::make_shared<GAMESESSION>();
				newPlayer->_state = ST_ALLOC;		// 일단 수락만 했고, 로그인 아직 아닌 상태
				newPlayer->_x = 0;
				newPlayer->_y = 0;
				newPlayer->_id = clientId;
				newPlayer->_hp = 0;
				newPlayer->_exp = 0;
				newPlayer->_level = 0;
				newPlayer->_name[0] = 0;
				newPlayer->_prevRemainData = 0;
				newPlayer->_socket = gCliSocket;

				clients[clientId] = newPlayer;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(gCliSocket), iocp, clientId, 0);
				newPlayer->registerRecv();
				gCliSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << " 유저 MAX " << endl;
			}
			ZeroMemory(&gOverExp._over, sizeof(gOverExp._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(gServerSocket, gCliSocket, gOverExp._sendBuf,
				0, addr_size + 16, addr_size + 16, 0, &gOverExp._over);
		}
		break;
		case OP_RECV:
		{
			auto it = clients.find(key);
			std::shared_ptr<GAMESESSION> cli = it->second;
			int remainData = numBytes + cli->_prevRemainData;

			OVER_EXP* recv_over = &cli->_recvOver;
			//cout << " workerThread (OP_RECV) 확인용 " << key << " " << cli->_id << endl;
			char* p = recv_over->_sendBuf;
			while (remainData > 0) {
				int packet_size = p[0];
				if (packet_size <= remainData) {
					processPacket(static_cast<int>(key), p);
					//cout << "OP_RECV  속 key : " << key << endl;
					p = p + packet_size;
					remainData = remainData - packet_size;
				}
				else break;
			}
			cli->_prevRemainData = remainData;

			if (remainData > 0) {
				memcpy(overEx->_sendBuf, p, remainData);
			}
			cli->registerRecv();
		}
		break;
		case OP_LOGIN_OK:
		{
			//cout << " 로그인 성공 !!!! " << endl;
			auto it = clients.find(key);
			std::shared_ptr<GAMESESSION> session = it->second;

			session->_state = ST_INGAME;
			session->sendSessionLoginPacket();	// 나한테 정보전송 -> 그래야 내가본다
			session->sendSessioLoginOkPacket();
			session->sendMapInfo(session->_id, session->_x, session->_y); // 맵 정보

			auto [sx, sy] = getSectorLocation(session->_x, session->_y);	// 로그인 후.. 나를 섹터에 등록
			{
				std::lock_guard<std::mutex> lock(sectorLock[sx][sy]);
				sectors[sx][sy].insert(session->_id);
			}

			// 이제 브로드캐스팅
			std::unordered_set<int> _near = it->second->getNearBySessions();
			for (auto& pl : _near) {	// 그리고 나서.... -> 로그인한유저 근처에 있는 애들한테 정보보내기
				auto it2 = clients.find(pl);
				std::shared_ptr<GAMESESSION> other = it2->second;
				if (ST_INGAME != other->_state) continue;	// 접속중인애들한테만!!

				if (other->_id == session->_id) // 나는 제외
					continue;
				if (false == isCanSee(session->_id, other->_id))	// 내가 안보여도 제외
					continue;

				if (isPlayer(other->_id)) // 걔가 플레이어면?
					other->sendAddPlayerPacket(session->_id);	// 상대한테 내가왔다고 알린다.
				else if (isMonster(pl)) {
					wakeUpMonster(other->_id, session->_id);	// 깨운다(왜냐면 npc니까..     얼려진상태로 있는게 디폴트임)
				}
				session->sendAddPlayerPacket(other->_id);	//이제 나한테.. 상대방정보 알리기
			}
		}
		break;
		case OP_LOGIN_FAIL:
		{

		}
		break;
		case OP_SEND:
		{

		}
		break;
		case OP_PLAYER_MOVE:
		{
			if (over == nullptr) {
				cout << "[ERROR] overlapped 포인터가 null입니다!" << endl;
				break;
			}
			if (overEx == nullptr) {
				cout << "[ERROR] ex_over is nullptr!" << endl;
				break;
			}
			if (not isMonster(key)) {
				delete overEx;
				break;
			}
			//cout << "OP_PLAYER_MOVE ?? " << endl;
			//if (not is_npc(key)) break;

			auto it = clients.find(key);
			if (it == clients.end()) break;
			std::shared_ptr<GAMESESSION> npc = it->second;

			if (not npc->_isActive) break;
			//if (npc->_is_npc_run) break;

			npc->_ll.lock();
			auto L = npc->_L;
			if (L == nullptr) {
				cout << "[ERROR] npc->_L is nullptr!" << endl;
				npc->_ll.unlock();
				delete overEx;
				break;
			}
			lua_getglobal(L, "event_player_move");
			lua_pushnumber(L, overEx->_playerId);
			int error = lua_pcall(L, 1, 0, 0);
			if (error) {
				cout << " OP_AI_HELLO Error : " << lua_tostring(L, -1);
				lua_pop(L, 1);
			}
			//lua_pop(L, 1);
			npc->_ll.unlock();
			delete overEx;
		}
		break;
		case OP_MONSTER_MOVE:
		{
			using namespace chrono;

			auto it_mon = clients.find(key);
			std::shared_ptr<GAMESESSION> monster = it_mon->second;

			int monsterId = static_cast<int>(key);

			if (not monster->_isMonsterChase) {
				doMonsterRandomMoving(monsterId);	// 그 키 가진 npc 랜덤으로 움직이게 만들기
				int idx = monsterId % TIMER_THREAD_COUNT;
				std::lock_guard<std::mutex> guard(timerLocks[idx]);
				timerQueues[idx].emplace(TIMER_EVENT{ monsterId, high_resolution_clock::now() + 1s, EV_RANDOM_MOVE, 0 });
			}
			delete overEx;
			break;
		}
		case OP_MONSTER_CHASE:
		{
			int targetPlayerId = overEx->_playerId;
			// 여기에.. 몬스터가 쫓아가는?
			//cout << "monster[" << key << "]가 플레이어" << targetPlayerId << " chasing 중.. " << endl;
			if (over == nullptr) {
				cout << "[ERROR] overlapped 포인터가 null입니다!" << endl;
				break;
			}
			auto it_mon = clients.find(key);
			if (it_mon == clients.end()) break;

			std::shared_ptr<GAMESESSION> monster = it_mon->second;
			if (not monster->_isActive) break;
			monster->_isMonsterChase = true;

			monster->_x = monster->_nextMonX;
			monster->_y = monster->_nextMonY;

			auto it = clients.find(targetPlayerId);
			std::shared_ptr<GAMESESSION> player = it->second;
			player->sendMovePacket(monster->_id);

			if (isMonSee(monster->_id, targetPlayerId)) {
				shared_ptr<GAMESESSION> player = clients[targetPlayerId];
				auto path = findPathAStar(monster->_x, monster->_y, player->_x, player->_y);
				if (!path.empty() && path.size() > 1) {
					monster->_nextMonX = path[1].first;
					monster->_nextMonY = path[1].second;

					int idx = monster->_id % TIMER_THREAD_COUNT;
					std::lock_guard<std::mutex> guard(timerLocks[idx]);
					timerQueues[idx].emplace(TIMER_EVENT{ monster->_id, chrono::high_resolution_clock::now() + 1s
						, EV_CHASE, targetPlayerId });
				}
			}
			else if (not isMonSee(monster->_id, targetPlayerId)) {
				//cout << " 시야에서 사라져서 쫓아가기 중단.. " << endl;
				monster->_isMonsterChase = false;
				int idx = monster->_id % TIMER_THREAD_COUNT;
				timerQueues[idx].emplace(TIMER_EVENT{ monster->_id, chrono::high_resolution_clock::now() + 1s
					, EV_RANDOM_MOVE, 0 });
			}
			break;
		}
		}

	}
}


// main =============================================================
int main()
{
	if (not initializeDB()) {
		cout << " DB연결 실패..." << endl;
		return -1;
	}
	else cout << " DB연결 성공!" << endl;

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	gServerSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(GAME_PORT);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(gServerSocket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(gServerSocket, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	monsterInit();
	initializeMap();

	g_hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(gServerSocket), g_hiocp, 9999, 0);
	gCliSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	gOverExp._compType = OP_ACCEPT;
	AcceptEx(gServerSocket, gCliSocket, gOverExp._sendBuf, 0, addr_size + 16, addr_size + 16, 0, &gOverExp._over);

	vector <thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency() * 2;

	for (int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back(workerThread, g_hiocp);

	for (int i = 0; i < TIMER_THREAD_COUNT; ++i)
		std::thread(doTimer, i).detach();

	for (auto& th : worker_threads)
		th.join();

	closesocket(gServerSocket);
	WSACleanup();
}
