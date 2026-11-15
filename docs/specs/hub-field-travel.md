# 거점↔필드 이동·귀환 (자율 큐1)

2026-11-16 기준 fdf1d66 / #53. 상태: 구현·실행 검증 완료. main 포함 증거는 #53 최종 댓글의 커밋·원격 확인으로 기록한다. 큐2 청크·충돌 기반, 큐3 옷장·커마는 후속이며 목표를 축소하지 않는다.

목적: 필드에서버튼/R로거점귀환,거점에서필드나가기를지원한다. 좌표는클라가정하지않으며서버가고정도착점을선택한다. 개인집/새DBschema/계정추가는하지않는다.

- AC-01: v3로서버·모든클라버전정렬. 명시action(EnterField/ReturnHome)+양수requestId,결과response. 잘못된action/0/과거id거부,동일id/action은결과재전송만하며위치·섹터·이벤트를중복변경하지않음. 목적지invalid/막힘/이미목적지역은변경없음.
- AC-02: 공통서버전환함수가기존관찰자leave/추격generation해제/섹터단일소속/새시야enter·본인MOVE/map/near목록초기화를보장한다. source/destination검증과world잠금유지. 기존포털도동일전환정리를사용한다. DB저장은기존codec/정상종료경계유지.
- AC-03: native클라버튼/R·chat충돌방지·pending중복차단·서버판정전낙관위치변경없음·성공/거절/timeout/접속종료표시. 서버응답id를매칭하고다른/지연된id는pending을완료시키지않는다. native게임송신에순서보존/부분송신outbox를사용하여이동요청이잘리지않게한다.
- AC-04: 실제worker TCP+DB대역왕복/이전observerleave·새observerenter/추격무효화·중복/실패/종료저장회귀,생산클라loopback/부분송신모의·키/클릭/다중화면미리보기·전체회귀/서버클라GUI Debug·Release빌드. 실제DB는실행한경우만따로기록,미리보기는유형구분.
- AC-05: architecture/작업큐/실측결과/이슈를갱신하고main반영확인. 큐1완료뒤승인메시지없이큐2착수. 새권한/중요제품선택은멋대로확장하지않는다.

UI: 기존cream/gold/brown pixel세계유지,게임HUD우상단버튼과R단축키. field에서는귀환,거점에서는나가기로명확히구분한다. 공용Clubhouse만있으므로개인집이라고표시하지않는다. pending/실패는색만이아닌문구로표시한다.

## 진행·검증 기록

2026-11-16, feature/거점필드-귀환 검증 기록:

| 기준 | 구현/검증 | 상태 |
| --- | --- | --- |
| AC-01 | shared/world_travel.hpp, protocol.hpp, world_travel_test.cpp | 요청 ID·재전송·거절·막힘·v3 길이 단위 검사 실행 통과 |
| AC-02 | region_transition.hpp, processPacket의 MOVE 포털/TELEPORT | 기존 run-regions.cmd 및 새 run-travel.cmd 실행 통과 |
| AC-03 | ClientState/PacketOutbox, client.cpp, travel_panel.hpp | 단위·native TCP 요청/분할 응답, R키/클릭 이벤트·채팅·누른 키 반복·ID·거절·timeout·종료 검사 통과 |
| AC-04 | run-travel.cmd, run-travel-client.cmd, run-regressions.cmd | 실제 worker/DB 대역 및 전체 41회귀 통과. 서버/native/GUI Debug·Release 6빌드 통과. 상태/3해상도 렌더러 미리보기 검토 ship |
| AC-05 | 작업 큐, architecture/travel.md, refactoring-step-49.md | 문서 검사 통과. 커밋/원격 포함 확인·이슈 종료는 #53 최종 댓글에 기록 |

새 테스트의 최초 실행은 저장 횟수 기대값 1에서 실패했다. 기존 로그인 저장 1회와 종료 저장 1회를 합쳐 2회인 계약에 맞게 테스트를 수정했고 재실행 통과했다. 생산 DB 저장 경로를 변경하지 않았다. 테스트 종료 중 error=64/Accept completion failed 로그는 통과 여부와 별도로 남긴다. 실제 DB 부하 검증으로 확대하지 않는다.

2026-11-16 후속 실행: 전체 회귀 최초 실행은 주민 포털 검사에서 명시적 LEAVE 누락을 검출했다. transitionRegion이 내부 시야만 지우던 처리를 보완하여 본인에게도 이전 대상 LEAVE를 보내고, 동일한 전체 회귀를 재실행하여 41개 통과했다. 테스트를 완화하지 않았다. 서버 2빌드는 이 수정 후 다시 통과했다. 기존 빌드의 수치 변환/기본 라이브러리 경고는 오류와 구분한다.

생산 렌더러 테스트 출력: build/tests/travel-states.png, travel-extra-states.png, travel-1280x720.png, travel-1920x1080.png, travel-800x900.png. UI 검토자는 이 다섯 출력과 기존 미리보기를 직접 보고 **ship** 판정했다. 기존 팔레트/글꼴·주요 지역 라벨의 겹침/잘림 없음과 상태별 가독성을 확인했다. 스킬에 따라 국소 확장만 검토했으며 새 DESIGN.md/시각 세계나 raster 자산을 만들지 않았다. native C++이므로 HTML/CSS detector는 실행하지 않았다.

증거의 한계: 렌더러 테스트 장면은 실제 OS 스크린샷이나 실제 DB 접속 화면이 아니다. 하단 프로필을 포함한 전체 게임 HUD 합성/실제 F11 조작은 이번 새 미리보기 범위가 아니며 기존 화면 적응 테스트와 코드 경계를 유지한다. 글자 bounds 자동 검증이라고 주장하지 않고 출력의 육안 검토와 구분한다. pending 중 Tab 채팅 전환도 막힌다. 이번 단계는 실제 DB 부하를 다시 실행하지 않았다.

다음: main 원격 포함 확인·#53 종료 후, 승인 메시지 없이 큐2 청크·충돌 작업의 별도 이슈/명세에 착수한다.

추가 실행: run-login-screens-preview.cmd의 기존 로그인/재시도/분할 회귀, run-display.cmd의 실제 SFML 창 크기·클릭 변환·F11/Escape·창 복원 검사도 통과했다. 이 자동 테스트는 사용자의 수동 OS 입력을 촬영한 증거와 구분한다.
