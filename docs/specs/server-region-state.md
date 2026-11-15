# 2B-2B2 서버 지역 위치 단일원본과 상시 구조 문서

2026-11-16 기준af732ff / #51. 상태: 구현/회귀/6빌드/실제DB검증 완료. 구조 문서 독자 검토 반영.

목적: GameObjectState.position(region/local)을단일원본으로삼고서버이동·포털·시야/전투·Lua좌표·추격경로가local을쓴다. v2송신은position을직접사용한다. 기존DB X/Y와baked맵/콘텐츠자료만명시변환경계로남긴다. wire v2/DBschema/계정생성범위변경없음.

- AC-01: GameObjectState의_x/_y제거,position의region/local소유. DBrestore→position/update_user_position경계유지. 유효/invalid/같은local다른region·카메라회귀 검사.
- AC-02: 지역한정충돌/수집/지도조회,local포털왕복과시야정리·sector단일소속. AI/A*는Position인수/동일region검사/local경로, 추격next도local. baked맵/기존content는어댑터로만접근. Lua get_x/get_y는local이며기존같은region알림가드보존.
- AC-03: 기존동시성/주민/전투/부활/실제worker TCP/DB대역저장·포털·stale AI회귀,새local API검사·전체회귀. 서버Debug/Release·클라/GUI기존호환빌드와생산렌더러미리보기. 실제DB/부하는실행여부별도기록하며자동새계정/DDL없음.
- AC-04: docs/architecture/입구·IOCP·월드/DB·클라/Lua·빌드/테스트문서에실제파일/함수와추적순서·오류/동기화·미완료계획기록. AGENTS/SDD에구조변경시동일작업에서문서갱신규칙추가. 내부링크/인코딩/독자질문검사.

테스트의기존legacy좌표시나리오는tests/legacy_position_fixture.hpp에명시어댑터를두어그시나리오를유지한다. 생산GameObject에legacy별도필드/proxy는두지않는다. 기존int A*와sendMapInfo호출은옛fixture호환wrapper로남길수있으나runtime은Position경로를쓴다.

제외: baked자료전면청크화/지역크기확대/DB region열/귀환UI/기존GUIstress의내부좌표와과거크래시확정. GUI는v2 adapter호환유지이며전체월드확장완료라고하지않는다. 기존잠금순서와동기DB경계를변경하지않는다.

## 실행 연결

| 기준 | 구현·실행 |
| --- | --- |
| AC-01 | GameObjectState.position/DBrestore·update경계,server_region_state_test와기존DB대역검사통과. 생산_x/_y필드없음. |
| AC-02 | myserver.cpp/Position map·portal·A*·monsterStand/송신local경로. 실제worker포털/시야·전투·stale AI회귀와Lua local6값검사통과. |
| AC-03 | 전체39회귀,서버/클라/GUI Debug·Release6빌드/콘솔W4·startup-check·생산클라loopback·renderer통과. 실제DB300명15초혼합EOF/저장300·오류0. 신규0/총600보존. |
| AC-04 | docs/architecture 5페이지+루트입구/AGENTS·SDD·workflow갱신. 독자7질문검토·모호성수정,소스링크/인코딩/현재와계획분리확인. |

[47단계](../refactoring-step-47.md), [실측JSON](../test-results/server-region-state-2026-11-16.json). 장시간/새계정생성/일반exe OS입력·durable readback·GUI크래시는미검증.
