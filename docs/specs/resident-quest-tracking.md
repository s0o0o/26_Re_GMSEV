# 주민 의뢰 추적 #74

2026-11-21 사전명세/기준main7cc74ec. 5큐2/5.

- AC-01: 목록 Enter/추적버튼은 Active/Ready만 수동고정, 잠금/미수락/완료 거부. A/자동버튼으로 고정해제. 자동순서 Boss체험→Pashmina체험→Boss방문, 완료시고정해제·대체추천.
- AC-02: 방문/조사/체험/보고 Goal을 공통으로 J목표행·다음행동/HUD에 전달하고 수동/자동 표시. 같은의뢰수락/보고/진행bit는불변.
- AC-03: 키반복/press-release외부취소·모달/거점왕복유지·끊김/로그인초기화. 상태가버튼누름중변하면 owner재검증.
- AC-04: 순수거부/우선순위·생산Enter/A/클릭·읽기/진행불변/수명·수동추적3크기+HUD3크기6렌더,관련검사·클라2빌드·독립검토/구조AI·원격반영. 최종전체회귀는5단계,실DB/OS미실행.

## Direction contract

THESIS: 지금 하고 싶은 부탁 하나를 내가 고른다. 수락/보상 버튼과 추적을 혼동하지 않게 한다.

OWN-WORLD: 기존 수첩3행·cream/brown/neodgm,선택설명/하단버튼 슬롯 보존.

STORY: J/Tab목록→진행행선택→Enter추적→닫고행동→A자동복귀.

FIRST VIEWPORT: 행아래 현재안내표시,556 추적/자동버튼,589접속한계. 기존닫기/Tab위치보존.

FORM: native SFML 국소확장/code-led,새래스터/DESIGN없음.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

기존체계보존/구조AI갱신으로 적용한다.

## 실행/수정 결과

관련4명령(resident-quest-tracking/resident-quest-log/field-journal/content-boundaries)·클라x64 Debug/Release2빌드 실제exit0. 순수거부/만료·기본순서/방문Goal,생산Enter/A/외부release/키반복·J목표/읽기불변/끊김·로그인통과. 왕복보존은reset위치코드확인/최종통합추가예정이다. 전체60등록만/5단계최종실행, 실제DB/수동OS·FPS미실행.

6PNG build/tests/quest-tracking-{list,hud}-{900x800,1280x720,800x900}.png각각열람. 생산map+panel fixture이며actor/전체HUD/OS캡처아니다. 독립검토가 버튼held중선택변경이면다른의뢰가고정되는문제를찾아채택했다. Up/Down시누름arming취소/동일생산회귀추가후같은6장재생성/관련4·클라2재실행exit0. 최종verdict와원격종료근거는후속/이슈74기록.

동일reviewer최종verdict **ship / owner지적resolved**,6장재열람/코드·회귀·로그확인,추가차단없음. 구조14문서143링크/diffcheck통과. 이후지도연결로자동진행한다.
