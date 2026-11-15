# 46단계: 일반 클라이언트 지역 내부 위치·카메라

작업 [#50](https://github.com/s0o0o/26_Re_GMSEV/issues/50), 기준a7bddf1, [사전 명세](specs/client-region-state.md).

## 기존 문제와 변경

v2는지역/내부좌표를전송했지만일반클라는수신즉시legacy로바꿔OBJECT.m_x/m_y에저장했다. 필드의10이42로변하고카메라·배경선택도32오프셋에의존했다.

- `client.cpp`: 위치의단일원본을 OBJECT.position(region,x,y)으로바꾸고일반클라의LegacyPacketAdapter를제거했다. v2패킷을검증한뒤그대로local로적용한다. moveAvatar가지역전환시players/map/combat/near목록을정리한다. 다른지역객체ENTER/MOVE/MAP은적용하지않는다. 같은지역주민보간은보존하고region변경시보간을리셋한다.
- `world_layout.hpp`: Position을받는local카메라계산을추가했다. 필드에서도camera(1,2)는1이지33이아니다. 기존int overload는서버/옛fixture가사용하므로남긴다.
- `clubhouse_renderer.hpp`: region을명시인수로받아배경/포털/라벨을local로그린다. 예전진단fixture의legacy호출은명시wrapper로만남겼으며native클라에서는사용하지않는다.
- 오래된공격요청x/y는서버가사용하지않는예약필드라0을보낸다. 전투판정위치는계속서버소유이며wire schema/버전은변하지않았다.
- `live_client_scene.cpp`도position을참조하도록갱신했다. 이번은compile-only이며실제DB scene/부하실행이아니다.

DB X/Y·스키마·패킷v2·서버게임상태/AI·GUIstress는변경하지않았다. 서버canonical상태는다음2B-2B2다. 이번만으로전체좌표마이그레이션완료라고하지않는다. CP949 client.cpp 인코딩을보존했다.

## 검증

1. 전체38회귀통과. 공유카메라검사는두지역전체칸의local화면범위와기존화면위치동일성,invalid안전성을검사했다.
2. 생산클라loopback에서local10,10의SunnyPeak와Clubhouse전환,이전객체/맵/전투/near정리,다른지역MOVE/ENTER/MAP무시,잘못된버전/좌표의상태불변을확인했다. 분할수신·실패후재시도회귀도유지.
3. 생산주민ENTER/대기·산책보간/LEAVE,필드hostile표시,알수없는타입거부검사통과. 화면크기/F11·월드캐릭터회귀통과.
4. 클라이언트x64 Debug/Release 최종재빌드통과. 초기빌드에서lodMapTexture의옛2인수move(4,4)가남아실패해명시Clubhouse Position으로수정했다. 보강한주민검사와생산로그인fixture도최종재실행통과. 기존C4244/MSB8028/LNK4098 등의경고는남아있다.
5. 생산렌더러지역명시+local카메라미리보기생성/육안확인. `build/tests/client-region-state-preview.png`는테스트출력이지실제DB접속캡처나새마을디자인이아니다. 두지역모두client local10,10/camera1,2이고DB코덱은기존좌표를유지한다.

실제DB/계정생성·저장/부하/일반exe OS입력은이번미실행. 변경없는서버/GUI도구는별도재빌드하지않았다. 이전300명실측을이번변경검증으로재사용하지않는다. 위치없는채팅/전투패킷의과거세대판별은별도범위다.

## 다음

서버GameObjectState를지역위치단일원본으로전환하고이동/충돌/AI·A*·전투/섹터가local을사용하도록정리한다(2B-2B2). DB입출력/기존맵자료경계의변환을명시하고실제worker·DB저장회귀를연결한다. 이어기존GUI도구좌표/크래시재현(#10),거점↔필드귀환,청크필드,옷장/커마순서다.
