# 주민 탐험 안내 #62

2026-11-17 · 구현/관련검증완료(사전명세갱신) · 주민큐2/3. 동일5ID/현재접속방문기록사용. 보상/DB/새패킷없음. main포함/안전정리증거는 #62 종료댓글연결.

- AC-01: 유효한열린주민대화에서G/안내버튼으로다음미방문ID선택. 방문기록이활성화된접속에만안내,반복선택은같은미방문목표. 방문하면다음미방문으로갱신/전부방문시완료. inactive는거부.
- AC-02: 거점에서는필드로R안내,필드에서는목표이름·상하좌우방향/Manhattan좌표거리표시. 실제최단경로나장애물우회라고부르지않음. 지도에서목표외곽선·범례표시와현재위치/방문번호가읽혀야함.
- AC-03: 목표는거점왕복유지/끊김·새로그인초기화. 대화범위이탈·대상없음·모달차단중안내입력은발동하지않음. 세션탐험기록경계와중복성검사.
- AC-04: 순수규칙·생산G/클릭·패킷/초기화·목표/완료렌더·기존대화/지도/옷장/귀환검사·클라2빌드·독립검토·구조/AI기록·main원격/안전정리·이슈종료.

## Direction contract

THESIS: 주민의말한마디가실제지도의다음목표로이어진다.

OWN-WORLD: 기존cream/brown/neodgm,목표는갈색외곽선과글자‘목표’,현재위치는파란테두리유지. 새raster없음.

STORY: E대화→G탐험안내→미방문지이름확인→R로필드→목표표시/방향·좌표거리→방문후다음지점.

FIRST VIEWPORT: 기존대화하단왼쪽안내버튼. 기존지도표식바깥목표테두리/범례상태,닫힌지도아래작은목표안내줄. 완료/타지역문구명시.

FORM: 기존대화/지도내국소확장/code-led,no newDESIGN/conceptroll. 경로찾기/서버퀘스트로꾸미지않음.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

## 실행 이력

2026-11-17:관련5명령(run-exploration-guidance/field-map/wardrobe-ui/travel-client/display)및native Debug/Release2빌드통과(`guidance-*.log`). 첫명령은순수안내규칙+기존대화규칙/생산UI를실행한다. inactive/중복/다음미방문/거리·방향/타지역/완료/reset과생산G·안내클릭·대화차단/본인패킷방문→다음목표/거점왕복/끊김·새로그인을검사했다. port0의도접속실패는reset회귀이며실제DB오류가아니다.

5상태x3크기15PNG `guide-{dialogue,hub,field,map,complete}-{900x800,1280x720,800x900}.png`는fixture/생산renderer 출력이다. 전체client_main/실접속OS게임캡처라고부르지않는다. 주에이전트가전부열어확인했고 `guidance_finish_review` 독립ship/필수수정없음. 실제경로찾기가아니며fixture배우생략한계를기록했다. 전체48회귀/실제DB·부하미실행. AC-01~04 정의된범위통과,main/정리는완료후이슈연결.
