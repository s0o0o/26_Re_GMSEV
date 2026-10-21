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
#include "startup_config.hpp"
#include "../../shared/clubhouse_map.hpp"
#include <cstring>
#include "world_map.hpp"
#include "pathfinder.hpp"
#include "../../shared/login_input.hpp"
#include "game_session.hpp"

using namespace std;

#include "include/lua.hpp"
#include "session_registry.hpp"

#include "game_database.hpp"
#include <locale.h>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "lua54.lib")

// 전역변수들 ========================================================

HANDLE g_hiocp;
SOCKET gServerSocket, gCliSocket;

// SESSION 부분 ============================================================

OVER_EXP gOverExp;

//////////////////////////////// a*알고리즘 

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

	if (abs(fromUser->object._x - toUser->object._x) > VIEW_RANGE)
		return false;
	return abs(fromUser->object._y - toUser->object._y) <= VIEW_RANGE;
}

bool isMonSee(int from, int to)
{
	auto from_it = clients.find(from);
	std::shared_ptr<GAMESESSION> fromUser = from_it->second;

	auto to_it = clients.find(to);
	std::shared_ptr<GAMESESSION> toUser = to_it->second;

	if (abs(fromUser->object._x - toUser->object._x) > 5)
		return false;
	return abs(fromUser->object._y - toUser->object._y) <= 5;
}


