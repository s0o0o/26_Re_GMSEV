# 클라이언트 화면과 NPC/Lua

#64: [채팅 구조](chat.md)의 ChatPanel/handleChatInput/drawProfileHud가 기존 ASCII·고정좌표 채팅을 대체한다. 내 프로필 P 표시/숨김과 몬스터0~5개 stackTop에 따라 채팅창 실제 y가 이동한다. 주민prompt/탐험HUD는 오른쪽으로 분리한다. 원본이미지/NPC idle·산책은 변경하지 않는다.

근접주민대화 #61: shared/town_dialogue.hpp는정식ID/type·현재위치·같은Clubhouse·walkable/거리2·중간통로로대상을선택한다. client의townDialogueCandidates는live players의주민3개ID만조회하며정적home좌표로대화하지않는다. town_dialogue_panel.hpp는E/Enter/클릭/닫기·2페이지·idle portrait를담당한다. handleTownDialogueInput이필드지도/옷장/귀환보다앞서대화중입력을소비하고상호모달을차단한다. drawTownDialogueUi/input마다LEAVE·범위이탈을정리하고moveAvatar/끊김/새로그인에서도닫는다. 서버NPC산책/수면은계속되며로컬안내대사는LLM/Lua생성이나서버대화권위가아니다.

접속 중 탐험 #60: `field_discovery.hpp::Record`는 active+5비트 상태만 가지며 서버확정 본인 AVATAR_INFO/ENTER/MOVE 처리 뒤 observe한다. AVATAR_INFO가 세션기록을활성화하고 중복정보는기록을지우지않는다. 단순moveAvatar/키입력·타플레이어이동에서는기록하지않는다. 유효SunnyPeak walkable타일·지점Chebyshev거리2이내를방문으로본다. 거점전환은유지,gameConnectionLost/beginLoginConnection은reset. 지도는같은Record의count/visited를표시하며서버·DB·패킷·보상은변경하지않는다. 메모리기록의한계를화면에명시한다.

필드 지도 #59: `field_map_panel.hpp`와 `handleFieldMapInput`은 옷장/귀환/채팅/이동 입력보다 먼저 처리한다. 열려 있으면 게임 입력을 소비하며 네트워크 수신/월드 갱신은 계속된다(서버 일시정지 아님). disconnect/새 로그인/거점 전환에서 닫는다. M/클릭은 위치를 보내지 않는다. 실제 terrain의256×256 지도 texture를 처음 열 때1회 생성하고 현재 위치/5개공통ID/거점출구만 매 프레임 표시한다. 지도용 작은 texture는 넓은 월드 렌더 texture와 별개다. 기존 글꼴·패널·letterbox 좌표를 유지한다.

갱신: 2026-11-17, #57. [구조 입구](README.md).

주민 원본 rect는 town_npc_renderer.hpp의 명시 셀을 사용한다. Boss 정면 수염/귀, Pashmina 정면/뒤, Snoozer 수면 프레임 잘림을 수정했고 `tests/run-sprite-frames.cmd`가24경우의 셀 픽셀 완전성과 인접 프레임 혼입0을 검사한다. 원본PNG·idle/산책·2배·발기준은 그대로다. 플레이어8프레임/90외형은 기존 원본일치 검사로 유지한다. 훈스터는50×50 단일원본 전체를 쓰고52×52의 텍스처 밖 참조를 제거했다.

