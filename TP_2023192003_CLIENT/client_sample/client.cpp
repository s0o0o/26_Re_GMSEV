#define _CRT_SECURE_NO_WARNINGS
#include "ui_alignment.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <unordered_map>
#include <Windows.h>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <limits>
using namespace std;

#include "..\..\shared\protocol.hpp"
#include "..\..\shared\login_input.hpp"
#include "hamster_customizer.hpp"
#include "login_flow.hpp"
#include "loading_screen.hpp"
#include "clubhouse_renderer.hpp"
ClubhouseRenderer& clubhouseRenderer() { static ClubhouseRenderer value; return value; }
HamsterCustomizer& hamsterCustomizer() {
    static HamsterCustomizer instance;
    return instance;
}

sf::TcpSocket s_socket;


constexpr auto TILE_WIDTH = 50;
constexpr auto WINDOW_WIDTH = SCREEN_WIDTH * TILE_WIDTH;   // size of window
constexpr auto WINDOW_HEIGHT = SCREEN_HEIGHT * TILE_WIDTH;

int clientMapData[SCREEN_HEIGHT][SCREEN_WIDTH];

int g_left_x;
int g_top_y;
int g_myid;

int profileWidth = 520;
int profileHeight = 121;
int userTextureWidth = 48;
int userTextureHeight = 48;
int monTextureWidth = 52;

std::int32_t id_input;
sf::String inputNickname;
sf::String inputAccountId;
bool choosingNickname = false;
bool isChatMode = false;
std::string chatInputStr = "";
bool id_entered = false;

std::string toUtf8(const sf::String& value) {
	const auto bytes = value.toUtf8();
	return std::string(bytes.begin(), bytes.end());
}

sf::String fromUtf8(const char* value) {
	const char* end = value + std::strlen(value);
	return sf::String::fromUtf8(value, end);
}

bool isMonNear = true;
bool isChat = false;
bool isAttackAnim = false;
bool isGetSeed = false;
bool isMinusHp = false;
bool isGetPotion = false;
bool isKill = false;
bool isChase = false;
chrono::system_clock::time_point getSeedTime;
chrono::system_clock::time_point uiTime;
chrono::system_clock::time_point HpTime;
chrono::system_clock::time_point killTime;

sf::RenderWindow* g_window;
sf::Font& gameFont() {
    static sf::Font font;
    return font;
}
// text
sf::Text nameText;
sf::Text hpText;
sf::Text maxHpText;
sf::Text levelText;
sf::Text seedNumText;

sf::Text monHpText[5];
sf::Text monIdText[5];

sf::Text idInputText;
sf::Text chatText;

// nearMonster관해
//std::vector<nearMonsterInfo> nearMonsters;
nearMonsterInfo nearMonsters[5];
int nearMonCnt = 0;

using GAMESTATE = LoginStage;
LoginFlow loginFlow;
std::string loginStatus;
bool startConnectionRequested = false;
bool gameResourcesReady = false;
cs_packet_login pendingLogin{};
std::size_t loginBytesSent = 0;
std::size_t loginBytesReceived = 0;
sf::Clock loginClock;
LoadingPhase loadingPhase = LoadingPhase::Connecting;
sf::Clock loadingClock;
void failLogin(const char* message) {
    std::cerr << "[LOGIN] Failed: " << message << " elapsed_ms=" << loginClock.getElapsedTime().asMilliseconds()
        << " sent=" << loginBytesSent << " received=" << loginBytesReceived << std::endl;
    s_socket.disconnect();
    loginFlow.fail();
    loginStatus = message;
}
class OBJECT {
private:
	bool m_showing;
	sf::Sprite m_sprite;

	sf::Text m_name;
	sf::Text m_chat;
	chrono::system_clock::time_point m_mess_end_time;
public:
	int         id;
	int         m_x, m_y;
	char        name[NAME_SIZE];
	short		_hp, _maxHp, _level, _exp, _seedNum, _damage; // hp, 최대hp, 레벨, 경험치. 획득씨앗갯수

	bool		_isDie;
	short		_monType;

	OBJECT(sf::Texture& t, int x, int y, int x2, int y2) {
		m_showing = false;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
		set_name("NONAME");
		m_mess_end_time = chrono::system_clock::now();
	}
	OBJECT() {
		m_showing = false;
	}
	void show()
	{
		m_showing = true;
	}
	void hide()
	{
		m_showing = false;
	}

	void a_move(int x, int y) {
		m_sprite.setPosition((float)x, (float)y);
	}

	void a_draw() {
		g_window->draw(m_sprite);
	}

	void move(int x, int y) {
		m_x = x;
		m_y = y;
	}

