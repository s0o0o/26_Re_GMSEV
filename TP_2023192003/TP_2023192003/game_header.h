#pragma once
constexpr short GAME_PORT = 3000;

constexpr short MAX_CHAT_LENGTH = 30;

constexpr int  MAX_USER = 10000;
constexpr int  NUM_MONSTER = 200000;
//constexpr int  NUM_MONSTER = 20000;	// 임시로..

constexpr short SCREEN_WIDTH = 18;
constexpr short SCREEN_HEIGHT = 16;

//SC로 시작하는건 서버에서 클라로 날려보내는거 타입 확인용
// ->즉 클라에서 프로세스 패킷에 case 부분에 들어가야함

//CS로 시작하는건 클라에서 날아온거 타입 확인용->즉 서버에서 프로세스 패킷 부분에 있는애들..

constexpr char S2C_P_AVATAR_INFO = 1;
constexpr char S2C_P_MOVE = 2;		// 움직였을 때
constexpr char S2C_P_ENTER = 3;		// 이게 add인듯
constexpr char S2C_P_LEAVE = 4;

constexpr char C2S_P_LOGIN = 5;
constexpr char C2S_P_MOVE = 6;

constexpr char S2C_P_CHAT = 7;
constexpr char S2C_P_STAT_CHANGE = 8;
constexpr char S2C_P_LOGIN_FAIL = 9;

constexpr char C2S_P_ATTACK = 10;
constexpr char C2S_P_CHAT = 11;
constexpr char C2S_P_TELEPORT = 12;		// 동접 테스트 할 때
										// 시작마을의 HOTSPOT을 방지하기 위해 
										// RANDOM TELEPORT할 때 사용
constexpr char S2C_P_AVATAR_LOGIN_OK = 13;
constexpr char S2C_P_ATTACK_PtoN = 14;	// 플레이어가 몬스터 때릴때
constexpr char S2C_P_ATTACK_NtoP = 15;	// 몬스터가 플레이어 때릴때
constexpr char S2C_P_MONSTER_DIE = 16;
constexpr char S2C_P_MONSTER_NEAR = 17;
constexpr char S2C_P_MAPINFO = 18;


constexpr char MAX_ID_LENGTH = 20;

constexpr char MOVE_UP = 1;
constexpr char MOVE_DOWN = 2;
constexpr char MOVE_LEFT = 3;
constexpr char MOVE_RIGHT = 4;

constexpr unsigned short MAP_HEIGHT = 2000;
constexpr unsigned short MAP_WIDTH = 2000;

//int mapObstacleData[MAP_HEIGHT][MAP_WIDTH];


// 따로 추가한..
// SECTOR
constexpr int SECTOR_SIZE = 20;	// 섹터사이즈
constexpr int MAX_SECTOR_X = (MAP_WIDTH + SECTOR_SIZE) / SECTOR_SIZE;
constexpr int MAX_SECTOR_Y = (MAP_HEIGHT + SECTOR_SIZE) / SECTOR_SIZE;
constexpr int TIMER_THREAD_COUNT = 8;

constexpr int NAME_SIZE = 20;
constexpr int BUF_SIZE = 300;


int map_data[MAP_HEIGHT][MAP_WIDTH];


bool playerCanMove(int x, int y) {
	if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
		return false;
	if (map_data[y][x] == 3) return false;
	else return true;
}



#pragma pack (push, 1)

// 서버에서 날라온거(클라에서 까는용)
struct sc_packet_avatar_info {	// 이게 로그인
	unsigned char size;
	char type;
	long long  id;
	short x, y;
	short max_hp;
	short hp;
	short level;
	int   exp;
	int	  seedNum;
};

struct sc_packet_avatar_login_ok {
	unsigned char size;
	char type;
	long long  id;
};

struct sc_packet_move {		// 움직였을 때
	unsigned char size;
	char type;
	long long id;
	short x, y;
	unsigned move_time;
};

struct sc_packet_enter {	// 새로 들어왔을 때
	unsigned char size;
	char type;
	long long  id;
	char name[MAX_ID_LENGTH];

	int monType;						
	short x, y;
};

struct sc_packet_leave {	// 나갔을 때 
	unsigned char size;
	char type;
	long long  id;
};

struct sc_packet_chat {			// 채팅
	unsigned char size;
	char type;
	int  id;						// 메세지를 보낸 Object의 ID
										// -1 => SYSTEM MESSAGE
										//       전투 메세지 보낼때 사용
	char message[MAX_CHAT_LENGTH];		// NULL terminated 문자열
};


struct sc_packet_stat_change {
	unsigned char size;
	char type;
	long long  id;
	short hp;
	short level;
	int   exp;
	int   seedNum;
	int	  mapType;
};

struct sc_packet_login_fail {
	unsigned char size;
	char type;
	long long  id;
	char reason;			// 0 : 알수 없는 이유
							// 1 : 다른 클라이언트에서 사용중
							// 2 : 부적절한 ID (특수문자, 20자 이상)
							// 3 : 서버에 동접이 너무 많음
};

struct sc_packet_attack_success {
	unsigned char size;
	char type;
	int  targetId;
	int  attackerId;
	short targetHp;
	int   seedNum;
};

struct sc_packet_monster_die {
	unsigned char size;
	char type;
	int id;
};

struct nearMonsterInfo {
	int monId;
	int monHp;
	int monType;
};

struct sc_packet_monster_near {
	unsigned char size;
	char type;
	int monCount;
	int monType;
	nearMonsterInfo monsters[5];
};


struct sc_packet_mapInfo {
	unsigned char size;		
	char type;
	short left_x,top_y;
	char tiles[4][SCREEN_WIDTH]; // y,x 순
};

struct sc_packet_init_map_obstacle_packet {
	unsigned char size;
	char type;
	short left_x;
	short top_y;
	int tiles[SCREEN_HEIGHT][SCREEN_WIDTH]{}; // 시야 범위
};

/////////////////////////////////////////////////////
///// 클라에서 날아온거 
struct cs_packet_login {
	unsigned char  size;
	char  type;
	char  name[MAX_ID_LENGTH];

	int inputId;		// 서버용id랑 클라가 입력한id구분용
};

struct cs_packet_move {
	unsigned char  size;
	char  type;
	char  direction;
	unsigned move_time;
};

struct cs_packet_attack {	// 공격
	unsigned char  size;
	char  type;
	int   id;
	int   x, y;
	int	  hp;
	int   seedNum;
	int   damage;
};

struct cs_packet_chat {
	unsigned char  size;
	char  type;
	int   id;
	char  message[MAX_CHAT_LENGTH];		// NULL terminated 문자열
};

struct cs_packet_teleport {
	unsigned char  size;
	char  type;
};

#pragma pack (pop)

