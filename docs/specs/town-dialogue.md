# 근접 주민 대화 #61

2026-11-17 · 구현/관련검증완료(사전명세에서갱신) · 주민큐1/3. 기존NPC3종을사용하고위치/스프라이트/서버행동·DB·프로토콜은변경하지않는다. 원격포함·안전정리증거는 #61 종료댓글연결.

- AC-01: 관측중인정식주민ID/type·동일Clubhouse·양쪽walkable·Manhattan거리2이내/중간통로존재로대상선택. 동률은ID낮은주민. 원본home좌표가아니라현재서버관측좌표사용.
- AC-02: E/표시버튼→주민성격별2페이지→E/Enter/다음→마지막닫기,Esc/닫기. 자동반복으로페이지건너뛰지않음. 대화중게임입력소비,채팅/옷장/지도/귀환pending/끊김은열기차단. 전역F11유지.
- AC-03: 대화대상고정,live관측LEAVE/범위이탈/지역전환/끊김/새로그인때닫기. NPC는멈추거나정지이미지로바꾸지않고기존idle/산책지속한다. 이는서버대화/AI생성이아닌로컬안내대사다.
- AC-04: 순수근접/선택/페이지·생산입력/관측패킷/정리회귀·3주민/크기별렌더·관련검사/클라Debug·Release·독립검토·구조/AI기록·main원격/안전정리/이슈종료.

## Direction contract

THESIS: 허전한거점의주민이각자한마디로산책과커마를안내한다.

OWN-WORLD: 기존cream/brown/gold·neodgm·원본주민portrait와idle,새raster/정체성없음.

STORY: 가까운주민의E안내→성격별2페이지읽기→닫고다시이동. 주민이멀어지면닫힘.

FIRST VIEWPORT: 기존월드/행동위에근접안내,대화는하단(100,440,700,220). 왼쪽원본portrait,오른쪽이름·짧은2줄대사,페이지·다음/닫기. 보이는동안입력을보호한다.

FORM: 기존거점HUD의좁은국소확장/code-led,no conceptroll/newDESIGN. 한국어대사는가상의게임내안내이며실제권리자공식대사가아니다.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

## 실행 이력

독립 `town_dialogue_finish_review`:18PNG와기존옷장참조·입력/위치/상태코드확인후ship. 필수수정없음/마지막페이지안내·수면portrait·의도된world가림확인. 실제시간의idle/부하/DB를정지PNG검토로확정하지않는다. 마지막 mouse-next assertion 추가후관련7명령/클라2빌드를같이재실행했다.

2026-11-17: `verify-dialogue.cmd`의town-dialogue/town/town-preview/field-map/wardrobe-ui/travel-client/display7명령및native Debug/Release2빌드통과. 순수규칙과생산E반복/페이지·클릭/채팅·모달/주민MOVE·LEAVE/지역/끊김정리검사를실행했다. 마지막페이지안내/렌더증거compositor수정뒤같은묶음을재실행exit0. 원본NPC/서버행동은변경하지않았다. 전체48회귀/실제DB·부하/수동OS는이단계에서미실행이다.

`dialogue-{100,101,102}-{0,1}-{900x800,1280x720,800x900}.png`18장은fixture대상·생산NPC/대화렌더를호출한출력이며실접속OS게임캡처가아니다. 주에이전트가전부열어확인했고독립검토에전달했다. portrait의다른idlephase도표시하며거점월드일부는대화패널뒤로가려지는의도된overlay다. NPC원본이잘린것과구분한다.
