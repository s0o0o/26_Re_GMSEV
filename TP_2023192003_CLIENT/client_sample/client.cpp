#define _CRT_SECURE_NO_WARNINGS
#include "window_display.hpp"
#include "ui_alignment.hpp"
#include "profile_layout.hpp"
#include "field_investigation_panel.hpp"
#include "field_journal_panel.hpp"
FieldJournalPanel& fieldJournalPanel(){static FieldJournalPanel panel;return panel;}
FieldInvestigationPanel& fieldInvestigationPanel(){static FieldInvestigationPanel panel;return panel;}
#include "chat_panel.hpp"
ChatPanel& chatPanel() { static ChatPanel panel; return panel; }
sf::Clock chatFrameClock;
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include "world_character.hpp"
#include "client_asset_paths.hpp"
// Native objects and camera consume region-local wire coordinates directly.
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
#include "combat_hud.hpp"
#include "travel_panel.hpp"
#include "../../shared/wardrobe_client_state.hpp"
#include "wardrobe_panel.hpp"
#include "field_map_panel.hpp"
FieldMapPanel& fieldMapPanel() { static FieldMapPanel value; return value; }
field_discovery::Record fieldDiscovery;
#include "exploration_guidance_hud.hpp"
exploration_guidance::State explorationGuide;
#include "town_walk_quest_hud.hpp"
town_walk_quest::State townWalkQuest; resident_experience_quest::State residentExperienceQuest;
resident_experience_quest::State pashminaExperienceQuest{3,4,101};
#include "../../shared/resident_quest_tracking.hpp"
resident_quest_tracking::State questTracking;
resident_quest_log::Snapshot residentQuestSnapshot(){return questTracking.decorate(resident_quest_log::snapshot(fieldDiscovery.active(),townWalkQuest,fieldDiscovery,residentExperienceQuest,pashminaExperienceQuest,fieldInvestigationPanel().state),fieldDiscovery,residentExperienceQuest,pashminaExperienceQuest,fieldInvestigationPanel().state);}
const resident_experience_quest::State& guidedExperienceQuest(){return resident_experience_guidance::preferred(fieldDiscovery.active(),residentExperienceQuest,pashminaExperienceQuest,fieldInvestigationPanel().state);}
WardrobePanel& wardrobePanel() { static WardrobePanel panel; return panel; }
bool wardrobeBackdropNeeded=false;
wardrobe::ClientState wardrobeState;
#include "../../shared/packet_outbox.hpp"
world_travel::ClientState travelState;
PacketOutbox gameOutbox;
TravelPanel travelPanel;
void send_packet(void* packet);
void gameConnectionLost();
#include "town_npc_renderer.hpp"
#include "town_dialogue_panel.hpp"
TownDialoguePanel& townDialoguePanel() { static TownDialoguePanel value; return value; }
TownNpcRenderer& townNpcRenderer() { static TownNpcRenderer value; return value; }
CombatHud& combatHud() { static CombatHud value; return value; }
sf::Clock combatClock;
ClubhouseRenderer& clubhouseRenderer() { static ClubhouseRenderer value; return value; }
HamsterCustomizer& hamsterCustomizer() {
    static HamsterCustomizer instance;
    return instance;
}