프로필 nameText는 `profile_layout.hpp::placeName`으로 NAME/SEED 사이 전용 nameSlot에 glyph bounds를 맞춘다. 전체 이름 데이터는 보존하며 화면 배치만 축소·중앙 정렬한다. 이전 문서의 SPEED 겹침은 실제 raster의 SEED 라벨 겹침을 잘못 부른 것이므로 [#57 명세](../specs/sprite-frame-integrity.md)에서 정정한다.

## 일반 클라이언트

[client.cpp](../../TP_2023192003_CLIENT/client_sample/client.cpp)의 main이 SFML 창·입력·로그인 단계·게임 화면을 운영한다. SFML TcpSocket의 논블로킹 송수신을 사용하며 서버와 달리 IOCP를 사용하지 않는다.

- beginLoginConnection/pumpLogin: 제한시간안에로그인요청을부분송신하고응답을누적수신한다. 실패하면커마로복귀하여선택을유지한재시도를지원한다.
- process_data: TCP바이트를길이별로조립하고is_valid_server_packet으로v5길이/지역/위치/외형을검증한다. [옷장 흐름](wardrobe.md)은 독립 draft·서버 확정 통지·실패 처리를 담당한다.
- ProcessPacket: 본인초기정보/이동,객체ENTER/LEAVE,지도,전투/채팅응답을화면상태에반영한다. 클라는전투결과의최종판정자가아니다.
- OBJECT.position은region/local단일원본. moveAvatar가본인지역변경때players/map/combat/near목록을지운다. 다른지역타인의ENTER/MOVE/MAP은적용하지않는다. 같은local좌표의다른지역도구별된다.
- client_main: local카메라/region명시배경→캐릭터/주민→전투표시/라벨/HUD렌더. [clubhouse_renderer.hpp](../../TP_2023192003_CLIENT/client_sample/clubhouse_renderer.hpp)의legacy overload는옛진단fixture용이며native클라는쓰지않는다.

[login_flow.hpp](../../TP_2023192003_CLIENT/client_sample/login_flow.hpp)는start/Customize/Connecting/Game단계전이를제한한다. [window_display.hpp](../../TP_2023192003_CLIENT/client_sample/window_display.hpp)는논리화면비율/입력좌표변환/F11전체화면·창복원에관여한다. 초광폭에서시야를더넓히는기능과단순크기맞춤은다르다.

[client_asset_paths.hpp](../../TP_2023192003_CLIENT/client_sample/client_asset_paths.hpp)가실행파일주변의자산루트를탐색한다. [hamster_customizer.hpp](../../TP_2023192003_CLIENT/client_sample/hamster_customizer.hpp)는외형선택/텍스처, [world_character.hpp](../../TP_2023192003_CLIENT/client_sample/world_character.hpp)는월드원본픽셀스케일·발기준배치를담당한다. 커마미리보기크기와월드캐릭터크기는별도다.

## NPC의 판단과 실제 행동

NPC 전체 행동을 Lua가 독자적으로 실행하는 구조는 아니다.

```text
Lua choose_direction → C++ 충돌/지역검사 → 위치반영 → 시야/패킷
타이머 만료 → IOCP에 행동 게시 → worker가 generation/대상검사 → 실행
```

[npc_script.hpp](../../TP_2023192003/TP_2023192003/npc_script.hpp)는 VM 생성·바인딩, 예산 제한 호출, 오류 격리를 담당한다. [npc.lua](../../TP_2023192003/TP_2023192003/npc.lua)는 개별 NPC의 방향 선택과 동일 칸 플레이어 인사 정책이다. C++가 이동·충돌·A*·전투·타이머·네트워크 권위를 소유한다. script 오류·예산 초과는 해당 NPC의 script 비활성화로 격리하여 서버 전체를 멈추지 않도록 한다.

API_get_x/API_get_y는 이제 지역 내부 좌표다. 같다는 이유로 지역까지 같은 것은 아니다. C++의 지역 검사와 메시지 대상 검사를 우회해 다른 지역에 알림을 보내면 안 된다. Lua 인수 검증을 RAII 잠금보다 먼저 하여 Lua 오류가 잠금을 남기지 않게 한다.

## 주민·적·부활

[town_npcs.hpp](../../shared/town_npcs.hpp)는주민종류/배치, [town_behaviour.hpp](../../shared/town_behaviour.hpp)는안전산책/대기정책이다. myserver.cpp의serviceTownResidents가관찰자/점유/허용영역을확인하고움직임과시야변경을보낸다. 잠자는주민은성격에맞는고정대기동작이며적처럼공격하지않는다.

클라의 [town_motion.hpp](../../TP_2023192003_CLIENT/client_sample/town_motion.hpp)는산책보간/방향, [town_npc_renderer.hpp](../../TP_2023192003_CLIENT/client_sample/town_npc_renderer.hpp)는스프라이트프레임/idle렌더를담당한다. 서버의자율행동과클라의시각보간은다른역할이다.

적은wakeUpMonster/trySleepMonster/doMonsterRandomMoving와OP_MONSTER_CHASE경로를사용한다. [monster_lifecycle.hpp](../../TP_2023192003/TP_2023192003/monster_lifecycle.hpp)는사망/respawn시각/generation무효화와원본spawn자료→Position복원을담당한다. doTimer의world tick에서serviceRespawns가씨앗복구/주민/몬스터부활을처리한다. generation은죽기전/이전추격이벤트가새상태에적용되지않게하는표식이다.

위치 없는 채팅·전투 메시지에 지역 전환 세대 태그를 모두 넣은 상태는 아니다. 이는 NPC의 행동 generation과 별개인 후속 개념이다. 개인 집·새 NPC·새 AI 정책·추가 지역은 별도 명세로 추가한다.

## #53 거점 이동 UI

handleTravelInput → ClientState.begin → send_packet/PacketOutbox → 서버 MOVE/결과 순서다. travel_panel.hpp가 버튼/대기/오류를 표시한다. 키 반복·채팅·timeout·종료·캐시 정리는 [귀환 구조](travel.md)에 정리했다. 주민 idle/산책 정책이나 스프라이트는 변경하지 않았다.

## #54 가시 필드·화면 밖 제외

ClubhouseRenderer의 FieldRenderer는 shared 청크 지형을 읽어 가시 타일만 그린다. client_main의 플레이어/NPC draw는 같은 지역과 한 칸 여유의 화면 범위를 통과해야 한다. 카메라는 Position의 지역 크기로 끝을 clamp한다. [청크 구조와 예산](field-chunks.md)을 참조한다. 기존 NPC idle/산책은 유지하며 새 대량 NPC를 추가한 것은 아니다.
