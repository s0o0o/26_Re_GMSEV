# 추적 의뢰 지도 연결 #75

2026-11-21 사전명세/maina9d9904/5큐3/5.

- AC-01: M지도는 J/HUD공통Goal의 방문/조사/체험 목표를 강조. 수동/자동선택 모두같고이미방문한목표를미방문이라고하지않는다. 의뢰없으면기존일반탐험목표유지.
- AC-02: Report Goal은필드목표링없음/R귀환·정확한주민E/Q보고안내. 지도에서이동·수락/보고없고기록/위치불변.
- AC-03: 생산M반복/닫기·지역/세션기존보호유지. 순수marker projection·생산목표3상태×3크기9렌더/관련검사/클라2빌드/독립검토·구조AI·원격종료. 전체회귀최종5단계,실DB/OS미실행.

## Direction contract

THESIS: 수첩에서고른부탁이같은지도의목표가된다. 새로운길찾기/이동기능처럼보이지않게한다.

OWN-WORLD: 기존SunnyPeak지형지도·갈색목표링/파란내위치·cream/brown/neodgm보존.

STORY: J추적→M위치확인/다음행동→닫고직접이동/체험→보고준비R귀환.

FIRST VIEWPORT: 기존지도위쪽140/18한줄목표문구·우측명소상태의행동종류/방문완료구분. 기존지도/닫기위치보존.

FORM: native SFML 국소확장/code-led,새래스터/DESIGN없음.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

기존체계보존/구조AI갱신으로적용한다.

## 실행 결과

관련4명령(resident-quest-map/field-map/resident-quest-tracking/content-boundaries)·클라x64 Debug/Release2빌드 VERIFY_EXIT=0. 순수유효marker/보고제외,생산M/수동자동·이미방문조사목표/보고무링·읽기불변/끊김통과. 3상태×3크기9PNG build/tests/quest-map-{0,1,2}-{900x800,1280x720,800x900}.png각각열람. 지도+생산패널fixture이며전체HUD/actor/실제OS게임캡처아니다. 전체61등록만/5단계최종실행,실제DB/OS·장기성능미실행. 구조14문서144링크/diffcheck통과. 독립반환/원격포함·정리/종료는후속/이슈75댓글.

독립quest_map_finish_review ship/필수수정·제안없음,9개개별열람/코드·로그확인. 기존체계보존으로마무리한다.
