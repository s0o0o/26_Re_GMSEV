# 2B-2B1: 일반 클라이언트의 지역 내부 상태

2026-11-15 기준a7bddf1 / #50. 상태: 2B-2B1 검증 완료.

목적: 일반클라가 v2 local좌표를 전역좌표로 바꿔 보관하는 임시어댑터를 제거한다. OBJECT.position(region,x,y)이 위치 단일원본이며 카메라·맵chunk·주민보간·월드렌더도local이다. 같은(10,10)의 마을/필드는 region으로 구분한다. 서버목적지결정/프로토콜v2/DB X,Y는 유지한다.

범위: 일반client.cpp, local camera overload, region명시 renderer, 생산클라loopback/렌더회귀. 서버canonical상태·이동/AI와 기존GUIstress adapter는 후속2B-2B2이므로 전체2B-2완료로보고하지않는다. 기존 미리보기의legacy renderer호출은 명시호환wrapper로만유지한다.

- AC-01: local카메라를두지역 전체칸/경계에서검사. actor상태에region/local만있고 기존 m_x/m_y와 native LegacyPacketAdapter사용없음. packet/body검증전상태변경없음.
- AC-02: 본인region전환이면 이전players·map·combat/near목록정리, 새local좌표/카메라 적용. 같은regionMOVE는기존주민보간유지. 다른region의타인ENTER/MOVE/MAP은무시하며 본인의초기정보/ENTER/MOVE만region전환허용. 존재하지않는객체MOVE는생성하지않음.
- AC-03: 생산클라loopback에서 같은local의다른지역/전환정리/뒤늦은다른지역패킷/invalid버전·좌표 방어. field와clubhouse renderer실제출력을region/local로생성하고육안확인. 주민idle/산책과기존화면/커마회귀보존.
- AC-04: 전체회귀·일반클라Debug/Release·생산렌더러검사. 변경없는서버/GUI도구·실제DB/OS창/부하는실행하지않았다면미검증표시. 이슈·기록·main·미리보기·다음작업연결.

지역전환에새세대번호는추가하지않는다. TCP의순서보장은현재연결내에서만의근거이며모든과거비동기이벤트를클라가판별한다는뜻이아니다. 위치없는전투/채팅패킷의region태그추가는범위밖. rollback은바이너리교체만필요하며 v2 wire와DB데이터는동일하다.

## 구현/실행 연결

- AC-01: client.cpp OBJECT.position만보관, native LegacyPacketAdapter/m_x/m_y/offset판정 제거. world_layout.hpp Position 카메라 overload. world_regions_test.cpp 두지역전칸카메라범위/기존화면위치동일성/invalid검사 통과.
- AC-02: moveAvatar가region변경의정리담당. 생산loopback에서 players/map/combat/near초기화, 다른지역MOVE/ENTER/MAP무시,같은local다른region전환과invalid상태불변 통과. NPC State는지역변경시보간리셋,같은지역산책보간보존.
- AC-03: login_screens_preview.cpp, town_client_test.cpp, world_regions_preview.cpp. 생산파서/renderer지역명시경로·주민움직임·월드캐릭터·F11/화면크기 검사 통과. 미리보기이미지생성/육안확인, 실제DB/일반exe OS캡처아님.
- AC-04: 전체38회귀 통과. 클라Debug/Release최종재빌드 통과. live_client_scene.cpp는새상태접근으로갱신하고compile-only 통과, 실제DBscene실행아님. 변경없는서버/GUI스트레스는이번별도재빌드/실제DB부하없음.

[46단계 설명](../refactoring-step-46.md), [검증JSON](../test-results/client-region-state-2026-11-15.json). 서버전환2B-2B2와GUI도구후속은남아있다.
