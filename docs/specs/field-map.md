# 필드 지도 #59

2026-11-17 · 구현·관련 검증 완료 · 큐2/3. #58의 동일5지점 ID/좌표와 실제 terrain을 표시하는 기존 HUD의 국소 확장. 모드Operate/code-led. 원격main/정리증거는 #59 종료 댓글에 연결한다.

- AC-01: M/버튼으로 필드에서만 열고 M/Esc/닫기로 닫는다. 채팅/옷장/귀환pending/끊김 중 열지 않으며 열린 동안 게임입력 소비, F11 기존 전역처리 유지. region전환/접속종료/새로그인에서 닫는다. M자동반복 재토글 방지.
- AC-02:256×256 실제terrain의 nearest 지도·내위치·거점출구·5지점ID+이름 표시. 임의 위치 이동/서버 요청/새 DB 없음. 유효좌표만 마커변환.
- AC-03: 논리900×800/letterbox 입력과900×800/1280×720/800×900 렌더·클릭 검증. 상태·지도 안 위치/범례 경계검사.
- AC-04: 생산client.cpp 주입입력/기존옷장·귀환·필드·display회귀/클라2빌드·독립검토·구조/AI기록·main원격/안전정리/이슈종료.

## Direction contract

THESIS: 넓은 필드에서 내가 어디이며 다음 표식이 어디인지 한눈에 확인한다.

OWN-WORLD: 기존 cream/brown/gold/neodgm 패널, 실제grass/river/cliff색 데이터 지도, 새raster/정체성 없음.

STORY: 필드[M]지도→내위치·번호와이름/거점출구 확인→닫고 이동. 지도 클릭은 이동 기능이 아니다.

FIRST VIEWPORT: 기존 HUD 우상단 귀환 아래 지도버튼. 중앙(70,90,760,600), 왼쪽432정사각 지도, 오른쪽5개 범례, 아래위치/닫기·닫은뒤R안내.

FORM: 좁게 정의한 기존HUD extension/code-led, no concept roll/newDESIGN. 지도와탐험목록은같은패널안에서이어진다.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

## 실행·진행 이력

최종 독립 `field_map_finish_review`: 필수4이미지/기존UI/소스/실행로그 직접확인, 지도범위 ship/필수수정없음. 3크기/현재위치와2번겹침해소/범례·버튼·입력검증증거 확인. 문서의 구현전 상태 갱신 제안 채택. AC-01~04의 정의된 범위 통과, 실제OS/DB/부하로 확대하지 않는다.

2026-11-17: 최초 CP949 삽입 패치의 배열 문자열 결합 오류로 include/함수헤더/옷장입력 줄이 누락되어 컴파일 실패했다. diff 확인 후3개 줄을 복구하고 제품·테스트를 재컴파일했다. 원본 한글바이트 roundtrip 보존. 실패원인은지도데이터/서버오류가 아니다.

`run-field-map.cmd`: 생산client.cpp의 M반복/채팅/옷장/귀환pending·모달 R/이동입력 소비, 지역전환/끊김/유효좌표/press-release 클릭/3크기 매핑 통과. 최초 렌더의 위치 점이 지점2숫자를 덮어 투명 파란테두리와그에맞는범례로 변경했다. 변경 뒤 map/wardrobe-ui/travel-client/field-preview/display5검사·native Debug/Release2빌드 전체 재실행exit0. 로그 build/tests/map-*.log. 실제DB/부하/수동OS/전체46회귀는 이 단계에서 미실행이다.

실제 생성한4PNG(field-map-900x800/1280x720/800x900/entry)는 생산renderer 출력이고 SFML입력은프로그램주입이다. 주 에이전트가 전부직접확인했고 `field_map_finish_review`에 독립검토를 맡겼다. 최종판정/원격포함은실제완료후이슈연결.
