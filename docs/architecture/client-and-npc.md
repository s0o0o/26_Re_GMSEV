# 클라이언트 화면과 NPC/Lua

갱신: 2026-11-16, #51. [구조 입구](README.md).

## 일반 클라이언트

[client.cpp](../../TP_2023192003_CLIENT/client_sample/client.cpp)의 main이 SFML 창·입력·로그인 단계·게임 화면을 운영한다. SFML TcpSocket의 논블로킹 송수신을 사용하며 서버와 달리 IOCP를 사용하지 않는다.

- beginLoginConnection/pumpLogin: 제한시간안에로그인요청을부분송신하고응답을누적수신한다. 실패하면커마로복귀하여선택을유지한재시도를지원한다.
- process_data: TCP바이트를길이별로조립하고is_valid_server_packet으로v2길이/지역/위치를검증한다.
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
