# 옷장·외형 동기화 #55

2026-11-16, #55. UI와 DB 저장·회귀·제품 빌드·독립 검토를 확인했다. [명세와 실행 이력](../specs/clubhouse-wardrobe.md). 최종 main 포함 증거는 이슈 종료 댓글에 연결한다. 추가 300명 혼합 부하의 최초 실패 원인은 미확정이며 #41 전체 해결로 취급하지 않는다.

## 공통 상태와 wire

appearance.hpp의 Value는 kind/fur/eyes/scarf 4바이트다. 기존2캐릭터·털색5·눈3·목도리3의90조합과 방향별 텍스처를 유지한다. wardrobe_rules.hpp의 연결별 Journal이 양수 요청ID·값·공용거점·재전송/과거ID를 검사한다.

현재 프로토콜은 #65 긴채팅의 v6다(외형 계약은 v5에서 추가). C2S_P_APPEARANCE(21)는 requestId+Value만 받으며 임의의 타인 ID를 받지 않는다. S2C_P_APPEARANCE_CHANGE(23)는 대상ID·region·Value, S2C_P_APPEARANCE_RESULT(22)는 요청ID·결과·현재 확정 외형이다. 서버/모든 클라 도구를 함께 빌드해야 한다. GUI 부하 도구는 새 패킷을 인식해 무시하며 새 외형 UI를 표시하는 도구는 아니다.

## 서버 검증·I/O·통지

processPacket → processAppearanceRequest → session.wardrobe.request 순서다. completionMutex를 유지하고 WorldStateLock에서 사전 검증한다. 유효한 저장 요청만 world 잠금을 풀어 Store.save를 호출한다. 예외를 저장 실패로 바꾸고 world를 다시 잡는다. 저장 성공과 현재 연결/거점 유효성 확인 뒤 object 외형을 바꾸고 본인→현재 live 관찰자→본인 결과를 보낸다. 관찰자는 비대칭 시야에 대비해 전체 live player registry의 viewList를 검사한다. 새 입장자는 기존 ENTER의 외형 필드를 받는다.

중복 요청은 저장/통지를 재실행하지 않는다. 실패/미설정/잘못된 값/다른 지역은 외형을 바꾸지 않는다. 저장 I/O가 world 잠금을 보유하지 않는지는 실제 worker의 테스트 Store에서 assert한다. 서버의 부분 송신/연결 수명 정책은 기존 경계를 유지한다.

## 승인된 DB 저장

사용자가 별도 외형 테이블 추가를 승인했다. appearance_sql.hpp와 관리 도구가 dbo.PLAYER_APPEARANCE(USER_ID PK, KIND/FUR/EYE/SCARF)를 정의한다. 관리 도구로 실제 DB 테이블을 생성했으며 서버 실행 시 자동 DDL은 하지 않는다. initializeDB는 열/타입/NOT NULL/단일 PK 계약을 검사하여 OdbcStore를 설치하고 종료 때 Unconfigured로 되돌린다. 기존 위치·능력치 테이블/프로시저는 변경하지 않았다.

appearance_storage.hpp의 Store.read/save를 OdbcStore가 구현한다. 공통 DB mutex와 parameterized SQL, 5초 query/lock timeout을 사용한다. save는 트랜잭션 내 UPDLOCK/HOLDLOCK 갱신·없으면 삽입하고 성공 반환을 확인한다. read는 계정·범위·NULL·길이·중복을 narrowing 전에 검사한다. 로그인은 WorldStateLock 밖에서 게임 자료와 외형을 읽는다. Found는 저장 외형 우선, Missing은 로그인 선택 유지, Error/예외/잘못된 Found는 로그인 거절로 구분한다. 미설정 대역의 Unavailable을 production DB 오류와 혼동하지 않는다.

실제 기존 전용 계정900001299의 외형 행1개를 저장·갱신하고 새 서버/클라 프로세스에서 복원을 검증했다. 마지막 값은 {kind1,fur4,eyes2,scarf2}이며 자동 삭제하지 않았다. 새 계정을 만든 검사가 아니다. 로그인 외형 읽기는 opt-in login_appearance_read trace로 계측하며 동기 ODBC를 비동기 DB로 바꾼 것은 아니다.

## 클라 상태·현재 연결 범위

wardrobe_client_state.hpp는 baseline/draft를 분리한다. 편집/취소는 확정 외형을 바꾸지 않으며 제출 뒤에는 임의 취소를 허용하지 않는다. 일치 ID 응답만 처리하고 timeout 뒤 재시도는 새 ID를 사용한다. 지역 변경은 편집을 닫고 연결 종료는 pending을 정리한다. timeout/접속 종료가 서버에서 이미 끝난 저장을 되돌린다는 뜻은 아니다.

client.cpp의 OBJECT에 확정 Value와 setAppearance를 추가했다. avatar INFO·다른 player ENTER·APPEARANCE_CHANGE에서 같은 텍스처를 선택하고 방향 source rect는 보존한다. 다른 지역/현재 없는 객체의 변경은 무시한다. 결과는 편집 상태만 확정하며 서버 변경 통지 이전에 월드 외형을 낙관적으로 바꾸지 않는다.

wardrobe_panel.hpp는 거점 옷장 버튼/W,4카테고리·아이템·4방향 미리보기·적용/취소·대기/실패 상태를 그린다. 입력은 이동/채팅/귀환보다 먼저 처리하며 pending 중 중복 적용/닫기를 막는다. 지역 변경 시 닫고 연결 종료 시 pending을 정리한다. SFML texture를 가진 패널은 전역 초기화 대신 함수 정적 수명으로 관리한다. 월드/HUD를 그린 뒤 overlay/swap 전에 한 번 배경을 캡처하여 viewport를 잘라 블러한다. 기존 90조합을 사용하며 보관된 액세서리 전부를 착용 구현한 것은 아니다.

## 실행·한계

run-wardrobe.cmd는 규칙/편집 상태 단위 검사와 실제 TCP/IOCP4worker+게임 DB 대역/외형 메모리 Store를 실행한다. 미설정/오류/예외·거점 제한·중복·live 관찰자·늦은 ENTER·world 밖 I/O를 검사했다. 메모리 Store는 실제 DB/파일 저장 증거가 아니다.

run-wardrobe-client.cmd는 생산 client.cpp를 포함하여 본인/주변/늦은 ENTER·방향 보존·다른 지역 무시·draft 비낙관 적용/종료를 검사하고 wardrobe-wire-preview.png를 만든다. 2캐릭터×4방향 텍스처의 생산 렌더러 테스트 출력이지 실제 옷장 화면/OS/DB 캡처가 아니다. 전체 회귀/제품 빌드/실제 옷장 UI 검증 상태는 명세에서 별도로 기록한다.
