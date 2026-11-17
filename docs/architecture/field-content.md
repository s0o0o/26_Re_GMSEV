# 필드 조사·상호작용 콘텐츠

2026-11-18 #66–68 · [구조 입구](README.md) · [3큐](../specs/field-content-queue.md) · [조사 계약](../specs/field-investigation.md)

## 탐험 수첩 #68

[수첩 계약](../specs/field-journal.md)의 `field_journal`은 기존 방문 Record와 조사 State의 inspected/completed를 읽기만 한다. `next`는 첫 체험 미완료 ID를 택한다. 거리 값은 SunnyPeak 내부 좌표의 Manhattan 거리이며 장애물을 고려한 경로 길이나 자동 이동이 아니다. 거점에서는 R로 필드에 가라는 안내를 표시한다.

`handleFieldJournalInput` → `FieldJournalPanel::handle`은 주민/조사보다 먼저 호출된다. 연결된 활성 Record가 필요하며 채팅·주민·조사·지도·옷장·귀환 중에는 열지 않는다. J fresh-key/버튼 press-release로 열고 J/Esc/닫기로 닫는다. 방향키/행 선택은 설명만 바꾸며 조사·체험 완료를 만들지 않는다. 다른 입력 helper는 수첩 visible을 차단 조건으로 사용한다. 기존 서버 패킷·월드 갱신은 멈추지 않는다.

오른쪽 395,170/490×490 패널은 다섯 장소의 방문·조사·체험을 별도 열/총계로 표시한다. 접속 중 데이터이므로 거점 왕복에는 유지하고 `gameConnectionLost`/새 로그인에서는 패널·Record·State를 초기화한다. 서버/DB 저장이나 보상은 없다. `run-field-journal.cmd`는 순수 요약과 생산 client.cpp 입력/상태 수명·읽기 불변·4상태×3크기 렌더를 검사한다. 화면 상태는 fixture로 주입한 것으로 실제 서버 탐험 전체 플레이 증거가 아니다.

#67 [상호작용 계약](../specs/field-interaction.md): State.begin은유효조사target/active·미진행/1.5초시작간격을검사한다. tick은0.9초뒤에도같은유효target이면completed5bit를기록하고그외close/cancel은진행만취소한다. 완료bit는접속수명/reset에연동되고보상/서버패킷/DB변경없다.

F/행동버튼은같은조사패널의fresh key/press-release경로다. draw직전tick에서채팅/주민/지도/옷장/귀환차단을재검사하고moveAvatar는즉시reconcile한다. LostFocus는진행취소,서버월드는계속갱신한다. draw(now)는시간비율에따른기하mark의위치·색/진행문구를생산한다. 5행동은서로다른무음관찰연출이며실제환경음재생이아니다.

`field_landmarks::all`의5ID/좌표가단일원본이다. `field_investigation::eligible`은SunnyPeak/valid/walkable/Manhattan2와중간통로를검사하고 `nearest`가동률시앞ID를선택한다. State.open은활성접속에서만target을열고별도5bit inspected에기록한다. 방문Record와구분되며영구저장·서버보상아니다.

생산client의handleFieldInvestigationInput은주민대화다음/지도·옷장·귀환·채팅/이동앞이다. 현재avatar.position은수신자기INFO/ENTER/MOVE의확정위치이며fieldDiscovery.active도필요하다. panel은E반복을거부하고클릭/닫기/Esc·입력소비/모달차단을맡는다. moveAvatar/draw마다reconcile로이탈·지역변화를정리하고disconnect/새로그인은State.reset한다. 거점왕복은조사bit보존.

오른쪽395,390의조사패널은장소별2줄설명/좌표/접속중한계·완료를보여준다. 왼쪽채팅/프로필은그대로보인다. 새이미지/패킷/DB/경제보상은없고NPC idle/월드갱신은계속된다.

#66 추가사용자요청의내프로필90%는 `profile_layout::selfStates`에서좌15/하785앵커의렌더변환으로프레임·초상화·동적글자를함께줄인다. 원본텍스처/명목NAME 슬롯/이름데이터는보존한다. `selfTop()`은676.1로바뀌며몬스터stack/채팅목표y는같은실제경계를쓴다. 채팅12px간격은유지한다.

검사: 순수 field_investigation/field_interaction/field_journal은 전체55회귀에 포함된다. run-field-content는 생산client.cpp E/F/서버패킷/모달/세크기5장소렌더·프로필변환을 검사한다. PNG는 fixture 생산렌더러 출력이며 전체게임 수동OS/실제DB/성능 증거가 아니다. 실행결과는 명세/이슈에 기록한다.