	void draw() {
		if (false == m_showing) return;
		float rx = (m_x - g_left_x) * TILE_WIDTH + 1;
		float ry = (m_y - g_top_y) * TILE_WIDTH + 1;
		        if (id >= 0 && id < MAX_USER) {
            m_sprite.setScale(2.f,2.f);
            m_sprite.setOrigin(24.f,44.f);
            m_sprite.setPosition(rx+25.f,ry+45.f);
        } else m_sprite.setPosition(rx,ry);
		g_window->draw(m_sprite);
		auto size = m_name.getGlobalBounds();
		if (m_mess_end_time < chrono::system_clock::now()) {
			if (id != g_myid) {
				m_name.setPosition(rx + 32 - size.width / 2, ry - 25);
				g_window->draw(m_name);
			}
		}
		else {
			m_chat.setPosition(30,550);
			g_window->draw(m_chat);
		}
	}

	void set_name(const char str[]) {
		m_name.setFont(gameFont());
		m_name.setString(fromUtf8(str));
		if (id < MAX_USER) m_name.setFillColor(sf::Color(0, 0, 0));
		else m_name.setFillColor(sf::Color(rand() % 256, rand() % 256, rand() % 256));
		m_name.setStyle(sf::Text::Regular);

		if (id == g_myid) {

		}
	}

	void set_chat(int id, const char str[]) {
		m_chat.setFont(gameFont());
		string sss = to_string(id) + " : " + str;
		m_chat.setString(str);
		m_chat.setFillColor(sf::Color::Blue);
		m_chat.setStyle(sf::Text::Regular);
		m_mess_end_time = chrono::system_clock::now() + chrono::seconds(3);
	}

	sf::Sprite& get_sprite() {
		return m_sprite;
	}
};

OBJECT avatar;
unordered_map <int, OBJECT> players;

OBJECT mapType[16];

OBJECT white_tile;
OBJECT black_tile;

sf::Texture* board;
sf::Texture* map;
sf::Texture* pieces;
sf::Texture* monHamster;
sf::Texture* hunHamster;
sf::Texture* myHam;
sf::Texture* startMainTx;
sf::Texture* chatTx;
sf::Texture* hunHam;

sf::Texture* hamProfile;
sf::Texture* monProfile;
sf::Texture* hunsterProfile;

sf::Texture* getSeedTex;
sf::Sprite getSeedSprite;
sf::Texture* attackTex;
sf::Sprite attackSprite;
sf::Texture* minusHpTex;
sf::Sprite minusHpSprite;
sf::Texture* hpTex;
sf::Sprite hpSprite;
sf::Texture* killTex;
sf::Sprite killSprite;
sf::Texture* chaseTex;
sf::Sprite chaseSprite;


// sprite
sf::Sprite profileSprite;
sf::Sprite monProfileSprite;
sf::Sprite startBgSprite;
sf::Sprite mapSprite;
sf::Sprite chatSprite;

void lodMapTexture() {
	sf::Texture* map;
	map = new sf::Texture;
	map->loadFromFile("mapTex.bmp");

	for (int i = 0; i < 16; ++i) {
		mapType[i] = OBJECT{ *map, 50 * i, 0, TILE_WIDTH, TILE_WIDTH };
	}
}


void client_initialize()
{
	board = new sf::Texture;
	pieces = new sf::Texture;
	monHamster = new sf::Texture;
	hunHamster = new sf::Texture;
	myHam = new sf::Texture;
	hunHam = new sf::Texture;

	getSeedTex = new sf::Texture;
	attackTex = new sf::Texture;
	minusHpTex = new sf::Texture;
	hpTex = new sf::Texture;
	killTex = new sf::Texture;
	chaseTex = new sf::Texture;

	hamProfile = new sf::Texture;
	monProfile = new sf::Texture;
	hunsterProfile = new sf::Texture;

	board->loadFromFile("mapTex.bmp");
	pieces->loadFromFile("IDLE.png");
	monHamster->loadFromFile("monHam.png");
	hunHamster->loadFromFile("훈스터작은버전.png");
	myHam->loadFromFile("character.png");
	hamProfile->loadFromFile("profile.png");
	monProfile->loadFromFile("monProfile.png");
	hunsterProfile->loadFromFile("hunProfile.png");

	getSeedTex->loadFromFile("getSeed.png");
	attackTex->loadFromFile("attack.png");
	minusHpTex->loadFromFile("minusHp.png");
	hpTex->loadFromFile("hpGet.png");
	killTex->loadFromFile("kill.png");
	chaseTex->loadFromFile("chasing.png");

	profileSprite.setTexture(*hamProfile);
	monProfileSprite.setTexture(*monProfile);

	getSeedSprite.setTexture(*getSeedTex);
	attackSprite.setTexture(*attackTex);
	minusHpSprite.setTexture(*minusHpTex);
	hpSprite.setTexture(*hpTex);
	killSprite.setTexture(*killTex);
	chaseSprite.setTexture(*chaseTex);

	if (false == gameFont().loadFromFile("neodgm.ttf")) {
		cout << "Font Loading Error!\n";
		exit(-1);
	}
	white_tile = OBJECT{ *board, 0, 0, TILE_WIDTH, TILE_WIDTH };
	black_tile = OBJECT{ *board, 50, 0, TILE_WIDTH, TILE_WIDTH };
	avatar = OBJECT{ *myHam, userTextureWidth, 0, userTextureWidth, userTextureHeight };
    avatar.get_sprite().setTexture(hamsterCustomizer().texture(hamsterCustomizer().selected(), hamsterCustomizer().selectedEyes(), hamsterCustomizer().selectedScarf(), hamsterCustomizer().selectedCharacter()));

	// 프로필
	profileSprite.setPosition(0 + 15, WINDOW_HEIGHT - profileHeight - 15);
	avatar.move(4, 4);
}

