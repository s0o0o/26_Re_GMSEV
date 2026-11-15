# 47단계: 서버 지역 위치와 상시 구조 문서

2026-11-16, 기준af732ff / 작업 [#51](https://github.com/s0o0o/26_Re_GMSEV/issues/51), [사전 명세](specs/server-region-state.md). 현재 구조의 입구는 [architecture/README.md](architecture/README.md)다.

## 기존 문제와 변경 흐름

일반클라와패킷은이미region/local이었지만 서버 GameObjectState는전역_x/_y를보관하고 location()에서지역을역산했다. 이단계를바꿔 **GameObjectState.position(region,x,y)** 자체를위치의단일원본으로삼았다. 옛전역필드나proxy를추가하지않았다.

1. myserver.cpp의로그인은DB restoreLegacy의Position을직접보관한다. 이동은현재Position→local한칸→지역충돌/씨앗수집→local portal→섹터/시야/패킷 순서다.
2. 시야/전투는지역검사와local거리, Lua get_x/get_y는local값을쓴다. A*는Position인수로같은지역만허용하며local경로를반환한다. 몬스터추격next와배회경계도local이다.
3. session_packets.cpp는location을v2에직접기록한다. sendMapInfo(Position)는local카메라/원점으로chunk를만든다. world_map.cpp만baked맵배열인덱스로변환한다.
4. DB update_user_position은Position을기존DB X/Y로저장한다. 기존field_content/씨앗복구/적부활원본자료만명시어댑터를사용한다. wire버전2와DB스키마는그대로다.
5. 기존legacy시나리오테스트는tests/legacy_position_fixture.hpp에서만변환한다. 새server_region_state_test는실제canonical값/지역충돌·수집/포털/A*/DB경계를직접검사한다.

myserver.cpp/pathfinder.cpp의CP949를보존했다. 기존잠금순서/동기DB와IOCP방식은바꾸지않았다. GUIstress의legacy내부좌표와과거크래시는별도후속이다. baked맵을청크필드로바꾼것이나DB region열마이그레이션완료가아니다.

## 상시 문서와 갱신 규칙

루트README에서구조문서로연결하고 IOCP·연결수명,월드/좌표/DB,클라/NPC/Lua,빌드/검증의4페이지를작성했다. 단순함수목록이아니라실제흐름과읽는순서/소유권·잠금/오류/미완료사항을연결했다. AGENTS·SDD·개발흐름에**구조변경과같은작업에서문서도갱신**하는규칙을추가했다.

문서작성스킬의독자검토를적용해대화이력없는검토자가문서만으로7가지질문에답하게했다. 통신방식·함수추적·DB동기호출·owner·좌표경계·Lua권한·검사데이터변경을구분했다. 지적된같은worker오해를수정하고 STATE/connectionId/generation 정의파일과함수위치링크,읽기어려운문장을보완했다. 재검토에서7질문모두명확하고중대모순없음을확인했으며추가지적된지역전환세대와NPC generation용어도구분했다. 코드검토/테스트를독자검토로대체한것은아니다.

## 실행 결과와 한계

- 전체39회귀통과. 실제worker/TCP+DB대역의이동/포털/시야/전투/주민/부활/stale AI/저장회귀,새local API검사를포함한다. 추가로생산Lua바인딩의API_get_x(MAX_USER)가legacy38아닌local6을반환하는검사를보강하고Lua worker검사최종재실행통과.
- 초기A*컴파일에서Position start인수와Node* start이름충돌을수정했다. 중간전투회귀가타깃5회타격기대에서실패해local myX/myY를옛combatAllowed(int,int)에넣던두곳을Position판정으로고쳤다. 시간제한을늘리거나판정을약화하지않았다.
- 서버/일반클라/기존GUIstress x64 Debug·Release6빌드와콘솔/W4 빌드통과. 생산클라loopback와지역renderer검사,DB/네트워크없는startup-check(적3/주민3)통과. 기존C4244/LNK4098/MSB8028등의경고는남아있다.
- 실제DB: 기존900001000~900001299만재사용·저장,혼합300명/15초/trace ON 1회통과. 로그인300/peak300/유지300/본인이동응답300/EOF300/저장성공반환300·실패0/클라·서버송신오류0. 총20780ms,drain474ms. 전후두범위각300,총600보존. 새계정/삭제/DDL없음.
- 측정말미미응답이동차이58은모든명령응답지연보장과구분한다. durable DB readback/장시간/다른맵·전투부하/일반exe OS입력·기존GUI크래시재현은이번미실행이다.
- `build/tests/server-region-state-preview.png`는생산GameObjectState→v2→local renderer/DBcodec를사용한**테스트출력**이며실제DB게임캡처나설계목업이아니다. 생성/육안확인했다.

구조화결과: [server-region-state-2026-11-16.json](test-results/server-region-state-2026-11-16.json). tools/check-architecture-docs.ps1로입구포함6개UTF-8문서·61개상대링크/AGENTS규칙검사통과,CP949 byte왕복/JSON파싱/git diff --check통과. 이자동검사는문서내용과코드의의미적동일성을증명하는검사는아니다.

다음: 기존GUI스트레스도구의좌표·전용계정설정을현계약과맞추고 #10 크래시재현조건/안전종료를검사한다. 이어거점↔필드귀환 → 청크필드 → 옷장/커마 순서다. #41 추가부하조건은열어둔다.