sf::TcpSocket s_socket;
void gameConnectionLost() {
    extern bool isChatMode; isChatMode=false; chatPanel().reset();
    fieldJournalPanel().reset(); fieldInvestigationPanel().reset(); townWalkQuest.reset(); residentExperienceQuest.reset(); pashminaExperienceQuest.reset(); questTracking.reset(); explorationGuide.reset(); townDialoguePanel().reset(); fieldDiscovery.reset(); fieldMapPanel().reset(); wardrobeState.disconnect(); travelState.disconnect(); gameOutbox.clear(); s_socket.disconnect();
}
bool flushGamePackets() {
    const bool okay=gameOutbox.pump([](const std::uint8_t* data,std::size_t size,std::size_t& sent) {
        const auto status=s_socket.send(data,size,sent);
        if(status==sf::Socket::Error || status==sf::Socket::Disconnected) return PacketOutbox::Send::Failed;
        return status==sf::Socket::NotReady ? PacketOutbox::Send::Wait : PacketOutbox::Send::Progress;
    });
    if(!okay) gameConnectionLost();
    return okay;
}


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
	int         id=-1;
	world_regions::Position position{world_regions::Id::Clubhouse,0,0};
    town_motion::State townMotion;
    avatar_appearance::Value appearance;
    void setAppearance(avatar_appearance::Value value) {
        if(!avatar_appearance::valid(value)) return;
        appearance=value;
        m_sprite.setTexture(hamsterCustomizer().worldTexture(value.fur,value.eyes,value.scarf,value.kind));
    }
	char        name[NAME_SIZE];
	short		_hp, _maxHp, _level, _exp, _seedNum, _damage; // hp, 최대hp, 레벨, 경험치. 획득씨앗갯수

	bool		_isDie;
	short		_monType=0;

	OBJECT(const sf::Texture& t, int x, int y, int x2, int y2) {
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

	void move(world_regions::Position next) {
        if(!world_regions::valid(next)) return;
        if(position.region!=next.region) townMotion={};
        if(town_npcs::isResident(_monType)) townMotion.move(next.x,next.y,combatClock.getElapsedTime().asSeconds());
        position=next;
    }

	void draw(sf::RenderTarget* output=nullptr) {
        auto& target=output?*output:*g_window;
		if (false == m_showing) return;
        if(town_npcs::isResident(_monType)) {
            const auto seconds=combatClock.getElapsedTime().asSeconds();
            const auto position=townMotion.sample(seconds);
            townNpcRenderer().draw(target,_monType,(position.x-g_left_x)*50.f,(position.y-g_top_y)*50.f,gameFont(),name,seconds,townMotion.facing);
            return;
        }
		float rx = (position.x - g_left_x) * TILE_WIDTH + 1;
		float ry = (position.y - g_top_y) * TILE_WIDTH + 1;
		        if (id >= 0 && id < MAX_USER) {
            world_character::place(m_sprite,rx-1.f,ry-1.f);
        } else m_sprite.setPosition(rx,ry);
		target.draw(m_sprite);
		auto size = m_name.getGlobalBounds();
		if (m_mess_end_time < chrono::system_clock::now()) {
			if (id != g_myid) {
                if(id>=0 && id<MAX_USER) centerText(m_name,{rx-56.f,ry-51.f,160.f,20.f});
                else m_name.setPosition(rx + 32 - size.width / 2, ry - 25);
				target.draw(m_name);
			}
		}
		else {
			centerText(m_chat,{std::max(0.f,std::min(660.f,rx-100.f)),std::max(0.f,ry-65.f),240.f,40.f});
			target.draw(m_chat);
		}
	}

	void set_name(const char str[]) {
		m_name.setFont(gameFont());
		m_name.setString(fromUtf8(str));
        strncpy_s(name,sizeof(name),str,_TRUNCATE);
        m_name.setCharacterSize(id<MAX_USER ? 16 : 30);
        m_name.setOutlineThickness(id<MAX_USER ? 1.f : 0.f);
        m_name.setOutlineColor({255,248,228});
		if (id < MAX_USER) m_name.setFillColor(sf::Color(72,46,37));
		else m_name.setFillColor(sf::Color(rand() % 256, rand() % 256, rand() % 256));
		m_name.setStyle(sf::Text::Regular);

		if (id == g_myid) {

		}
	}

	void set_chat(int id, const char str[]) {
		m_chat.setFont(gameFont());
		string sss = to_string(id) + " : " + str;
		const auto full=fromUtf8(str); const auto preview=ChatPanel::wrap(full,20); m_chat.setString(preview.size()>1 ? preview.front()+L"..." : full); m_chat.setCharacterSize(16); m_chat.setOutlineThickness(2); m_chat.setOutlineColor({255,248,228});
		m_chat.setFillColor({72,46,37});
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
    avatar.get_sprite().setTexture(hamsterCustomizer().worldTexture(hamsterCustomizer().selected(), hamsterCustomizer().selectedEyes(), hamsterCustomizer().selectedScarf(), hamsterCustomizer().selectedCharacter()));

	// 프로필
	profileSprite.setPosition(0 + 15, WINDOW_HEIGHT - profileHeight - 15);
	avatar.move({world_regions::Id::Clubhouse,4,4});
}

void client_finish()
{
	players.clear();
	delete board;
	delete pieces;
}



// 서버로부터 온거 까는거 
town_dialogue::Candidates townDialogueCandidates() {
    town_dialogue::Candidates result{};
    for(std::size_t i=0;i<town_npcs::residents.size();++i) {
        const auto& resident=town_npcs::residents[i];const auto found=players.find(resident.id);
        if(found!=players.end()) result[i]={found->second.id,found->second._monType,found->second.position,true};
    }
    return result;
}
bool handleFieldJournalInput(const sf::Event& e,sf::Vector2f point){
    const auto log=residentQuestSnapshot();
    const bool consumed=fieldJournalPanel().handle(e,point,log.active && travelState.status!=world_travel::ClientStatus::Disconnected,
        isChatMode || townDialoguePanel().visible() || fieldInvestigationPanel().visible() || fieldMapPanel().visible() || wardrobeState.visible() || travelState.pending(),fieldDiscovery,fieldInvestigationPanel().state,log.goal.target,log);
    const int request=fieldJournalPanel().takeTrackRequest();if(request>=0)questTracking.select(request,residentQuestSnapshot());
    return consumed;
}bool handleFieldInvestigationInput(const sf::Event& e,sf::Vector2f mouse){
    return fieldInvestigationPanel().handle(e,mouse,avatar.position,fieldDiscovery.active(),
        fieldJournalPanel().visible() || isChatMode || townDialoguePanel().visible() || fieldMapPanel().visible() || wardrobeState.visible() || travelState.pending(),
        travelState.status!=world_travel::ClientStatus::Disconnected,GetTickCount64());
}
void syncResidentQuestContext() {
    residentExperienceQuest.update(fieldDiscovery.active(),fieldInvestigationPanel().state);
    pashminaExperienceQuest.update(fieldDiscovery.active(),fieldInvestigationPanel().state);
    townDialoguePanel().setQuestContext(townWalkQuest,fieldDiscovery);
    if(townDialoguePanel().state.type==101)
        townDialoguePanel().setExperienceContext(pashminaExperienceQuest,fieldInvestigationPanel().state,fieldDiscovery.active());
    else if(townWalkQuest.stage()==town_walk_quest::Stage::Reported)
        townDialoguePanel().setExperienceContext(residentExperienceQuest,fieldInvestigationPanel().state,fieldDiscovery.active());
}
bool handleTownDialogueInput(const sf::Event& event,sf::Vector2f point) {
    syncResidentQuestContext();
    const bool consumed=townDialoguePanel().handle(event,point,avatar.position,townDialogueCandidates(),
        fieldJournalPanel().visible() || fieldInvestigationPanel().visible() || isChatMode || wardrobeState.visible() || fieldMapPanel().visible() || travelState.pending(),
        travelState.status!=world_travel::ClientStatus::Disconnected);
    if(townDialoguePanel().takeGuideRequest()) {explorationGuide.select(fieldDiscovery);townDialoguePanel().showGuide(explorationGuide);}
    if(townDialoguePanel().takeQuestRequest()) {
        const bool experienceReply=townDialoguePanel().state.type==101 || townWalkQuest.stage()==town_walk_quest::Stage::Reported;
        const bool boss=townDialoguePanel().visible() && townDialoguePanel().state.type==100 && townDialoguePanel().state.target==town_npcs::baseId;
        if(townDialoguePanel().state.type==101) {
            const bool pashmina=townDialoguePanel().visible() && townDialoguePanel().state.target==town_npcs::baseId+1;
            if(pashminaExperienceQuest.stage()==town_walk_quest::Stage::Available)pashminaExperienceQuest.accept(fieldDiscovery.active(),pashmina,true,fieldInvestigationPanel().state);
            else pashminaExperienceQuest.report(fieldDiscovery.active(),pashmina,fieldInvestigationPanel().state);
        } else if(townWalkQuest.stage()==town_walk_quest::Stage::Reported) {
            if(residentExperienceQuest.stage()==town_walk_quest::Stage::Available)residentExperienceQuest.accept(fieldDiscovery.active(),boss,true,fieldInvestigationPanel().state);
            else residentExperienceQuest.report(fieldDiscovery.active(),boss,fieldInvestigationPanel().state);
        } else if(townWalkQuest.stage()==town_walk_quest::Stage::Available) {
            if(townWalkQuest.accept(fieldDiscovery,boss))explorationGuide.select(fieldDiscovery);
        } else townWalkQuest.report(fieldDiscovery,boss);
        syncResidentQuestContext();
        if(experienceReply)townDialoguePanel().showExperience();else townDialoguePanel().showQuest(townWalkQuest,fieldDiscovery);
    }
    syncResidentQuestContext();
    return consumed;
}
void drawTownDialogueUi(sf::RenderTarget& target) {
    const auto candidates=townDialogueCandidates();
    townDialoguePanel().state.reconcile(avatar.position,candidates,travelState.status!=world_travel::ClientStatus::Disconnected);
    syncResidentQuestContext();
    if(townDialoguePanel().visible()) townDialoguePanel().draw(target,gameFont(),townNpcRenderer(),combatClock.getElapsedTime().asSeconds());
    else if(!isChatMode && !wardrobeState.visible() && !fieldMapPanel().visible() && !travelState.pending()
        && travelState.status!=world_travel::ClientStatus::Disconnected) {
        const auto* c=town_dialogue::nearest(avatar.position,candidates);
        if(c) townDialoguePanel().drawPrompt(target,gameFont(),c->type);
    }
}
void moveAvatar(world_regions::Position next) {
    if(!world_regions::valid(next)) return;
    if(avatar.position.region!=next.region) {
        players.clear();
        memset(clientMapData,0,sizeof(clientMapData));
        combatHud().clear(); nearMonCnt=0;
        memset(nearMonsters,0,sizeof(nearMonsters));
    }
    fieldInvestigationPanel().state.reconcile(next,fieldDiscovery.active()); townDialoguePanel().state.reconcile(next,townDialogueCandidates(),travelState.status!=world_travel::ClientStatus::Disconnected);
    fieldMapPanel().regionChanged(next); wardrobeState.regionChanged(next); avatar.move(next);
    g_left_x=world_layout::cameraX(next); g_top_y=world_layout::cameraY(next);
}
bool handleFieldMapInput(const sf::Event& event,sf::Vector2f point) {
    return fieldMapPanel().handle(event,point,avatar.position,fieldJournalPanel().visible() || fieldInvestigationPanel().visible() || isChatMode || townDialoguePanel().visible() || wardrobeState.visible() || travelState.pending(),
        travelState.status!=world_travel::ClientStatus::Disconnected);
}

bool handleTravelInput(const sf::Event& event,sf::Vector2f mouse,bool chat) {
    const bool enabled=!travelState.pending() && travelState.status!=world_travel::ClientStatus::Disconnected;
    if(travelPanel.activate(event,mouse,chat,enabled)) {
        cs_packet_teleport packet{};
        if(travelState.begin(avatar.position,GetTickCount64(),packet)) send_packet(&packet);
        return true;
    }
    return event.type==sf::Event::KeyPressed && event.key.code!=sf::Keyboard::Escape
        && (!enabled || (!chat && event.key.code==sf::Keyboard::R));
}
bool handleWardrobeInput(const sf::Event& event,sf::RenderWindow& window,sf::Vector2f point) {
    const bool wasOpen=wardrobeState.visible();
    const auto action=wardrobePanel().handle(event,point,wardrobeState,avatar.position,avatar.appearance,
        fieldJournalPanel().visible() || fieldInvestigationPanel().visible() || isChatMode || townDialoguePanel().visible() || travelState.pending() || !wardrobeState.connected);
    if(action==WardrobePanel::Action::Opened) {
        static bool iconsReady=false;
        if(!iconsReady) iconsReady=wardrobePanel().loadIcon("assets/wardrobe/source/accessory-box-icons.png");
        wardrobeBackdropNeeded=true;
    }
    if(action==WardrobePanel::Action::Apply) {
        cs_packet_appearance packet{};
        if(wardrobeState.submit(GetTickCount64(),packet)) send_packet(&packet);
    }
    return wasOpen || wardrobeState.visible() || action!=WardrobePanel::Action::None;
}
void drawWardrobeUi(sf::RenderWindow& window) {
    const auto mouse=window.mapPixelToCoords(sf::Mouse::getPosition(window));
    if(wardrobeState.visible()) {
        // Capture the completed world backbuffer before overlay/swap, never an old post-swap buffer.
        if(wardrobeBackdropNeeded) { wardrobePanel().capture(window); wardrobeBackdropNeeded=false; }
        wardrobePanel().draw(window,gameFont(),hamsterCustomizer(),wardrobeState,mouse);
    } else if(avatar.position.region==world_regions::Id::Clubhouse)
        wardrobePanel().drawEntry(window,gameFont(),isChatMode || travelState.pending() || !wardrobeState.connected,mouse);
}
void ProcessPacket(char* ptr)
{
    if(!is_valid_server_packet(reinterpret_cast<const std::uint8_t*>(ptr),static_cast<unsigned char>(ptr[0]))) return;
	static bool first_time = true;
	switch (ptr[1])
	{
    case S2C_P_APPEARANCE_RESULT:
        wardrobeState.receive(*reinterpret_cast<const sc_packet_appearance_result*>(ptr));
        break;
    case S2C_P_APPEARANCE_CHANGE: {
        const auto& change=*reinterpret_cast<const sc_packet_appearance_change*>(ptr);
        if(change.region!=static_cast<std::uint8_t>(avatar.position.region)) break;
        if(change.id==g_myid) avatar.setAppearance(change.appearance);
        else {
            auto found=players.find(change.id);
            if(found!=players.end() && found->second.position.region==avatar.position.region)
                found->second.setAppearance(change.appearance);
        }
        break;
    }
    case S2C_P_TRAVEL_RESULT:
        travelState.receive(*reinterpret_cast<const sc_packet_travel_result*>(ptr));
        break;
	case S2C_P_AVATAR_INFO:
	{
		sc_packet_avatar_info* packet = reinterpret_cast<sc_packet_avatar_info*>(ptr);
		g_myid = packet->id;
		avatar.id = g_myid;
        avatar.setAppearance({packet->character_kind,packet->fur_color,packet->eye_style,packet->scarf_style});

		avatar._hp = packet->hp;
		hpText.setString(to_string(avatar._hp));
		avatar._maxHp = packet->max_hp;
		maxHpText.setString(" / " + to_string(avatar._maxHp));

		avatar._level = packet->level;
		levelText.setString(to_string(avatar._level));
		avatar._seedNum = packet->seedNum;
		seedNumText.setString(to_string(avatar._seedNum));

		fieldDiscovery.start();
        moveAvatar({static_cast<world_regions::Id>(packet->region),packet->x,packet->y});
        fieldDiscovery.observe(avatar.position);
            explorationGuide.update(fieldDiscovery);
            townWalkQuest.update(fieldDiscovery);
		g_left_x = world_layout::cameraX(avatar.position);
		g_top_y = world_layout::cameraY(avatar.position);
		avatar.show();
	}
	break;
	case S2C_P_ENTER:
	{
		sc_packet_enter* my_packet = reinterpret_cast<sc_packet_enter*>(ptr);
		int id = my_packet->id;
        if(id!=g_myid && my_packet->region!=static_cast<std::uint8_t>(avatar.position.region)) break;
		cout << "sc_packet_enter : " << id << endl;

		if (id == g_myid) {
			// moveAvatar owns region transition cleanup.
            moveAvatar({static_cast<world_regions::Id>(my_packet->region),my_packet->x,my_packet->y});
            fieldDiscovery.observe(avatar.position);
            explorationGuide.update(fieldDiscovery);
            townWalkQuest.update(fieldDiscovery);
			g_left_x = world_layout::cameraX(avatar.position);
			g_top_y = world_layout::cameraY(avatar.position);
			avatar.show();
		}
		else if (id < MAX_USER) {
			players[id] = OBJECT{ *myHam, userTextureWidth, 0, userTextureHeight, userTextureHeight };
			players[id]._monType = my_packet->monType;
            players[id].setAppearance({my_packet->character_kind,my_packet->fur_color,my_packet->eye_style,my_packet->scarf_style});
			players[id].id = id;
			players[id].move({static_cast<world_regions::Id>(my_packet->region),my_packet->x,my_packet->y});
			players[id].set_name(my_packet->name);
			players[id].show();

		}
        else {
            const int type=my_packet->monType;
            if(town_npcs::isResident(type)) {
                const auto rect=TownNpcRenderer::frame(type);
                players[id]=OBJECT{townNpcRenderer().texture(type),rect.left,rect.top,rect.width,rect.height};
            } else if(type==1) players[id]=OBJECT{*monHamster,monTextureWidth,0,monTextureWidth,monTextureWidth};
            else if(type==2) players[id]=OBJECT{*hunHamster,0,0,static_cast<int>(hunHamster->getSize().x),static_cast<int>(hunHamster->getSize().y)};
            else break; // Unknown NPC types must not leave an uninitialized object.
            players[id]._monType=static_cast<short>(type);
            if(town_npcs::hostile(type)) combatHud().enter(id,my_packet->hp,my_packet->max_hp);
            players[id].id=id; players[id].move({static_cast<world_regions::Id>(my_packet->region),my_packet->x,my_packet->y});
            players[id].set_name(my_packet->name); players[id].show();
        }
		break;
	}
	case S2C_P_MOVE:
	{
		//cout << " move패킷날라옴.." << endl;
		sc_packet_move* my_packet = reinterpret_cast<sc_packet_move*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			// moveAvatar owns region transition cleanup.
            moveAvatar({static_cast<world_regions::Id>(my_packet->region),my_packet->x,my_packet->y});
            fieldDiscovery.observe(avatar.position);
            explorationGuide.update(fieldDiscovery);
            townWalkQuest.update(fieldDiscovery);
			g_left_x = world_layout::cameraX(avatar.position);
			g_top_y = world_layout::cameraY(avatar.position);
		}
		else {
			if(my_packet->region!=static_cast<std::uint8_t>(avatar.position.region)) break;
            auto moving=players.find(other_id);
            if (moving != players.end()) moving->second.move({static_cast<world_regions::Id>(my_packet->region),my_packet->x,my_packet->y});
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
			players.erase(other_id); combatHud().leave(other_id);
		}
		break;
	}
    case S2C_P_CHAT: {
        const auto& packet=*reinterpret_cast<const sc_packet_chat*>(ptr);
        std::string message;if(!chat_rules::decode(packet.message,message))break;
        sf::String sender=std::to_string(packet.id);
        if(packet.id==g_myid){sender=fromUtf8(avatar.name);avatar.set_chat(packet.id,message.c_str());}
        else {auto it=players.find(packet.id);if(it!=players.end()){sender=fromUtf8(it->second.name);it->second.set_chat(packet.id,message.c_str());}}
        chatPanel().receive(sender,message);break;
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
    case S2C_P_ATTACK_FEEDBACK: {
        auto p=reinterpret_cast<sc_packet_attack_feedback*>(ptr);
        combatHud().feedback(static_cast<AttackFeedback>(p->reason),combatClock.getElapsedTime().asSeconds());
        break;
    }
    case S2C_P_ATTACK_PtoN: {
        auto p=reinterpret_cast<sc_packet_attack_success*>(ptr);
        combatHud().hit(p->targetId,p->targetHp,combatClock.getElapsedTime().asSeconds());
        for(int i=0;i<nearMonCnt;++i) if(nearMonsters[i].monId == p->targetId) {
            nearMonsters[i].monHp=p->targetHp; monHpText[i].setString(to_string(p->targetHp));
        }
        if(p->attackerId == g_myid) {
            avatar._seedNum=p->seedNum; seedNumText.setString(to_string(avatar._seedNum));
            isAttackAnim=true; uiTime=chrono::system_clock::now()+chrono::milliseconds(450);
            if(p->targetHp == 0) { isKill=true; killTime=chrono::system_clock::now()+chrono::milliseconds(400); }
        }
        break;
    }
    case S2C_P_MONSTER_DIE: {
        auto p=reinterpret_cast<sc_packet_monster_die*>(ptr);
        players.erase(p->id); combatHud().leave(p->id);
                for(int i=0;i<nearMonCnt;++i) if(nearMonsters[i].monId == p->id) {
            for(int j=i;j+1<nearMonCnt;++j) nearMonsters[j]=nearMonsters[j+1];
            --nearMonCnt;
            break;
        }
        for(int i=0;i<nearMonCnt;++i) {
            monHpText[i].setString(to_string(nearMonsters[i].monHp));
            monIdText[i].setString(to_string(nearMonsters[i].monId));
        }
        break;
    }
    case S2C_P_MONSTER_NEAR: {
        auto p=reinterpret_cast<sc_packet_monster_near*>(ptr);
        nearMonCnt=0;
        for(int i=0;i<p->monCount;++i) {
            if(p->monsters[i].monHp <= 0) continue;
            nearMonsters[nearMonCnt]=p->monsters[i];
            monHpText[nearMonCnt].setString(to_string(p->monsters[i].monHp));
            monIdText[nearMonCnt].setString(to_string(p->monsters[i].monId));
            ++nearMonCnt;
        }
        break;
    }
	case S2C_P_MAPINFO:
	{
		//cout << "S2C_P_MAPINFO 날라옴 " << endl;
		sc_packet_mapInfo* p = reinterpret_cast<sc_packet_mapInfo*>(ptr);
        if(p->region!=static_cast<std::uint8_t>(avatar.position.region)) break;

		g_left_x = world_layout::cameraX(avatar.position);
		g_top_y = world_layout::cameraY(avatar.position);

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
	profile_layout::placeName(nameText);

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

void drawProfileHud(sf::RenderTarget& out) {
	if(chatPanel().profileVisible) out.draw(profileSprite,profile_layout::selfStates()); // 프로필그리는거

	for (int i = 0; i < nearMonCnt; ++i) {
		if (nearMonsters[i].monType == 1)
			monProfileSprite.setTexture(*monProfile);
		else if (nearMonsters[i].monType == 2)
			monProfileSprite.setTexture(*hunsterProfile);

		const auto slot=profile_layout::monsterSlot(chatPanel().profileVisible,i);
        const auto textureSize=monProfileSprite.getTexture()->getSize();
        monProfileSprite.setScale(slot.width/textureSize.x,slot.height/textureSize.y);
        monProfileSprite.setPosition(slot.left,slot.top);
		out.draw(monProfileSprite); // 프로필그리는거


		monHpText[i].setFont(gameFont());
		monHpText[i].setFillColor(sf::Color::Black);
		monHpText[i].setStyle(sf::Text::Regular);
		monHpText[i].setCharacterSize(16); centerText(monHpText[i],{slot.left+114,slot.top+36,80,18});

		monIdText[i].setFont(gameFont());
		monIdText[i].setFillColor(sf::Color::Black);
		monIdText[i].setStyle(sf::Text::Regular);
		monIdText[i].setCharacterSize(16); centerText(monIdText[i],{slot.left+114,slot.top+6,80,18});

		out.draw(monHpText[i]);
		out.draw(monIdText[i]);
	}
	// 플필폰트
	if(chatPanel().profileVisible) {
	profileFontInit();
	out.draw(nameText,profile_layout::selfStates());
	out.draw(seedNumText,profile_layout::selfStates());
	out.draw(hpText,profile_layout::selfStates());
	out.draw(maxHpText,profile_layout::selfStates());
	out.draw(levelText,profile_layout::selfStates());
    }
}
bool handleChatInput(const sf::Event& event,sf::Vector2f point) {
    const bool consumed=chatPanel().handle(event,point,
        fieldJournalPanel().visible() || fieldInvestigationPanel().visible() || townDialoguePanel().visible() || fieldMapPanel().visible() || wardrobeState.visible() || travelState.pending(),
        travelState.status!=world_travel::ClientStatus::Disconnected,GetTickCount64(),[](const std::string& message) {
            cs_packet_chat packet{};packet.size=sizeof(packet);packet.type=C2S_P_CHAT;packet.id=avatar.id;
            std::memcpy(packet.message,message.data(),message.size());
            if(!gameOutbox.enqueue(&packet,sizeof(packet)))return false;
            return flushGamePackets();
        });
    isChatMode=chatPanel().editing;return consumed;
}
bool handleGameUiInput(const sf::Event& event,sf::RenderWindow& window,sf::Vector2f point) {
    // Releases/focus cleanup must reach every latch, even behind the owning modal.
    if(event.type==sf::Event::KeyReleased || event.type==sf::Event::LostFocus) {
        const bool journal=handleFieldJournalInput(event,point);
        const bool town=handleTownDialogueInput(event,point);
        const bool field=handleFieldInvestigationInput(event,point);
        const bool map=handleFieldMapInput(event,point);
        const bool wardrobe=handleWardrobeInput(event,window,point);
        const bool travel=handleTravelInput(event,point,isChatMode);
        const bool chat=handleChatInput(event,point);
        return journal || town || field || map || wardrobe || travel || chat;
    }
    return handleFieldJournalInput(event,point) || handleTownDialogueInput(event,point)
        || handleFieldInvestigationInput(event,point) || handleFieldMapInput(event,point)
        || handleWardrobeInput(event,window,point) || handleTravelInput(event,point,isChatMode)
        || handleChatInput(event,point);
}
void drawChat() {
    chatPanel().place(chatFrameClock.restart().asSeconds(),true,profile_layout::stackTop(chatPanel().profileVisible,nearMonCnt));
    chatPanel().draw(*g_window,gameFont(),travelState.status!=world_travel::ClientStatus::Disconnected);
}

bool temp = true;
void client_main()
{
	//cout << " 이거 호출됨 " << endl;
	char net_buf[BUF_SIZE];
	size_t   received;

    wardrobeState.tick(GetTickCount64()); travelState.tick(GetTickCount64());
    if(travelState.status!=world_travel::ClientStatus::Disconnected && flushGamePackets()) {
        const auto recv_result=s_socket.receive(net_buf,BUF_SIZE,received);
        if(recv_result==sf::Socket::Error || recv_result==sf::Socket::Disconnected) gameConnectionLost();
        else if(recv_result!=sf::Socket::NotReady && received>0) process_data(net_buf,received);
    }

	// 맵그리기 

    clubhouseRenderer().draw(*g_window,avatar.position.region,g_left_x,g_top_y);
    clubhouseRenderer().drawPickups(*g_window,&clientMapData[0][0],SCREEN_WIDTH,SCREEN_HEIGHT);
	temp = false;
	avatar.draw();
	    for (auto& pl : players) {
        if(!field_chunks::onScreen(pl.second.position,avatar.position.region,g_left_x,g_top_y,18,16,1)) continue;
        pl.second.draw();
        if(pl.first >= MAX_USER && town_npcs::hostile(pl.second._monType)) combatHud().drawMonster(*g_window,gameFont(),pl.first,
            (pl.second.position.x-g_left_x)*50.f+1,(pl.second.position.y-g_top_y)*50.f+1,combatClock.getElapsedTime().asSeconds());
    }
    combatHud().drawMessage(*g_window,gameFont(),combatClock.getElapsedTime().asSeconds());
    clubhouseRenderer().drawLabel(*g_window,gameFont(),avatar.position.region);
    travelPanel.draw(*g_window,gameFont(),avatar.position.region,travelState,
        g_window->mapPixelToCoords(sf::Mouse::getPosition(*g_window)),isChatMode);
	sf::Text text;
	text.setFont(gameFont());
	char buf[100];
	sprintf_s(buf, "(%d, %d)", avatar.position.x, avatar.position.y);

    drawProfileHud(*g_window);




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

	drawChat();


	text.setString(buf);
	g_window->draw(text);
    drawWardrobeUi(*g_window);
    drawTownDialogueUi(*g_window);
    if(!isChatMode && !townDialoguePanel().visible() && !fieldInvestigationPanel().visible() && !fieldMapPanel().visible() && !wardrobeState.visible() && !travelState.pending())
        fieldJournalPanel().draw(*g_window,gameFont(),avatar.position,fieldDiscovery,fieldInvestigationPanel().state,false,&guidedExperienceQuest(),residentQuestSnapshot());
    fieldInvestigationPanel().state.tick(GetTickCount64(),avatar.position,fieldDiscovery.active() && travelState.status!=world_travel::ClientStatus::Disconnected && !fieldJournalPanel().visible() && !isChatMode && !townDialoguePanel().visible() && !fieldMapPanel().visible() && !wardrobeState.visible() && !travelState.pending());
    if(!fieldJournalPanel().visible() && !isChatMode && !townDialoguePanel().visible() && !fieldMapPanel().visible() && !wardrobeState.visible() && !travelState.pending())
        fieldInvestigationPanel().draw(*g_window,gameFont(),avatar.position,fieldDiscovery.active(),GetTickCount64());
    if(!fieldJournalPanel().visible() && !fieldInvestigationPanel().visible() && !townDialoguePanel().visible() && !fieldMapPanel().visible() && !wardrobeState.visible() && !isChatMode && !travelState.pending()
        && !town_dialogue::nearest(avatar.position,townDialogueCandidates())) {
        if(!drawTrackedQuest(*g_window,gameFont(),residentQuestSnapshot(),avatar.position))
            drawExplorationGuidance(*g_window,gameFont(),explorationGuide,avatar.position);
    }
    if(fieldMapPanel().visible()) fieldMapPanel().draw(*g_window,gameFont(),avatar.position,fieldDiscovery,explorationGuide.target());
    else if(avatar.position.region==world_regions::Id::SunnyPeak) fieldMapPanel().drawEntry(*g_window,gameFont(),isChatMode || travelState.pending() || travelState.status==world_travel::ClientStatus::Disconnected,fieldDiscovery);
}

void send_packet(void* packet)
{
    const auto* p=static_cast<const std::uint8_t*>(packet);
    if(travelState.status==world_travel::ClientStatus::Disconnected) return;
    if(!gameOutbox.enqueue(packet,p[0])) gameConnectionLost();
    else flushGamePackets();
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
    chatPanel().reset(); isChatMode=false;
    fieldJournalPanel().reset(); fieldInvestigationPanel().reset(); townWalkQuest.reset(); residentExperienceQuest.reset(); pashminaExperienceQuest.reset(); questTracking.reset(); explorationGuide.reset(); townDialoguePanel().reset(); fieldDiscovery.reset(); fieldMapPanel().reset(); wardrobeState={}; travelState={}; gameOutbox.clear(); travelPanel.pressed=false; travelPanel.rDown=false;
    s_socket.disconnect();
    in_packet_size = saved_packet_size = 0;
    s_socket.setBlocking(true);
    const auto status = s_socket.connect("127.0.0.1", port, sf::seconds(2));
    s_socket.setBlocking(false);
    if (status != sf::Socket::Done) { failLogin("Cannot connect to server. Check server and retry."); return; }
    if (!gameResourcesReady) { client_initialize(); gameResourcesReady = true; }
    avatar.get_sprite().setTexture(hamsterCustomizer().worldTexture(hamsterCustomizer().selected(), hamsterCustomizer().selectedEyes(), hamsterCustomizer().selectedScarf(), hamsterCustomizer().selectedCharacter()));
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
    std::array<wchar_t,32768> executable{};
    const auto length=GetModuleFileNameW(nullptr,executable.data(),static_cast<DWORD>(executable.size()));
    std::error_code assetError;
    const auto originalCwd=std::filesystem::current_path(assetError);
    if(!length || length>=executable.size() || assetError) return 1;
    const auto assetRoot=client_assets::locate(std::filesystem::path(executable.data()),originalCwd);
    if(assetRoot.empty()) { std::cerr<<"[STARTUP] Game assets missing near executable.\n"; return 1; }
    std::filesystem::current_path(assetRoot,assetError);
    if(assetError) return 1;
    std::cout<<"[STARTUP] Game assets ready.\n";
	wcout.imbue(locale("korean"));
	gameFont().loadFromFile("neodgm.ttf");
	if (!gameFont().loadFromFile("neodgm.ttf")) {
		cout << "Font Loading Error!\n";
		exit(-1);
	}

	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "HAMSTER RPG 2023192003 김연수");
	g_window = &window;
    if(!townNpcRenderer().load("assets/npcs/source")) { std::cerr<<"Town NPC assets missing"<<std::endl; return -1; }
    WindowDisplay display(window, "HAMSTER RPG" );
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
			if (display.handle(event,window)) continue;

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
                sf::Vector2i pixel=sf::Mouse::getPosition(window);
                if(event.type==sf::Event::MouseButtonPressed || event.type==sf::Event::MouseButtonReleased)
                    pixel={event.mouseButton.x,event.mouseButton.y};
                if(event.type==sf::Event::MouseWheelScrolled)pixel={event.mouseWheelScroll.x,event.mouseWheelScroll.y};
                if(handleGameUiInput(event,window,window.mapPixelToCoords(pixel))) continue;
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
							p.x = 0; // Reserved legacy claim; server owns attack position.
							p.y = 0;
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
						cs_packet_move p{};
						p.size = sizeof(p);
						p.type = C2S_P_MOVE;
						p.direction = direction;
						send_packet(&p);
					}
				}
			}
		}

        if (!window.isOpen()) break;
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