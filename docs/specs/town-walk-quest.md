# Boss 산책 의뢰 #63

2026-11-17 · 구현/실행검증 · 주민큐3/3. 접속중로컬체험이며서버퀘스트/보상/DB저장/새패킷없음. 같은접속의이미방문한지점도인정한다.

- AC-01: 미수락→Boss대화Q/버튼수락→1 Flower Walk/2 Riverside Rest 둘러보기→보고가능→Boss근접대화Q/보고완료. 수락·보고는정식Boss대화유효성/활성접속을필요로한다. 타주민/거리밖/중복/미완료보고거부.
- AC-02: 기존서버확정위치Record로진행하며예측입력/타인위치로완료하지않는다. 수락전같은접속방문인정. 거점왕복유지·끊김/새로그인초기화. 누른채Q반복으로수락과보고가자동연속되지않는다.
- AC-03: Boss에게의뢰대상/보상없음/접속중한계·상태별버튼·진행·보고안내표시. 필드HUD는진행/보고귀환을우선하고보고뒤일반탐험안내로복귀. 기존지도/대화/portrait·idle·산책유지.
- AC-04: 순수상태·생산패킷/입력·다중크기6상태렌더/기존회귀·전체51회귀/추가창검사·native Debug/Release·독립검토·구조/AI기록·main포함/안전정리/이슈종료및3큐감사.

## Direction contract

THESIS: 주민에게나가겠다고말하고돌아와이야기를마치는작은산책완결.

OWN-WORLD: 기존cream/brown/neodgm·Boss원본portrait,새이미지/경제장식없음.

STORY: E대화→Q수락→지도1/2방문→R귀환→E/Q보고. 이미방문한기록인정·접속종료시초기화.

FIRST VIEWPORT: 기존대화본문의의뢰설명과Boss전용Q버튼을다음/안내위에추가. 하단HUD는산책진행/보고안내한줄만표시하고다른HUD와겹치지않게우선순위를준다.

FORM: 기존대화/탐험내국소확장/code-led,no newDESIGN/conceptroll. 서버보상이나영구퀘스트라고꾸미지않음.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

## 실행 증거와 한계

- AC-01/02: `tests/run-town-walk-quest.cmd` 순수 상태 검사와 생산 client.cpp fixture에서 수락/미완료 보고/방문1/2/보고/중복/타주민/Q 반복/종료·새접속 초기화를 실제 실행했다. seedNum 불변도 검사했다.
- 최초 fixture는 필드 위치에서 거점 Boss ENTER를 먼저 넣어 정상 지역 검증에 거부됐다. 거점 위치→ENTER 순서와 대상 존재 assertion으로 수정했다. 최초 wrapper의 잘못된 성공 반환은 통과로 취급하지 않는다. signed-negative 종료 거부를 보강하고 별도 `cmd exit /b -1` probe로 확인했다. 최초 자식 종료코드는 미수집이다.
- AC-03: offer/active/progress/return/ready/reported × 900×800/1280×720/800×900, 총18 PNG를 실제 생성·직접 열람했다. portrait 아래 접속 중/재접속 초기화 표기를 보강했다. `build/tests/quest-*.png`는 생산 렌더러 fixture 출력이며 일부 world actor를 생략한다. 일반 exe 수동 OS 조작/실제 DB 캡처가 아니다.
- AC-04: 최종 `build/tests/verify-resident-queue.cmd` exit0. 전체51 회귀, 추가11 명령(의뢰/지도/주민/crop/캐릭터/필드/옷장wire·UI/귀환/화면/로그인), 클라 x64 Debug/Release 빌드2개 통과. 로그는 `resident-final-*.log`, `resident-native-{Debug,Release}.log`. 기존 변환 warning은 남아 있다.
- 새 DB/계정/패킷/보상·실제 DB 부하·일반 exe 수동 OS 입력은 이번 큐에서 실행하지 않았다. 독립 검토/원격 main 포함 증거는 최종 결과와 이슈 #63에 연결한다.

독립 `walk_quest_finish_review`: 새 문맥/읽기 전용으로 명세·코드·18PNG/참조2장·로그를 확인하고 **ship**, 필수 수정 없음. 접속 한계/본문/버튼/HUD가 세 크기에서 겹치지 않고 기존 시각 체계를 유지한다고 판정했다. 검토자가 테스트를 재실행한 것은 아니다. 구조 문서12개/로컬 링크110개와 NPC 원본21개 SHA256 검사 통과. 최종 main 포함/이슈 종료 증거는 #63 종료 댓글에서 확인한다.