void initializeMap() {
    std::cout << "[MAP] Preparing Club House 24x18..." << std::endl;
    for (int y=0;y<MAP_HEIGHT;++y)
        for (int x=0;x<MAP_WIDTH;++x)
            map_data[y][x] = clubhouse::walkable(x,y) ? 0 : 3;
    std::cout << "[MAP] Club House ready." << std::endl;
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
void GAMESESSION::updateSector(short oldX, short oldY) {
	auto [old_sx, old_sy] = getSectorLocation(oldX, oldY);
	auto [new_sx, new_sy] = getSectorLocation(object._x, object._y);

	if (old_sx == new_sx && old_sy == new_sy) return;
	{
		std::scoped_lock lock(sectorLock[old_sx][old_sy], sectorLock[new_sx][new_sy]);
		sectors[old_sx][old_sy].erase(object._id);
		sectors[new_sx][new_sy].insert(object._id);
	}
}

std::unordered_set<int> GAMESESSION::getNearBySessions() {
	std::unordered_set<int> result;
	auto [sx, sy] = getSectorLocation(object._x, object._y);
	if (sx < 0 || sx >= MAX_SECTOR_X || sy < 0 || sy >= MAX_SECTOR_Y) {
		std::cout << "[ERROR] 잘못된 섹터 인덱스! x: " << object._x << " y: " << object._y
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
				if (id == object._id) continue;

				auto it = clients.find(id);
				if (it == clients.end()) continue;

				auto other = it->second;
				if (!other || other->_state != ST_INGAME) continue;
				if (other->monster._isDie) continue;

				if (abs(other->object._x - object._x) > VIEW_RANGE) continue;
				if (abs(other->object._y - object._y) > VIEW_RANGE) continue;

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

	int x = user->object._x;
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

	int y = user->object._y;
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
	if (monster->monster._isActive.exchange(true) == false) {
		int idx = get_timer_queue_index(monster->object._id);
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
		if (!isPlayer(user->object._id)) continue;
		if (user->_state != ST_INGAME) continue;

		if (isCanSee(monster->object._id, user->object._id)) {
			return; // 누군가에게라도 보이면 깨운 상태 유지
		}
	}
	monster->monster._isActive = false;
}

void doMonsterRandomMoving(int monId) {
	auto it = clients.find(monId);
	std::shared_ptr<GAMESESSION> monster = it->second;

	if (isPlayer(monId)) return;
	if (monster->monster._isActive == false) return;
	if (monster->monster._isMonsterChase) return;

	unordered_set<int> old_vl;
	{
		auto nearby_ids = monster->getNearBySessions();
		for (int id : nearby_ids) {
			if (isMonster(id)) continue;
			if (!isCanSee(monster->object._id, id)) continue;
			old_vl.insert(id);
		}
	}
	short x = monster->object._x;
	short y = monster->object._y;
	short old_x = x;
	short old_y = y;
	switch (rand() % 4) {
	case 0: if (x < (MAP_WIDTH - 1)) x++; break;
	case 1: if (x > 0) x--; break;
	case 2: if (y < (MAP_HEIGHT - 1)) y++; break;
	case 3:if (y > 0) y--; break;
	}
	monster->object._x = x;
	monster->object._y = y;

	monster->updateSector(old_x, old_y);

	unordered_set<int> new_vl;	// 이동후 새 뷰리스트..

	{
		auto nearby_ids = monster->getNearBySessions(); // monster->x, monster->y는 이미 이동된 상태
		for (int id : nearby_ids) {
			if (isMonster(id)) continue;
			if (!isCanSee(monster->object._id, id)) continue;
			new_vl.insert(id);
		}
	}

	for (auto pl : new_vl) {
		auto it = clients.find(pl);
		std::shared_ptr<GAMESESSION> vl_other = it->second;
		if (0 == old_vl.count(pl)) {		// 이동해서 새로 보이는애라면, 
			// 플레이어의 시야에 등장
			vl_other->sendAddPlayerPacket(monster->object._id);	// 걔한테 나(monster) 추가
		}
		else {
			vl_other->sendMovePacket(monster->object._id);	// 그게 아니라면.. 단순히 위치만 업뎃
		}
	}

	for (auto pl : old_vl) {			// 이동 전 뷰리 보면서...
		auto it = clients.find(pl);
		std::shared_ptr<GAMESESSION> vl_other = it->second;
		if (0 == new_vl.count(pl)) {	// 새 뷰리에 없다면?
			vl_other->_vlL.lock();
			if (0 != vl_other->_viewList.count(monster->object._id)) {	// 만약.. 클라의 뷰리에. 나(monster) 존재한다면
				vl_other->_vlL.unlock();
				vl_other->sendRemoveSessionPacket(monster->object._id);	// 걔한테 나(monster) 지우라고 알리기
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

bool monsterInit(int count)
{
	cout << "monster initialize begin.\n";
	const auto started = GetTickCount64();
    std::cout << "[STARTUP] Monsters=" << count << " (GMSEV_MONSTER_COUNT; max 200000)" << std::endl;
    for (int i = MAX_USER; i < MAX_USER + count; ++i) {
		auto newMonster = std::make_shared<GAMESESSION>();

		newMonster->object._x = rand() % MAP_WIDTH;
		newMonster->object._y = rand() % MAP_HEIGHT;
		newMonster->object._hp = rand() % 5 + 5;

		newMonster->monster._monType = rand() % 2 + 1;	// 1이면 일반, 2면 훈스터
		newMonster->object._id = i;
		newMonster->_state = ST_INGAME;

		newMonster->monster._isActive = false;
		if (newMonster->monster._monType == 1)
			sprintf_s(newMonster->object._name, "HAM");
		if (newMonster->monster._monType == 2)
			sprintf_s(newMonster->object._name, "HUN");

		clients[i] = newMonster;

        auto L = newMonster->monster._L = luaL_newstate();
        if (!L) { std::cerr << "[STARTUP] Lua allocation failed monster=" << i << std::endl; return false; }
        luaL_openlibs(L);
        const auto failScript = [&]() {
            std::cerr << "[STARTUP] npc.lua failed monster=" << i << ": " << lua_tostring(L,-1) << std::endl;
            lua_close(L);
            newMonster->monster._L = nullptr;
            return false;
        };
        if (luaL_loadfile(L,"npc.lua") != LUA_OK) return failScript();
        if (lua_pcall(L,0,0,0) != LUA_OK) return failScript();
        lua_getglobal(L,"set_uid");
        lua_pushinteger(L,i);
        if (lua_pcall(L,1,0,0) != LUA_OK) return failScript();

		lua_register(L, "API_SendMessage", API_SendMessage);
		lua_register(L, "API_get_x", API_get_x);
		lua_register(L, "API_get_y", API_get_y);

		// 딱히 필요없지 않나..?
		auto [sx, sy] = getSectorLocation(newMonster->object._x, newMonster->object._y);
		{
			std::lock_guard<std::mutex> lock(sectorLock[sx][sy]);
			sectors[sx][sy].insert(i);
		}
        if ((i-MAX_USER+1)%250 == 0 || i-MAX_USER+1 == count)
            std::cout << "[STARTUP] Monsters " << i-MAX_USER+1 << "/" << count << " elapsed_ms=" << GetTickCount64()-started << std::endl;
	}
    std::cout << "[STARTUP] Monsters complete elapsed_ms=" << GetTickCount64()-started << std::endl;
    cout << "monster initialize end.\n";
    return true;
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
					if (npc->monster._isActive == false) {
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
	if (it == clients.end() || !it->second) return;
	std::shared_ptr<GAMESESSION> session = it->second;
    if (session->disconnectStarted.exchange(true)) return;
    session->transportFailed = true;
    const bool wasInGame = session->_state == ST_INGAME;

	session->_vlL.lock();
	unordered_set <int> vl = session->_viewList;
	session->_vlL.unlock();
	for (auto& p_id : vl) {
		if (isMonster(p_id)) continue;
		auto& pl = clients[p_id];
		std::shared_ptr<GAMESESSION> vl_other = pl;
		if (!vl_other || ST_INGAME != vl_other->_state) continue;

		if (vl_other->object._id == sessionId) continue;
		vl_other->sendRemoveSessionPacket(sessionId);
	}
	closesocket(session->_socket);
	session->_state = ST_FREE;
    if (!wasInGame) return;
	if (update_user_info(session->_userInputId, session->object._x, session->object._y, session->object._seedNum,
		session->object._hp, session->object._level, session->object._exp, session->object._maxHp, session->object._damage)) {
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
        const auto loginStarted = GetTickCount64();
        std::cout << "[LOGIN] Received session=" << sessionId << " bytes=" << static_cast<unsigned>(p->size) << std::endl;
		if (!validCharacterKind(p->character_kind) || !validFurColor(p->fur_color) || !validEyeStyle(p->eye_style) || !validScarfStyle(p->scarf_style) || p->inputId < 0 || !login_input::nickname(p->name, sizeof(p->name))) {
            sc_packet_login_fail fail{};
            fail.size = sizeof(fail);
            fail.type = S2C_P_LOGIN_FAIL;
            session->registerSend(&fail);
            break;
        }

		int dbUserX = 0, dbUserY = 0, dbUserHp = 0, dbUserLevel = 0,
			dbUserExp = 0, dbUserSeedNum = 0, dbUserMaxHp = 0, dbUserDamage = 0;
		int dbId = p->inputId;

		std::cout << "[LOGIN] DB lookup begin session=" << sessionId << std::endl;
        const auto loginResult = load_or_create_user(
            [&]() {
                return get_DB_login_info(dbId, dbUserX, dbUserY, dbUserHp, dbUserLevel,
                    dbUserExp, dbUserSeedNum, dbUserMaxHp, dbUserDamage);
            },
            [&]() { return insert_new_user(dbId, p->name); });
        std::cout << "[LOGIN] DB lookup end session=" << sessionId << " result=" << static_cast<int>(loginResult) << " elapsed_ms=" << GetTickCount64()-loginStarted << std::endl;
        if (loginResult != LoginLookupResult::Found) {
            sc_packet_login_fail fail{};
            fail.size = sizeof(fail);
            fail.type = S2C_P_LOGIN_FAIL;
            session->registerSend(&fail);
            // Stay connected for retry; do not save an uninitialized character.
            break;
        }

        clubhouse::safePosition(dbUserX,dbUserY);
        session->_userInputId = dbId;
        session->object._furColor = p->fur_color;
        session->object._characterKind = p->character_kind;
        session->object._eyeStyle = p->eye_style;
        session->object._scarfStyle = p->scarf_style;
		session->object._x = dbUserX;
		session->object._y = dbUserY;
		session->object._hp = dbUserHp;
		session->object._level = dbUserLevel;
		session->object._exp = dbUserExp;
		session->object._seedNum = dbUserSeedNum;
		session->object._maxHp = dbUserMaxHp;
		session->object._damage = dbUserDamage;
		session->monster._monType = 0;

		strncpy_s(session->object._name, p->name, MAX_ID_LENGTH - 1);
		//session->sendMapInfo(session->object._id, session->object._x, session->object._y);

		OVER_EXP* ov = new OVER_EXP;
		ov->_compType = OP_LOGIN_OK;
		        if (!PostQueuedCompletionStatus(g_hiocp,1,sessionId,&ov->_over)) {
            std::cerr << "[LOGIN] Completion enqueue failed: " << GetLastError() << std::endl;
            delete ov;
            sc_packet_login_fail fail{};
            fail.size = sizeof(fail);
            fail.type = S2C_P_LOGIN_FAIL;
            session->registerSend(&fail);
            break;
        }
        std::cout << "[LOGIN] Completion queued session=" << sessionId << std::endl;

		if (update_user_info(session->_userInputId, session->object._x, session->object._y, session->object._seedNum,
			session->object._hp, session->object._level, session->object._exp, session->object._maxHp, session->object._damage)) {
			//cout << "없뎃성공" << endl;
		}
		else cout << "업뎃실패. " << endl;
	}
	break;
	case C2S_P_MOVE:
	{
		cs_packet_move* p = reinterpret_cast<cs_packet_move*>(packetFromC);

		short oldX = session->object._x;
		short oldY = session->object._y;
		short x = session->object._x;
		short y = session->object._y;

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
				session->object._seedNum++;
				session->sendStatChange(4);
				map_data[y][x] = rand() % 2;
				break;
			case 5:
				cout << "똥 밟음 -HP" << endl;
				if (session->object._hp > 0)
					session->object._hp--;
				session->sendStatChange(5);
				map_data[y][x] = rand() % 2;
				break;
			case 6:
				cout << "potion Get" << endl;
				if (session->object._hp < session->object._maxHp)
					session->object._hp++;
				session->sendStatChange(6);
				map_data[y][x] = rand() % 2;
				break;
			}
			session->object._x = x;
			session->object._y = y;
		}
		else {
			//cout << " 이동 불가.. " << endl;
			session->object._x = oldX;
			session->object._y = oldY;
		}

		session->updateSector(oldX, oldY);
		unordered_set<int> _near_vlist = session->getNearBySessions();
		unordered_set<int> nearMonsterList = _near_vlist;

		session->_vlL.lock();
		unordered_set<int> _oldVl = session->_viewList;	// 지금 유저의. 이전 시야를 백업해둔다..
		session->_vlL.unlock();

		session->sendMovePacket(sessionId, p->move_time);	// 나한테 나 움직였다고 알리기
		session->sendMapInfo(session->object._id, session->object._x, session->object._y);

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
				wakeUpMonster(near_other->object._id, session->object._id);	// 깨운다(왜냐면 npc니까.. 얼려진상태로 있는게 디폴트임)
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
					old_other->monster._isActive = false;
			}
		}

		vector<shared_ptr<GAMESESSION>> targets;
		int myX = session->object._x;
		int myY = session->object._y;
		for (int id : nearMonsterList) {
			if (isPlayer(id)) continue;
			if (targets.size() >= MAX_NEAR_MONSTERS) break;

			auto it = clients.find(id);
			if (it == clients.end()) continue;

			std::shared_ptr<GAMESESSION> monster = it->second;

			int dx = abs(monster->object._x - myX);
			int dy = abs(monster->object._y - myY);

			if (dx <= 2 and dy <= 2) targets.push_back(monster);
		}

		//cout << " 공격가능 몬스터 수 : " << targets.size() << endl;
		int tempCnt = 0;
		const int cnt = static_cast<int>(targets.size());
		nearMonsterInfo nearMons[MAX_NEAR_MONSTERS]{};
		for (auto& target : targets) {
			nearMons[tempCnt].monHp = target->object._hp;
			nearMons[tempCnt].monId = target->object._id;
			nearMons[tempCnt].monType = target->monster._monType;
			tempCnt++;
		}
		session->sendMonsterNearPacket(nearMons, cnt);


	}
	break;
	case C2S_P_ATTACK:
	{
		cout << "seed : " << session->object._seedNum << endl;
		unordered_set<int> viewList = session->getNearBySessions();
		vector<shared_ptr<GAMESESSION>> targets;

		int myX = session->object._x;
		int myY = session->object._y;

		for (int id : viewList) {
			if (isPlayer(id)) continue;
			if (targets.size() >= MAX_NEAR_MONSTERS) break;

			auto it = clients.find(id);
			if (it == clients.end()) continue;

			std::shared_ptr<GAMESESSION> monster = it->second;
			int dx = abs(monster->object._x - myX);
			int dy = abs(monster->object._y - myY);

			if (dx <= 2 and dy <= 2) targets.push_back(monster);	// 범위 설정
		}

		//cout << "주변 몬스터 수 : " << targets.size() << endl;
		for (auto& target : targets) {
			target->object._hp -= session->object._damage;	// 데미지 깎고..
			session->object._seedNum--;				// seed 갯수 줄이기
			if (target->object._hp <= 0) {
				cout << target->object._id << " 사망 " << endl;
				target->object._hp = 0;
				target->monster._isDie = true;
				target->monster._isActive = false;
				session->sendMonsterDiePacket(target->object._id);
				session->sendAttackPacket_PtoN(target->object._id, target->object._hp);
			}
			else {
				session->sendAttackPacket_PtoN(target->object._id, target->object._hp);
				auto path = findPathAStar(target->object._x, target->object._y, session->object._x, session->object._y);
				if (!path.empty() && path.size() > 1) {
					// path[0] = 현재 위치, path[1] = 다음 목적지
					int next_x = path[1].first;
					int next_y = path[1].second;
					target->monster._nextMonX = next_x;
					target->monster._nextMonY = next_y;
					session->sendMovePacket(target->object._id);
					target->monster._isMonsterChase = true;

					// 타이머에 chase 등록
					int idx = target->object._id % TIMER_THREAD_COUNT;
					std::lock_guard<std::mutex> guard(timerLocks[idx]);
					timerQueues[idx].emplace(TIMER_EVENT{ target->object._id,
						chrono::high_resolution_clock::now() + 100ms, EV_CHASE, session->object._id });
				}
			}
		}

		int tempCnt = 0;
		const int cnt = static_cast<int>(targets.size());
		nearMonsterInfo nearMons[MAX_NEAR_MONSTERS]{};
		for (auto& target : targets) {
			nearMons[tempCnt].monHp = target->object._hp;
			nearMons[tempCnt].monId = target->object._id;
			nearMons[tempCnt].monType = target->monster._monType;
			tempCnt++;
		}
		session->sendMonsterNearPacket(nearMons, cnt);
		if (update_user_info(session->_userInputId, session->object._x, session->object._y, session->object._seedNum,
			session->object._hp, session->object._level, session->object._exp, session->object._maxHp, session->object._damage)) {
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
				newPlayer->object._x = 0;
				newPlayer->object._y = 0;
				newPlayer->object._id = clientId;
				newPlayer->object._hp = 0;
				newPlayer->object._exp = 0;
				newPlayer->object._level = 0;
				newPlayer->object._name[0] = 0;
				newPlayer->_prevRemainData = 0;
				newPlayer->_socket = gCliSocket;

				clients[clientId] = newPlayer;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(gCliSocket), iocp, clientId, 0);
				if (!newPlayer->registerRecv()) disconnect(clientId);
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
			//cout << " workerThread (OP_RECV) 확인용 " << key << " " << cli->object._id << endl;
			char* p = recv_over->_sendBuf;
			bool invalidPacket = false;
			while (remainData > 0) {
				const auto packetSize = static_cast<std::uint8_t>(p[0]);
				if (packetSize < PACKET_HEADER_SIZE || packetSize > BUF_SIZE) {
					invalidPacket = true;
					break;
				}
				if (packetSize > remainData) break;
				if (!is_valid_client_packet(static_cast<std::uint8_t>(p[1]), packetSize)) {
					invalidPacket = true;
					break;
				}
				processPacket(static_cast<int>(key), p);
				p += packetSize;
				remainData -= packetSize;
			}
			if (invalidPacket) {
				disconnect(static_cast<int>(key));
				continue;
			}
			cli->_prevRemainData = remainData;

			if (remainData > 0) {
				memcpy(overEx->_sendBuf, p, remainData);
			}
			if (!cli->registerRecv()) disconnect(static_cast<int>(key));
		}
		break;
		case OP_LOGIN_OK:
		{
			//cout << " 로그인 성공 !!!! " << endl;
			auto it = clients.find(key);
			std::shared_ptr<GAMESESSION> session = it->second;

            if (session->disconnectStarted || session->transportFailed || session->_state != ST_ALLOC) {
                delete overEx;
                break;
            }
            session->sendSessionLoginPacket();	// 나한테 정보전송 -> 그래야 내가본다
			session->sendSessioLoginOkPacket();
            if (session->transportFailed || session->disconnectStarted) {
                delete overEx;
                disconnect(static_cast<int>(key));
                break;
            }
            session->_state = ST_INGAME;
            std::cout << "[LOGIN] Success sends accepted session=" << key << std::endl;
            delete overEx;
			session->sendMapInfo(session->object._id, session->object._x, session->object._y); // 맵 정보

			auto [sx, sy] = getSectorLocation(session->object._x, session->object._y);	// 로그인 후.. 나를 섹터에 등록
			{
				std::lock_guard<std::mutex> lock(sectorLock[sx][sy]);
				sectors[sx][sy].insert(session->object._id);
			}

			// 이제 브로드캐스팅
			std::unordered_set<int> _near = it->second->getNearBySessions();
			for (auto& pl : _near) {	// 그리고 나서.... -> 로그인한유저 근처에 있는 애들한테 정보보내기
				auto it2 = clients.find(pl);
				std::shared_ptr<GAMESESSION> other = it2->second;
				if (ST_INGAME != other->_state) continue;	// 접속중인애들한테만!!

				if (other->object._id == session->object._id) // 나는 제외
					continue;
				if (false == isCanSee(session->object._id, other->object._id))	// 내가 안보여도 제외
					continue;

				if (isPlayer(other->object._id)) // 걔가 플레이어면?
					other->sendAddPlayerPacket(session->object._id);	// 상대한테 내가왔다고 알린다.
				else if (isMonster(pl)) {
					wakeUpMonster(other->object._id, session->object._id);	// 깨운다(왜냐면 npc니까..     얼려진상태로 있는게 디폴트임)
				}
				session->sendAddPlayerPacket(other->object._id);	//이제 나한테.. 상대방정보 알리기
			}
		}
		break;
		case OP_LOGIN_FAIL:
		{

		}
		break;
		case OP_SEND:
		{
            delete overEx;
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

			if (not npc->monster._isActive) break;
			//if (npc->_is_npc_run) break;

			npc->monster._ll.lock();
			auto L = npc->monster._L;
			if (L == nullptr) {
				cout << "[ERROR] npc->monster._L is nullptr!" << endl;
				npc->monster._ll.unlock();
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
			npc->monster._ll.unlock();
			delete overEx;
		}
		break;
		case OP_MONSTER_MOVE:
		{
			using namespace chrono;

			auto it_mon = clients.find(key);
			std::shared_ptr<GAMESESSION> monster = it_mon->second;

			int monsterId = static_cast<int>(key);

			if (not monster->monster._isMonsterChase) {
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
			if (not monster->monster._isActive) break;
			monster->monster._isMonsterChase = true;

			monster->object._x = monster->monster._nextMonX;
			monster->object._y = monster->monster._nextMonY;

			auto it = clients.find(targetPlayerId);
			std::shared_ptr<GAMESESSION> player = it->second;
			player->sendMovePacket(monster->object._id);

			if (isMonSee(monster->object._id, targetPlayerId)) {
				shared_ptr<GAMESESSION> player = clients[targetPlayerId];
				auto path = findPathAStar(monster->object._x, monster->object._y, player->object._x, player->object._y);
				if (!path.empty() && path.size() > 1) {
					monster->monster._nextMonX = path[1].first;
					monster->monster._nextMonY = path[1].second;

					int idx = monster->object._id % TIMER_THREAD_COUNT;
					std::lock_guard<std::mutex> guard(timerLocks[idx]);
					timerQueues[idx].emplace(TIMER_EVENT{ monster->object._id, chrono::high_resolution_clock::now() + 1s
						, EV_CHASE, targetPlayerId });
				}
			}
			else if (not isMonSee(monster->object._id, targetPlayerId)) {
				//cout << " 시야에서 사라져서 쫓아가기 중단.. " << endl;
				monster->monster._isMonsterChase = false;
				int idx = monster->object._id % TIMER_THREAD_COUNT;
				timerQueues[idx].emplace(TIMER_EVENT{ monster->object._id, chrono::high_resolution_clock::now() + 1s
					, EV_RANDOM_MOVE, 0 });
			}
			break;
		}
		}

	}
}


// main =============================================================
int main(int argc, char* argv[])
{
    const bool startupCheck = argc == 2 && std::strcmp(argv[1],"--startup-check") == 0;
    int count = 0;
    char* setting = nullptr;
    size_t settingLength = 0;
    const auto settingError = _dupenv_s(&setting,&settingLength,"GMSEV_MONSTER_COUNT");
    const bool validSetting = settingError == 0 && startup_config::monsterCount(setting,count);
    std::free(setting);
    if (!validSetting) {
        std::cerr << "[STARTUP] GMSEV_MONSTER_COUNT must be an integer from 0 to 200000.\n";
        return 1;
    }
	if (!startupCheck && !initializeDB()) {
		cout << " DB연결 실패..." << endl;
		return -1;
	}
	else if (!startupCheck) cout << " DB연결 성공!" << endl;

    if (startupCheck) std::cout << "[STARTUP] Check mode: no DB or network.\n";
    std::cout << "[STARTUP] Club House is a safe room; monsters disabled (requested=" << count << ")." << std::endl;
    const bool monstersReady = monsterInit(0);
    if (monstersReady) initializeMap();
    if (startupCheck || !monstersReady) {
        for (const auto& entry : clients) {
            if (entry.second && entry.second->monster._L) {
                lua_close(entry.second->monster._L);
                entry.second->monster._L = nullptr;
            }
        }
        clients.clear();
        if (!startupCheck) finalizeDB();
        return monstersReady ? 0 : 1;
    }

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	gServerSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(GAME_PORT);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	    if (bind(gServerSocket,reinterpret_cast<sockaddr*>(&server_addr),sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "[STARTUP] Bind failed error=" << WSAGetLastError() << std::endl;
        closesocket(gServerSocket); WSACleanup(); finalizeDB(); return 1;
    }
	    if (listen(gServerSocket,SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[STARTUP] Listen failed error=" << WSAGetLastError() << std::endl;
        closesocket(gServerSocket); WSACleanup(); finalizeDB(); return 1;
    }
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);


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

    std::cout << "[READY] Server ready. Login is available on port " << GAME_PORT << std::endl;
	for (auto& th : worker_threads)
		th.join();

	closesocket(gServerSocket);
	WSACleanup();
}