void client_finish()
{
	players.clear();
	delete board;
	delete pieces;
}



// 서버로부터 온거 까는거 
void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case S2C_P_AVATAR_INFO:
	{
		sc_packet_avatar_info* packet = reinterpret_cast<sc_packet_avatar_info*>(ptr);
		g_myid = packet->id;
		avatar.id = g_myid;
        avatar.get_sprite().setTexture(hamsterCustomizer().texture(packet->fur_color, packet->eye_style, packet->scarf_style, packet->character_kind));

		avatar._hp = packet->hp;
		hpText.setString(to_string(avatar._hp));
		avatar._maxHp = packet->max_hp;
		maxHpText.setString(" / " + to_string(avatar._maxHp));

		avatar._level = packet->level;
		levelText.setString(to_string(avatar._level));
		avatar._seedNum = packet->seedNum;
		seedNumText.setString(to_string(avatar._seedNum));

		avatar.move(packet->x, packet->y);
		g_left_x = world_layout::cameraX(packet->x);
		g_top_y = world_layout::cameraY(packet->y);
		avatar.show();
	}
	break;
	case S2C_P_ENTER:
	{
		sc_packet_enter* my_packet = reinterpret_cast<sc_packet_enter*>(ptr);
		int id = my_packet->id;
		cout << "sc_packet_enter : " << id << endl;

		if (id == g_myid) {
			if (world_layout::outdoors(avatar.m_x) != world_layout::outdoors(my_packet->x)) memset(clientMapData,0,sizeof(clientMapData));
            avatar.move(my_packet->x, my_packet->y);
			g_left_x = world_layout::cameraX(my_packet->x);
			g_top_y = world_layout::cameraY(my_packet->y);
			avatar.show();
		}
		else if (id < MAX_USER) {
			players[id] = OBJECT{ *myHam, userTextureWidth, 0, userTextureHeight, userTextureHeight };
			players[id]._monType = my_packet->monType;
            players[id].get_sprite().setTexture(hamsterCustomizer().texture(my_packet->fur_color, my_packet->eye_style, my_packet->scarf_style, my_packet->character_kind));
			players[id].id = id;
			players[id].move(my_packet->x, my_packet->y);
			players[id].set_name(my_packet->name);
			players[id].show();

		}
		else {
			players[id]._monType = my_packet->monType;
			if (players[id]._monType == 1)
				players[id] = OBJECT{ *monHamster, monTextureWidth, 0, monTextureWidth, monTextureWidth }; // 몬스터용
			else if (players[id]._monType == 2)
				players[id] = OBJECT{ *hunHamster, 0, 0, monTextureWidth, monTextureWidth }; // 훈스터용
			players[id].id = id;
			players[id].move(my_packet->x, my_packet->y);
			players[id].set_name(my_packet->name);
			players[id].show();
		}
		break;
	}
	case S2C_P_MOVE:
	{
		//cout << " move패킷날라옴.." << endl;
		sc_packet_move* my_packet = reinterpret_cast<sc_packet_move*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			if (world_layout::outdoors(avatar.m_x) != world_layout::outdoors(my_packet->x)) memset(clientMapData,0,sizeof(clientMapData));
            avatar.move(my_packet->x, my_packet->y);
			g_left_x = world_layout::cameraX(my_packet->x);
			g_top_y = world_layout::cameraY(my_packet->y);
		}
		else {
			players[other_id].move(my_packet->x, my_packet->y);
		}
		break;
	}

	case S2C_P_LEAVE:
	{
		sc_packet_leave* my_packet = reinterpret_cast<sc_packet_leave*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.hide();
		}
		else {
			players.erase(other_id);
		}
		break;
	}
	case S2C_P_CHAT:
	{
		cout << "S2C_P_CHAT 도착 " << endl;
		sc_packet_chat* my_packet = reinterpret_cast<sc_packet_chat*>(ptr);
		int other_id = my_packet->id;

		if (other_id == g_myid) {
			avatar.set_chat(my_packet->id,my_packet->message);
		}
		else {
			players[other_id].set_chat(my_packet->id,my_packet->message);
		}

		break;
	}
	case S2C_P_AVATAR_LOGIN_OK:
	{
		cout << " S2C_P_AVATAR_LOGIN_OK 이거 날라옴!!" << endl;
		loginFlow.accept();
		break;
	}
	case S2C_P_LOGIN_FAIL:
	{
		cout << " 로그인 실패 " << endl;
		failLogin("Login rejected. Retry or go back to edit your ID.");
		break;
	}
	case S2C_P_ATTACK_PtoN:
	{
		sc_packet_attack_success* p = reinterpret_cast<sc_packet_attack_success*>(ptr);

		int monsterId = p->targetId;
		int monsterHp = p->targetHp;

		avatar._seedNum = p->seedNum;
		seedNumText.setString(to_string(avatar._seedNum));
		isAttackAnim = true;
		uiTime = chrono::system_clock::now() + chrono::milliseconds(450);
		break;
	}
	case S2C_P_MONSTER_DIE:
	{
		cout << " 몬스터 죽음" << endl;
		sc_packet_monster_die* p = reinterpret_cast<sc_packet_monster_die*>(ptr);
		int id = p->id;
		isKill = true;
		killTime = chrono::system_clock::now() + chrono::milliseconds(400);
		players.erase(id);
		break;
	}
	case S2C_P_MONSTER_NEAR:
	{
		sc_packet_monster_near* p = reinterpret_cast<sc_packet_monster_near*>(ptr);

		*nearMonsters = *p->monsters;
		nearMonCnt = p->monCount;

		for (int i = 0; i < p->monCount; ++i) {
			nearMonsters[i].monId = p->monsters[i].monId;
			nearMonsters[i].monHp = p->monsters[i].monHp;
			nearMonsters[i].monType = p->monsters[i].monType;
			//cout << "nearMonster정보 [" << nearMonsters[i].monId << "]/hp:" << nearMonsters[i].monHp
			//	<< ", type:" << nearMonsters[i].monType << endl;

			monHpText[i].setString(to_string(nearMonsters[i].monHp));
			monIdText[i].setString(to_string(nearMonsters[i].monId));
		}

		break;
	}
	case S2C_P_MAPINFO:
	{
		//cout << "S2C_P_MAPINFO 날라옴 " << endl;
		sc_packet_mapInfo* p = reinterpret_cast<sc_packet_mapInfo*>(ptr);

		g_left_x = world_layout::cameraX(avatar.m_x);
		g_top_y = world_layout::cameraY(avatar.m_y);

		const int MAP_ROW_CHUNK = 4;

		int start_y = p->top_y; // 패킷 안에 포함된 top_y 기준으로 갱신
		for (int j = 0; j < MAP_ROW_CHUNK; ++j) {
			int draw_y = p->top_y + j - g_top_y;

			if (draw_y < 0 || draw_y >= SCREEN_HEIGHT) continue;

			for (int i = 0; i < SCREEN_WIDTH; ++i) {
				const int draw_x=p->left_x+i-g_left_x;
                if (draw_x >= 0 && draw_x < SCREEN_WIDTH) clientMapData[draw_y][draw_x] = p->tiles[j][i];
				//cout << " i,j:(" << i << "," << j<< "):" << (int)p->tiles[j][i];
			}
			//cout << endl;
		}
		// 클라의 맵데이터 갱신
		break;
	}
	case S2C_P_STAT_CHANGE:
	{
		cout << "S2C_P_STAT_CHANGE 날라옴" << endl;
		sc_packet_stat_change* p = reinterpret_cast<sc_packet_stat_change*>(ptr);

		if (p->mapType == 4) {
			isGetSeed = true;
			getSeedTime = chrono::system_clock::now() + chrono::milliseconds(450);
		}
		if (p->mapType == 5) {
			isMinusHp = true;
			uiTime = chrono::system_clock::now() + chrono::milliseconds(450);
		}
		if (p->mapType == 6) {
			isGetPotion = true;
			HpTime = chrono::system_clock::now() + chrono::milliseconds(450);
		}
		avatar._seedNum = p->seedNum;
		seedNumText.setString(to_string(avatar._seedNum));

		avatar._hp = p->hp;
		hpText.setString(to_string(avatar._hp));
		avatar._level = p->level;
		levelText.setString(to_string(avatar._level));
		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

size_t in_packet_size = 0, saved_packet_size = 0;
char packet_buffer[BUF_SIZE];
void process_data(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;

	while (0 != io_byte) {
		if (0 == in_packet_size) {
			in_packet_size = static_cast<std::uint8_t>(ptr[0]);
			if (in_packet_size < PACKET_HEADER_SIZE || in_packet_size > BUF_SIZE) {
				std::cerr << "Invalid packet size: " << in_packet_size << std::endl;
                if (loginFlow.stage() == GAMESTATE::ConnectingScreen) failLogin("Invalid server packet. Rebuild server and client together.");
				s_socket.disconnect();
				in_packet_size = 0;
				saved_packet_size = 0;
				return;
			}
		}
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			if (!is_valid_server_packet(reinterpret_cast<const std::uint8_t*>(packet_buffer), in_packet_size)) {
				std::cerr << "Invalid server packet" << std::endl;
                if (loginFlow.stage() == GAMESTATE::ConnectingScreen) failLogin("Server protocol mismatch. Rebuild server and client together.");
				s_socket.disconnect();
				in_packet_size = 0;
				saved_packet_size = 0;
				return;
			}
			ProcessPacket(packet_buffer);
            if (loginFlow.stage() == GAMESTATE::CustomizeScreen) { in_packet_size = saved_packet_size = 0; return; }
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

void profileFontInit() {
	nameText.setFont(gameFont());
	nameText.setFillColor(sf::Color::Black);
	nameText.setStyle(sf::Text::Regular);
	nameText.setPosition(0 + 250, WINDOW_HEIGHT - profileHeight - 7);

	seedNumText.setFont(gameFont());
	seedNumText.setFillColor(sf::Color::Black);
	seedNumText.setStyle(sf::Text::Regular);
	seedNumText.setPosition(0 + 250 + 210, WINDOW_HEIGHT - profileHeight - 7);

	levelText.setFont(gameFont());
	levelText.setFillColor(sf::Color::Black);
	levelText.setStyle(sf::Text::Regular);
	levelText.setPosition(0 + 250, WINDOW_HEIGHT - profileHeight + 25);

	hpText.setFont(gameFont());
	hpText.setFillColor(sf::Color::Black);
	hpText.setStyle(sf::Text::Regular);
	hpText.setPosition(0 + 250, WINDOW_HEIGHT - profileHeight + 57);

	maxHpText.setFont(gameFont());
	maxHpText.setFillColor(sf::Color::Black);
	maxHpText.setStyle(sf::Text::Regular);
	maxHpText.setPosition(0 + 250 + 40, WINDOW_HEIGHT - profileHeight + 57);
}

void initStartScreenInit() {
	startMainTx = new sf::Texture;
	startMainTx->loadFromFile("main.png");
	startBgSprite.setTexture(*startMainTx);

	idInputText.setFont(gameFont());
	idInputText.setCharacterSize(30);
	idInputText.setFillColor(sf::Color::Black);
	idInputText.setPosition(250, 300);
}

void drawChat() {
	g_window->draw(chatSprite);
	g_window->draw(chatText);
}


bool temp = true;
void client_main()
{
	//cout << " 이거 호출됨 " << endl;
	char net_buf[BUF_SIZE];
	size_t   received;

	auto recv_result = s_socket.receive(net_buf, BUF_SIZE, received);
	if (recv_result == sf::Socket::Error) {
		wcout << L"Recv 에러!";
		exit(-1);
	}
	if (recv_result == sf::Socket::Disconnected) {
		wcout << L"Disconnected\n";
		exit(-1);
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);

	// 맵그리기 

    clubhouseRenderer().draw(*g_window,g_left_x,g_top_y);
    clubhouseRenderer().drawPickups(*g_window,&clientMapData[0][0],SCREEN_WIDTH,SCREEN_HEIGHT);
	temp = false;
	avatar.draw();
	for (auto& pl : players) pl.second.draw();
    clubhouseRenderer().drawLabel(*g_window,gameFont(),avatar.m_x);
	sf::Text text;
	text.setFont(gameFont());
	char buf[100];
	sprintf_s(buf, "(%d, %d)", avatar.m_x, avatar.m_y);

	g_window->draw(profileSprite); // 프로필그리는거

	for (int i = 0; i < nearMonCnt; ++i) {
		if (nearMonsters[i].monType == 1)
			monProfileSprite.setTexture(*monProfile);
		else if (nearMonsters[i].monType == 2)
			monProfileSprite.setTexture(*hunsterProfile);

		monProfileSprite.setPosition(0 + 15, 15 + 125 * i);
		g_window->draw(monProfileSprite); // 프로필그리는거


		monHpText[i].setFont(gameFont());
		monHpText[i].setFillColor(sf::Color::Black);
		monHpText[i].setStyle(sf::Text::Regular);
		monHpText[i].setPosition(0 + 250, 85 + 125 * i);

		monIdText[i].setFont(gameFont());
		monIdText[i].setFillColor(sf::Color::Black);
		monIdText[i].setStyle(sf::Text::Regular);
		monIdText[i].setPosition(0 + 250, 20 + 125 * i);

		g_window->draw(monHpText[i]);
		g_window->draw(monIdText[i]);
	}
	// 플필폰트
	profileFontInit();
	g_window->draw(nameText);
	g_window->draw(seedNumText);
	g_window->draw(hpText);
	g_window->draw(maxHpText);
	g_window->draw(levelText);



	if (isGetSeed) {
		if (getSeedTime < chrono::system_clock::now()) {
			isGetSeed = false;
		}
		else {
			getSeedSprite.setPosition(0, 0);
			g_window->draw(getSeedSprite);
		}
	}
	else if (isMinusHp) {
		if (uiTime < chrono::system_clock::now()) {
			isMinusHp = false;
		}
		else {
			minusHpSprite.setPosition(0, 0);
			g_window->draw(minusHpSprite);
		}
	}
	else if (isKill) {
		if (killTime < chrono::system_clock::now()) {
			isKill = false;
		}
		else {
			killSprite.setPosition(0, 0);
			g_window->draw(killSprite);
		}
	}
	else if (isAttackAnim) {
		if (uiTime < chrono::system_clock::now()) {
			isAttackAnim = false;
		}
		else {
			attackSprite.setPosition(0, 0);
			g_window->draw(attackSprite);
		}
	}
	else if (isGetPotion) {
		if (HpTime < chrono::system_clock::now()) {
			isGetPotion = false;
		}
		else {
			hpSprite.setPosition(0, 0);
			g_window->draw(hpSprite);
		}
	}

	if (isChase) {
		chaseSprite.setPosition(0, 0);
		g_window->draw(chaseSprite);
	}

	if (isChatMode) drawChat();


	text.setString(buf);
	g_window->draw(text);
}

void send_packet(void* packet)
{
	unsigned char* p = reinterpret_cast<unsigned char*>(packet);
	size_t sent = 0;
	s_socket.send(packet, p[0], sent);
}

void drawStartScreen(sf::RenderTarget& window, const sf::String& nickname) {
	window.draw(startBgSprite);
    sf::RectangleShape field({426,64});
    field.setPosition(237,641);
    field.setFillColor({248,248,248});
    window.draw(field);
    sf::Text fieldLabel(choosingNickname ? "NAME:" : "ID:",gameFont(),28);
    centerText(fieldLabel,{247,641,120,64});
    fieldLabel.setFillColor(sf::Color(72,20,10));
    window.draw(fieldLabel);
    sf::RectangleShape hint({650,45});
    hint.setPosition(125,550);
    hint.setFillColor({255,248,228});
    window.draw(hint);


	// 실시간 ID 입력 표시
	//gameFont().loadFromFile("neodgm.ttf");
	idInputText.setFont(gameFont());
	idInputText.setString(choosingNickname ? nickname : inputAccountId);
	idInputText.setCharacterSize(38);
	idInputText.setFillColor(sf::Color::Black);
    centerText(idInputText,{367,641,286,64});
    window.draw(idInputText);
    sf::Text prompt;
    prompt.setFont(gameFont());
    prompt.setCharacterSize(18);
    prompt.setFillColor(sf::Color::Black);
    prompt.setPosition(150, 563);
    prompt.setString(choosingNickname ?
        "Nickname (1-12 chars) / Enter: customize / Esc: account ID" :
        "Account ID (digits) - Enter to choose nickname");
    centerText(prompt,{125,550,650,45});
    window.draw(prompt);
}

void chatInit() {
	chatTx = new sf::Texture;
	chatTx->loadFromFile("chat.png");
	chatSprite.setTexture(*chatTx);

	chatText.setFont(gameFont());  // font는 기존에 사용하던 폰트
	chatText.setCharacterSize(25);
	chatText.setFillColor(sf::Color::Blue);
	chatText.setPosition(30, 600);  // 화면 하단
}


void drawGameScreen() {
	client_main();
}

void drawCustomizationScreen(sf::RenderTarget& window) {
    if (loginFlow.stage() == GAMESTATE::ConnectingScreen) {
        hamsterCustomizer().drawBackdrop(window);
        drawLoadingScreen(window,gameFont(),hamsterCustomizer().texture(hamsterCustomizer().selected(),
            hamsterCustomizer().selectedEyes(),hamsterCustomizer().selectedScarf(),hamsterCustomizer().selectedCharacter()),
            inputNickname,loadingPhase,loadingClock.getElapsedTime().asSeconds());
        return;
    }
    hamsterCustomizer().draw(window, gameFont());
    sf::Text identity(sf::String("ID " + std::to_string(id_input) + " / ") + inputNickname, gameFont(), 22);
    identity.setFillColor(sf::Color::White);
    centerText(identity,{160,563,580,40});
    window.draw(identity);
    const bool connecting = loginFlow.stage() == GAMESTATE::ConnectingScreen;
    for (int i=0;i<2;++i) {
        sf::RectangleShape button({230,52});
        button.setPosition(i == 0 ? 190.f : 480.f,610);
        button.setFillColor(connecting ? sf::Color(140,140,140) : (i == 0 ? sf::Color(255,248,228) : sf::Color(237,193,107)));
        window.draw(button);
        sf::Text text(i == 0 ? "Back (Esc)" : (connecting ? "Connecting..." : "Connect (Enter)"),gameFont(),22);
        text.setFillColor(sf::Color(72,46,37));
        centerText(text,button.getGlobalBounds());
        window.draw(text);
    }
    sf::Text status(loginStatus,gameFont(),19);
    status.setFillColor(sf::Color::White);
    centerText(status,{160,680,580,48});
    window.draw(status);
}

void beginLoginConnection(unsigned short port = GAME_PORT) {
    s_socket.disconnect();
    in_packet_size = saved_packet_size = 0;
    s_socket.setBlocking(true);
    const auto status = s_socket.connect("127.0.0.1", port, sf::seconds(2));
    s_socket.setBlocking(false);
    if (status != sf::Socket::Done) { failLogin("Cannot connect to server. Check server and retry."); return; }
    if (!gameResourcesReady) { client_initialize(); gameResourcesReady = true; }
    avatar.get_sprite().setTexture(hamsterCustomizer().texture(hamsterCustomizer().selected(), hamsterCustomizer().selectedEyes(), hamsterCustomizer().selectedScarf(), hamsterCustomizer().selectedCharacter()));
    const auto nickname = toUtf8(inputNickname);
    pendingLogin = {};
    pendingLogin.size = sizeof(pendingLogin);
    pendingLogin.type = C2S_P_LOGIN;
    pendingLogin.inputId = id_input;
    pendingLogin.fur_color = hamsterCustomizer().selected();
    pendingLogin.eye_style = hamsterCustomizer().selectedEyes();
    pendingLogin.scarf_style = hamsterCustomizer().selectedScarf();
    pendingLogin.character_kind = hamsterCustomizer().selectedCharacter();
    strncpy_s(pendingLogin.name,nickname.c_str(),MAX_ID_LENGTH-1);
    avatar.set_name(pendingLogin.name);
    nameText.setString(fromUtf8(pendingLogin.name));
    loginBytesSent = 0;
    loginBytesReceived = 0;
    std::cout << "[LOGIN] Connected; request bytes=" << sizeof(pendingLogin) << std::endl;
    loginClock.restart();
    loadingPhase = LoadingPhase::Sending;
    loginStatus = "Connecting to your hamster world...";
}

void pumpLogin(sf::Time timeout = sf::seconds(10)) {
    if (loginClock.getElapsedTime() > timeout) {
        std::cerr << "[LOGIN] Timeout sent=" << loginBytesSent << "/" << sizeof(pendingLogin)
            << " received=" << loginBytesReceived << " partial=" << saved_packet_size << std::endl;
        failLogin(loginBytesSent < sizeof(pendingLogin) ? "Login send stalled. Check connection and retry." :
            (loginBytesReceived == 0 ? "No login response. Check server DB/logs and rebuild both apps." :
             "Incomplete login response. Check server logs and app versions."));
        return;
    }
    if (loginBytesSent < sizeof(pendingLogin)) {
        std::size_t sent = 0;
        const auto status = s_socket.send(reinterpret_cast<const char*>(&pendingLogin)+loginBytesSent,sizeof(pendingLogin)-loginBytesSent,sent);
        loginBytesSent += sent;
        if (status == sf::Socket::Error || status == sf::Socket::Disconnected) { failLogin("Login send failed. Please retry."); return; }
        if (loginBytesSent < sizeof(pendingLogin)) return;
    }
    loadingPhase = loginBytesReceived ? LoadingPhase::Receiving : LoadingPhase::Waiting;
    char buffer[BUF_SIZE];
    std::size_t received = 0;
    const auto status = s_socket.receive(buffer,sizeof(buffer),received);
    if (status == sf::Socket::Error || status == sf::Socket::Disconnected) { failLogin("Server disconnected. Please retry."); return; }
    if (received) { loginBytesReceived += received; process_data(buffer,received); }
}
int main()
{
	wcout.imbue(locale("korean"));
	gameFont().loadFromFile("neodgm.ttf");
	if (!gameFont().loadFromFile("neodgm.ttf")) {
		cout << "Font Loading Error!\n";
		exit(-1);
	}

	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "HAMSTER RPG 2023192003 김연수");
	g_window = &window;
	initStartScreenInit();
    if (!hamsterCustomizer().load("assets/characters") || !hamsterCustomizer().setBackdrop(*startMainTx)) { std::cerr << "Character texture loading failed" << std::endl; return -1; }
    if (!clubhouseRenderer().load("assets/maps/clubhouse-sheet.png") || !clubhouseRenderer().loadOutdoor("assets/maps/sunny-peak-sheet.png")) { std::cerr << "Club House asset missing" << std::endl; return -1; }
	chatInit();
	lodMapTexture();

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			if (loginFlow.stage() == GAMESTATE::startScreen) {

				if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) choosingNickname = false;
				if (event.type == sf::Event::TextEntered && !choosingNickname) {
                    if (event.text.unicode == '\b' && !inputAccountId.isEmpty()) inputAccountId.erase(inputAccountId.getSize() - 1);
                    else if (event.text.unicode >= '0' && event.text.unicode <= '9' && inputAccountId.getSize() < 10) inputAccountId += event.text.unicode;
                }
                else if (event.type == sf::Event::TextEntered) {
					if (event.text.unicode == '\b' && !inputNickname.isEmpty()) {
						inputNickname.erase(inputNickname.getSize() - 1);
					}
					else if (login_input::allowed(event.text.unicode)) {
						sf::String candidate = inputNickname;
						candidate += event.text.unicode;
						if (candidate.getSize() <= MAX_NICKNAME_CODEPOINTS &&
							toUtf8(candidate).size() < MAX_ID_LENGTH) {
							inputNickname = candidate;
						}
					}
				}

				if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
					if (!choosingNickname) {
                        if (login_input::accountId(toUtf8(inputAccountId).c_str(), id_input)) choosingNickname = true;
                    }
                    else if (!inputNickname.isEmpty()) {
						const std::string nicknameUtf8 = toUtf8(inputNickname);
						if (!login_input::nickname(nicknameUtf8.c_str(), nicknameUtf8.size() + 1)) continue;
						loginStatus.clear();
                        loginFlow.finishCredentials();
					}
				}
			}
			else if (loginFlow.stage() == GAMESTATE::CustomizeScreen) {
                hamsterCustomizer().handleEvent(event, window);
                const bool click = event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left;
                const auto point = click ? window.mapPixelToCoords({event.mouseButton.x,event.mouseButton.y}) : sf::Vector2f(-1,-1);
                if ((event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) || sf::FloatRect(190,610,230,52).contains(point)) {
                    loginFlow.back();
                    loginStatus.clear();
                } else if ((event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) || sf::FloatRect(480,610,230,52).contains(point)) {
                    if (loginFlow.connect()) { loadingPhase = LoadingPhase::Connecting; loadingClock.restart(); startConnectionRequested = true; }
                }
            }
            else if (loginFlow.stage() == GAMESTATE::GameScreen) {

				if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tab) {
					isChatMode = !isChatMode;
					if (isChatMode)
						cout << "채팅 모드 ON" << endl;
					else {
						cout << "채팅 모드 OFF" << endl;
						chatInputStr.clear();
					}
				}

				// 채팅 모드일 때 문자 입력 처리
				if (isChatMode) {
					if (event.type == sf::Event::TextEntered) {
						if (event.text.unicode == '\b' && !chatInputStr.empty()) {
							chatInputStr.pop_back();  // 백스페이스
						}
						else if (event.text.unicode >= 32 && event.text.unicode < 128) {
							chatInputStr += static_cast<char>(event.text.unicode);  // ASCII 문자만
						}

						chatText.setString("> " + chatInputStr);
					}
					else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
						if (!chatInputStr.empty()) {
							cs_packet_chat p;
							p.size = sizeof(p);
							p.type = C2S_P_CHAT;
							p.id = avatar.id;
							//strcpy_s(p.message, chatInputStr.c_str());
							
							strncpy(p.message, chatInputStr.c_str(), MAX_CHAT_LENGTH - 1);
							p.message[MAX_CHAT_LENGTH - 1] = '\0';
							std::cout << "보내는 사람: " << p.id << std::endl;
							std::cout << "보내는 메시지: " << chatInputStr << std::endl;
							std::cout << "패킷에 복사된 내용: " << p.message << std::endl;
							cout << p.message << endl;
							send_packet(&p);

							chatInputStr.clear();
						}
						isChatMode = false;

					}
				}


				if (not isChatMode and event.type == sf::Event::KeyPressed) {
					int direction = -1;
					switch (event.key.code) {
					case sf::Keyboard::Left:    // 왼
						direction = 2;
						// 예시: 왼쪽 방향 입력 시 텍스처 Rect만 변경
						avatar.get_sprite().setTextureRect(sf::IntRect(userTextureWidth * direction, 0, userTextureHeight, userTextureHeight));
						break;
					case sf::Keyboard::Right:
						direction = 3;
						avatar.get_sprite().setTextureRect(sf::IntRect(userTextureWidth * direction, 0, userTextureHeight, userTextureHeight));
						break;
					case sf::Keyboard::Up:
						direction = 0;
						avatar.get_sprite().setTextureRect(sf::IntRect(userTextureWidth * direction, 0, userTextureHeight, userTextureHeight));
						break;
					case sf::Keyboard::Down:
						direction = 1;
						avatar.get_sprite().setTextureRect(sf::IntRect(userTextureWidth * direction, 0, userTextureHeight, userTextureHeight));
						break;
					case sf::Keyboard::Space:
						if (avatar._seedNum > 0) {
							// 공격
							cs_packet_attack p;
							p.size = sizeof(p);
							p.type = C2S_P_ATTACK;
							p.id = avatar.id;
							p.hp = avatar._hp;
							p.damage = avatar._damage;
							p.x = avatar.m_y;
							p.y = avatar.m_y;
							p.seedNum = avatar._seedNum;
							send_packet(&p);
						}
						else
							cout << "no seed" << endl;
						break;
					case sf::Keyboard::Escape:
						window.close();
						break;
					}
					if (-1 != direction) {
						cs_packet_move p;
						p.size = sizeof(p);
						p.type = C2S_P_MOVE;
						p.direction = direction;
						send_packet(&p);
					}
				}
			}
		}

        window.clear();
        if (loginFlow.stage() == GAMESTATE::startScreen) {
            drawStartScreen(window, inputNickname);
        } else if (loginFlow.stage() == GAMESTATE::CustomizeScreen || loginFlow.stage() == GAMESTATE::ConnectingScreen) {
            drawCustomizationScreen(window);
            if (loginFlow.stage() == GAMESTATE::ConnectingScreen && !startConnectionRequested) pumpLogin();
        } else if (loginFlow.stage() == GAMESTATE::GameScreen) {
            client_main();
        }
		window.display();
        if (startConnectionRequested && window.isOpen()) { startConnectionRequested = false; beginLoginConnection(); }
	}
	client_finish();

	return 0;
}