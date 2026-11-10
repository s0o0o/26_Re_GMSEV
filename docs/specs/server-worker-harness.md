# 명세: DB 없는 실제 서버 worker 통합 검증

- 상태: 검증 완료 (아래 제한된 범위, 사전 명세)
- 이슈: [#30](https://github.com/s0o0o/26_Re_GMSEV/issues/30)
- 기준: `145417c`, 2026-11-10
- 대상 독자: 프로젝트 작성자와 이후 수정 작업자. [이전 수명 명세](async-session-lifetime.md)의 전체 worker 검증 공백을 좁힌다.

## 목적·범위와 제외

테스트가 직접 fragment를 복사하거나 완료 판정만 호출하지 않고, 실제 AcceptEx → WSARecv → worker 파서 → processPacket → LOGIN_OK → 송신 → disconnect를 통과시킨다.

포함: 테스트 실행 파일에 생산 `myserver.cpp`를 main 이름만 바꿔 포함, 실제 loopback 임시 포트와 IOCP, DB 함수만 메모리 대역으로 링크, 분할/연속 프레임, 미로그인 입력, 잘못된 크기, 로그인 실패/재시도, 두 관찰자의 입장/이동/퇴장, 30회 순차 재접속과 실제 섹터·시야 정리. 발견 결함은 이 범위에서만 수정한다.

제외: 실제 ODBC/계정 데이터 변경, 게임 UI, Lua/NPC/타이머 실행, 다수 worker 간 경합, 동시 접속 상한/장시간 부하, 전역 registry 동시성 재설계. 최대 동시 접속은 2개이며 30회 반복은 동접 30명이 아니다.

## 현재 동작·목표와 영향

현재 회귀는 실제 TCP/IOCP와 공통 판정 함수를 검사하지만 생산 worker 파서를 호출하지 않는다. 목표는 그 생산 함수를 복제하지 않고 테스트하는 것이다. 테스트 전용 관찰 지점은 컴파일 매크로 아래에서만 사용한다. 관찰자는 같은 worker에서 상태를 복사해 mutex/condition_variable로 전달하고 테스트 스레드는 실행 중 registry를 직접 읽지 않는다.

worker의 정상 종료를 위해 성공한 `PostQueuedCompletionStatus(port,0,0,nullptr)`를 내부 종료 신호로 인식한다. 게임 패킷이 아니며 정상 서버는 이 신호를 게시하지 않는다. 기존 null/error 완료와 구분한다. 전체 서버 종료 설계를 구현하는 작업은 아니다.

- 서버: worker 종료 신호·테스트 관찰 지점과 범위 내 발견 결함만 변경.
- 클라이언트/프로토콜/리소스: 변경 없음.
- DB: `game_database.cpp`와 ODBC 라이브러리를 테스트에 링크하지 않는다. lookup/save는 대역의 호출 횟수만 기록한다. 정상 서버 빌드의 DB 구현은 유지한다.

## 오류·경계

로그인 헤더 한 바이트만 수신, 본문 미완성 상태에서 종료, 프레임 여러 개의 연속 수신, 미로그인 이동, 로그인 후 재로그인, 잘못된 size/알 수 없는 type, lookup 오류 후 같은 소켓으로 재시도, 종료/ID 재사용 뒤 이전 완료, 중복 입장·퇴장과 섹터 흔적을 확인한다. 대기는 조건/소켓 timeout으로 제한하며 임의 sleep으로 동작 순서를 가정하지 않는다.

이전 완료의 순서를 결정적으로 재현하기 위해 **새 접속이 ST_ALLOC일 때 이전 접속 owner의 LOGIN_OK와 0바이트 SEND를 PostQueuedCompletionStatus로 주입**한다. 이는 실제 오류 GQCS를 강제한 것이 아니라 인위적인 오래된 완료 게시다. 실제 소켓 close의 완료는 별도로 생산 worker가 처리한다. 이전 명세의 실제 IOCP 취소 테스트도 회귀에 유지한다.

소유권 해제는 (1) 다음 ID 재사용 뒤 주입한 작업과 완료 처리 참조가 없어져 이전 weak_ptr가 만료되는지, (2) 마지막 접속은 free registry 슬롯만 남아 use_count가 1이 된 뒤 worker를 종료하고 registry를 비우면 모든 관찰 weak_ptr가 만료되는지로 확인한다. registry가 정상 보유 중인 참조를 누수로 판정하지 않는다.

## 완료 기준과 검증 방법

| ID | 관찰 가능한 결과 | 검증 |
| --- | --- | --- |
| AC-01 | 실제 AcceptEx가 연결을 ST_ALLOC으로 등록하고 정상 로그인에 avatar/login-ok/map 응답 | 두 TCP peer + 생산 worker, DB 대역 호출 확인 |
| AC-02 | 1바이트 헤더/미완성 본문이 실제 worker에 보관되고 나머지 수신 후 로그인; 합쳐 보낸 두 이동 프레임 모두 정확히 반영 | 관찰 snapshot + 응답 좌표·move_time |
| AC-03 | 로그인 전 이동은 게임 응답/섹터 진입 없이 무시; 로그인 후 재로그인은 DB 재조회 없이 무시; 잘못된 크기/타입과 미완성 로그인 종료는 저장 없이 정리; lookup 실패 뒤 재시도 가능 | 첫 응답 타입·대역 조회/저장 횟수·종료 snapshot |
| AC-04 | 주변 상대가 ENTER/LEAVE를 수신하고 종료자 자신의 시야·실제 모든 섹터·다른 시야의 ID가 사라짐 | 실제 패킷 및 worker에서 잠금 후 snapshot |
| AC-05 | 같은 ID 재사용 시 접속 번호 변경, 30회 순차 반복 후 spectator가 계속 동작; pending 작업은 소유권을 해제 | snapshot/weak_ptr, 관찰자의 이동 응답 |
| AC-06 | worker를 join하고 테스트 소켓을 닫아 정상 종료; 기존 회귀 및 서버 Debug/Release 통과 | 테스트 전용 runner·전체 회귀·MSBuild |

## 구현 계획

| 작업 | AC | 위치 | 상태 |
| --- | --- | --- | --- |
| T-01 | AC-01~05 | `tests/server_worker_test.cpp`: 생산 소스 포함/DB 대역/관찰·네트워크 시나리오 | 실행 통과 |
| T-02 | AC-02~04, AC-06 | `myserver.cpp`: 테스트 관찰/내부 종료 신호/미로그인 게임 요청 및 로그인 후 재로그인 거부 | 구현·실행 통과 |
| T-03 | AC-06 | 테스트 runner 연결, 빌드·반복 실행·문서 검증 | 실행 통과 |

## 검증 결과·인계

### 사전 기록

명세 커밋 `f973898`에서는 아직 실행하지 않았다. 이후 구현으로 아래 결과를 얻었다. 구현/병합 SHA는 #30에 기록한다.

### 2026-11-10 결과

환경: Windows MSVC x64, assertions 활성. 기준 `f973898` 이후 이번 브랜치 구현.

- AC-01~05 실행 통과: [server_worker_test.cpp](../../tests/server_worker_test.cpp), [단독 runner](../../tests/run-server-worker.cmd). `tests\run-server-worker.cmd`에서 생산 cpp/main 이름 변경 포함, 실제 TCP/AcceptEx/IOCP worker 하나를 실행한다. 1/5바이트 미완성 로그인 fragment를 실제 worker가 보관하는지 snapshot으로 기다린 후 나머지를 보낸다. 두 이동은 한 번에 써서 두 응답의 좌표·move_time을 검사한다(TCP가 한 번의 WSARecv로 전달한다는 보장은 하지 않는다).
- AC-03 실패 재현 → 수정: 미로그인 MOVE + LOGIN을 함께 보냈을 때 첫 응답이 AVATAR_INFO가 아니라 MOVE여서 테스트가 실패했다. `processPacket()`에 ST_ALLOC/ST_INGAME 요청 경계를 추가한 후 동일 테스트 통과. 로그인 후 새 계정으로 재로그인 요청을 보내도 조회/저장 대역 호출이 0임을 확인했다.
- AC-04 실행 통과: 실제 모든 섹터와 시야 목록을 worker에서 검사했다. 관찰자 TCP 스트림에서 입장/퇴장 건수도 세어 중복 없이 각 접속당 1회인지 확인했다. 실제 DB 저장이 아니라 대역 호출 횟수를 검사한다(정상 로그인 시 기존 저장 1회 + 종료 시 저장 1회).
- AC-05 실행 통과: 30회 순차 재접속, 같은 ID=1과 다른 connectionId, 로그인 전 이전 완료 주입 후 정상 로그인, 관찰자 이동 지속, pending 참조 소멸을 확인했다. 최종 하네스 단독 실행 10회 추가 반복 → 추가 300회 순차 재접속 통과. 최대 동시 peer 2개, worker 1개이다.
- AC-06: `tests\run-regressions.cmd` 전체 17개 통과. 서버 MSBuild x64 Debug/Release, Debug `--startup-check` 통과(DB·네트워크 없음). 기존 C4244/LNK4098 경고 유지. 테스트 매크로 없는 정상 서버도 빌드했다. 테스트 바이너리 `dumpbin /imports`에서 ODBC/SQL import가 없음을 확인했다.
- 테스트 하네스 수정 이력: 서버의 대체 토큰 `not`을 컴파일하려고 `/permissive-`를 맞췄다. 마지막 free registry 슬롯의 정상 참조를 weak_ptr 누수로 오판한 검사를 고쳤다. worker drain → join → registry clear → weak 만료 순서로 검사한다. 읽지 않은 응답을 남기고 close하는 일부 오류 시나리오에서 `[NET]` 종료 오류가 나오고, pending AcceptEx 취소 시 Accept completion failed가 나올 수 있다. 이것만으로 테스트 실패는 아니며 종료·저장·참조 단언 결과로 판단한다.
- 문서 독립 독자 검토에서 완료 주입 방식과 해제 판정 시점이 모호하다는 지적을 받아 위 경계 설명을 보완했다. 문서 링크·UTF-8·diff 검증은 [28단계](../refactoring-step-28.md)에 연결한다.

이 검증은 실제 DB, Lua/NPC/타이머, UI, 다중 worker 경합과 장시간 부하를 보장하지 않는다. #10/#19의 실제 원인은 아직 이 결과로 확정하지 않는다. 다음 작업은 다중 worker·동시 접속/종료 시 registry와 세션 상태 접근의 동기화 경계를 별도 명세로 다룬다.
