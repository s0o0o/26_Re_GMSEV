# 지역 채팅 #64/#65

[구조 입구](README.md) · [최초 구현](../specs/region-chat.md) · [긴 기록/검증](../specs/chat-history.md)

## 통신과 검증

현재 #65 v6 C2S_P_CHAT/S2C_P_CHAT은 message[120]/총126bytes다. `shared/chat_rules.hpp`는 NUL 종료·1~119 UTF-8 bytes·정규 Unicode scalar/제어문자/공백-only를 검사한다. 멀티바이트를 잘라 넣지 않는다. 이전v5 로그인은거부하며서버·일반클라·스트레스도구를같이업데이트해야한다. 이름/외형/DB 필드 계약은 바꾸지 않는다.

서버 `processPacket`은 ingame/현 연결 검사 뒤 WorldStateLock 아래 decode→연결별 nextChatAt의500ms 검사→clients.snapshot의 같은 region/ingame/플레이어에게 sendChatPacket 순서다. 패킷 p.id는 신뢰하지 않고 실제 sessionId를 사용한다. 같은 지역 전체이며 시야/거리 제한 채널은 아니다. 유효성 실패는 rate 시간을 소비하지 않는다. rate 필드는 GAMESESSION 수명과 함께 사라진다. 추가 DB 호출/잠금은 없다. 서버 Lua API의 NPC 대화 경로는 별도이며 변경하지 않는다.

## 클라이언트

`ChatPanel`은 draft UTF-8/최근64개의 sf::String 기록/입력키 latch/발신대기/위치 easing을 가진다. Tab/클릭→TextEntered Unicode→UTF-8 append/코드포인트 backspace→Enter→PacketOutbox→논블로킹 TCP. Enter 전송은 서버 echo 전 기록에 낙관 추가하지 않는다. 입력중 다른 게임키를 소비하며 주민/지도/옷장/귀환 모달은 기존 우선순위다. SFML가 전달한 확정 Unicode를 처리하는 것과 실제 Windows IME 조합/포커스 동작을 직접 검증한 것은 다르다.

#65 `wrap`은한글등2칸/ASCII1칸으로42표시칸줄바꿈하며5행만그린다. 입력은마지막줄을보여주고이전부분은생략표시한다. `offset`은최신으로부터의행거리,`unread`는스크롤중새메시지수다. 수신시새로추가한행수만offset에더해현재위치를보존하고64개메시지퇴출후범위를clamp한다. 휠(패널내)/PageUp/PageDown은offset을움직이고End/최신버튼은offset·unread를0으로한다. 기존프로필영역/모달·새접속reset은유지한다. 긴월드말풍선은20표시칸의짧은미리보기만보이고기록에원문을남긴다.

S2C_P_CHAT는 bounded decode 후 알려진 avatar/players 이름과3초 월드 말풍선, 모르는 ID는 숫자 이름으로 기록만 추가한다. `players[id]`로 유령 object를 생성하지 않는다. 말풍선은 고정 화면좌표 대신 해당 캐릭터 위에 배치한다. chat record는 접속중이며 지역왕복 보존/끊김·새로그인 초기화한다.

`OBJECT::draw`는 선택적 RenderTarget을 받아 같은 배우/말풍선 생산 경로를 RenderTexture에도 그릴 수 있다. 인자 없는 일반 게임 호출은 기존 g_window를 사용한다. 숨긴 OS창의 검은 readback을 정상 캡처로 취급하지 않기 위한 테스트 seam이다.

## 실제 공간 배치

900×800 논리 뷰의 왼쪽15px/폭360px/높이174px다. `profile_layout::stackTop`은 내 프로필(기본표시/P숨김) 위에 쌓이는0~5 몬스터 프로필의 전체 top을 계산한다. `drawProfileHud`는 실제프로필·글자를 그린다. 몬스터는260×60/행간66으로 압축하고 최대5개에서도 채팅이 화면 안에 남는다.

#66 내프로필은프레임·초상화·동적글자를같이90%로줄여좌15/하785앵커를유지한다. `selfStates()`의변환후top은676.1이며stack/채팅이이를따른다. 몬스터목록은내프로필위8px기존여백,채팅은전체stack위12px로각각구분한다. 명목NAME 슬롯은변경하지않고그린결과전체에변환한다.

`drawChat`은 프로필 stackTop-12-174를 목표 y로 삼아 지수 ease-out한다. 새 프로필 수신/소멸 시 다음 프레임부터 실제 y가 이동한다. 전환중 잠깐 교차할 수 있지만 안정뒤12px 간격이다. z-order로 위에 덮는 구현이 아니다. 안내/산책HUD와주민prompt는 오른쪽으로분리했고 지도/옷장/주민대화모달은 채팅보다 위의 기존 UI 우선순위를 유지한다.

## 검증 범위

`run-chat.cmd`: 순수 UTF-8/길이/rate + SFML문자열기록/scroll anchor·eviction 규칙 + 생산 client.cpp 입력·TCP/수신·0/1/5 프로필/세 크기 fixture 렌더. `run-wardrobe.cmd`의 기존4worker/DB대역 fixture는 TCP 발신자/region/rate/NUL과119byte분할송신복원을검사한다. fixture의 지역변경/rate시각 일부는 결정적 제어이며 실제 DB/수동 OS/대규모 부하 증거가 아니다.

`tests/chat_input_probe.cpp`는생산ChatPanel만보여주는DB없는OS입력검증창이다. Enter는로컬echo만하며10분뒤자동종료한다. 확정문자이벤트를build/tests/chat-input-events.log에남기며게임서버·DB로전송하지않는다. Computer Use도구의한/영전환미지원/입력경합으로실제IME조합자동재현은미검증이다. 사용자는직접입력한기본한글이의도대로표시됨을확인했다. 로그의OS확정한글관찰/사용자기본확인을전체게임·모든조합/취소/포커스증거로확대하지않는다. 최종실행상태는명세에기록한다.

재현: x64 Native Tools Prompt의저장소루트에서 `tests/build-chat-input-probe.cmd`로빌드한뒤 `build/tests/chat_input_probe.exe`를연다. 이창은게임로그인/실제계정/DB를사용하지않는다.
