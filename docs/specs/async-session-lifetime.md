# 명세: 비동기 완료의 접속 소유권과 재접속 격리

- 상태: 검증 완료 (아래 제한된 범위, 사전 명세)
- 이슈: [#29](https://github.com/s0o0o/26_Re_GMSEV/issues/29)
- 기준: `81b212f`, 2026-10-28
- 관련: [세션·월드 명세](session-world-lifecycle.md), [SDD 절차](../sdd.md)

## 목적·범위

접속 종료 후 늦게 도착한 RECV/SEND/로그인 완료가 해제된 객체를 참조하거나 같은 숫자 ID의 새 접속을 종료·변경하지 않도록 한다.

포함: 작업별 원래 접속 소유권, 수신 버퍼 수명, 완료 경로의 접속 고정, 송수신 등록과 close의 직렬화, 즉시 실패/취소/지연 완료의 해제, DB 없는 실제 IOCP 테스트.

제외: DB 부하·그래픽 E2E, 전체 registry/게임 데이터 동시성 재설계, graceful 서버 종료와 전체 IOCP drain, 부분 송신 재시도, 새 게임 기능, 프로토콜/클라이언트 변경. 기존 로그인/부하 원인을 이번 결함 하나로 확정하지 않는다.

## 현재 동작과 목표

현재 `GAMESESSION::_recvOver`가 실제 pending WSARecv 저장소이고, worker가 완료 key의 ID로 registry를 다시 조회한다. 객체 교체 뒤 원래 버퍼가 해제되거나 오래된 오류가 새 접속을 disconnect할 위험이 있다. SEND/LOGIN_OK 작업에는 원래 접속 소유권이 없다.

목표: heap 작업 객체가 원래 `shared_ptr<GAMESESSION>`와 버퍼를 보유 → IOCP 완료가 원래 객체를 선택 → 종료 플래그/ID 일치 검사 → 원래 객체만 처리 → 작업 1회 해제. 새 접속은 별개 객체이므로 영향을 받지 않는다. 세션 내부 `_recvOver`는 미완성 패킷 보관용으로만 사용한다. 작업 완료까지 OS에 준 메모리는 유지한다.

등록/close는 transport 잠금으로, 같은 세션의 수신·로그인 완료와 종료 처리는 completion 잠금으로 보호한다. 게임/AI가 보내는 송신은 completion 잠금을 요구하지 않아 NPC 잠금과의 역순 획득을 피한다. 종료한 접속의 완료는 버리되 작업 소유권은 반드시 해제한다.

## 영향

- 서버: `iocp_operation.hpp`, `game_session.*`, 완료 판정 헤더, `myserver.cpp` worker/로그인/종료 경로.
- 클라이언트/공유 프로토콜: 변경 없음. connectionId/owner는 서버 내부 정보.
- DB: 기존 저장 경로 유지. 이번 검증은 실제 DB를 사용하지 않는다.

## 오류·경계

즉시 제출 실패에는 완료가 오지 않으므로 즉시 작업을 해제한다. pending 취소/close 후에는 성공·0바이트·오류 어느 완료든 원래 작업을 해제한다. 동일 ID 재접속, LOGIN_OK 지연, 수신 분할/연속 프레임, 중복 RECV 등록, 소유권 없는 stack 세션의 비동기 제출을 검사한다. 전역 ACCEPT는 세션 작업과 구분한다.

## 완료 기준과 검증

| ID | 관찰 가능한 결과 | 검증 방법 |
| --- | --- | --- |
| AC-01 | pending RECV/SEND 작업이 원래 객체를 유지하고 완료 해제 뒤 남은 소유자가 없으면 소멸 | 실제 loopback IOCP와 weak_ptr 검사 |
| AC-02 | 같은 ID 새 접속으로 교체 후 오래된 성공/오류/로그인 완료는 원래 객체만 가리키며 새 접속은 종료·상태 변경되지 않음 | 공통 완료 판정 + 지연 큐/취소 IOCP 테스트, worker 코드 대조 |
| AC-03 | 즉시 등록 실패와 중복 RECV 거부가 누수·이중 삭제 없이 처리됨 | 전송 회귀 및 소유권 약참조 검사 |
| AC-04 | 미완성 수신 프레임은 다음 작업 버퍼로 보존; 정상 송수신과 기존 월드 TCP 테스트 유지 | 실제 IOCP 분할 수신 테스트 + 기존 회귀 |
| AC-05 | 서버 Debug/Release 빌드와 회귀 전체 통과, 기존 프로토콜/클라이언트 소스 불변 | MSBuild, runner, diff 확인 |

## 구현 계획·작업

| 작업 | 연결 AC | 내용 | 상태 |
| --- | --- | --- | --- |
| T-01 | AC-01, AC-03, AC-04 | 작업 owner/heap receive, 제출·close 잠금, pending 보호 | 구현·테스트 통과 |
| T-02 | AC-02 | worker와 processPacket/disconnect를 원래 객체로 고정; 로그인 owner | 구현·코드 대조 + 공통 판정 테스트 통과 |
| T-03 | AC-01~04 | DB 없는 취소/지연/분할/교체 테스트 및 기존 테스트 적응 | 실행 통과 |
| T-04 | AC-05 | 빌드·회귀·문서 검사, 결과 기록 | 실행 통과 |

## 검증 결과

### 사전 기록

`39e98dd`에서 구현 전에 이 명세와 완료 기준을 커밋했다. 당시에는 코드 확인만 수행했고 실행 통과를 기록하지 않았다.

### 2026-10-28 실행·코드 대조

대상: `39e98dd` 이후 이번 feature 브랜치의 구현 변경. 최종 구현/병합 SHA는 #29에 기록한다. 환경: Windows, MSVC x64, assertions 활성.

- AC-01~04: [session_lifetime_test.cpp](../../tests/session_lifetime_test.cpp) 실행 통과. 실제 임시 포트 TCP/IOCP로 지연 SEND, pending RECV 취소와 LOGIN_OK 큐를 확인했다. 원래 owner/weak_ptr 수명, 같은 ID 교체 후 새 객체의 플래그·상태·버퍼 불변, 즉시 실패 해제, 중복 RECV 거부, 버퍼 경계 및 소유권 없는 세션 거부를 검사한다.
- AC-04: 한 바이트씩 전송하며 다음 작업에 prefix가 보존되는 것을 검사한다. 테스트에서 fragment를 직접 보관하므로 서버 worker의 파서 전체를 실행한 E2E 검증은 아니다. worker의 복사 대상과 원래 owner 사용은 코드 대조했다.
- AC-02: 실제 종료·DB 저장 함수를 테스트 서버에서 실행하지 않고, **생산 worker가 사용하는 동일한 완료 판정 함수**로 Ignore/Disconnect/Process를 확인한다. 잘못된 key, 성공/오류/0바이트 및 전송 실패 상태의 정리 분기를 검사했다. worker의 RAII 해제와 원래 객체 전달은 코드 대조했다. 전송 실패 뒤 shutdown으로 깨운 완료는 무시하지 않고 Disconnect로 처리한다.
- 명령: x64 Native Tools Command Prompt에서 `tests\run-regressions.cmd` → 전체 16개 통과. 수명 테스트를 같은 runner의 cl 명령으로 다시 빌드하고 `build\tests\session_lifetime_test.exe`를 10회 반복 → 20회 취소/재사용 × 10 = 추가 200회 통과.
- AC-05: MSBuild `TP_2023192003/TP_2023192003.sln /t:Build /p:Configuration=Debug /p:Platform=x64` 및 Release 통과. 기존 C4244·LNK4098 경고 유지. Debug 실행 파일 `--startup-check` 통과(DB·네트워크 없음). 클라이언트/프로토콜 파일 diff 없음.
- 실패 이력: 추가 경계 테스트의 initializer list에 signed/unsigned 혼합으로 컴파일 오류가 발생해 명시적 int 변환 후 재빌드·반복 검증했다. 통과 결과와 구분해 남긴다.
- 문서 UTF-8·상대 링크 및 `git diff --check` 검증은 [27단계 기록](../refactoring-step-27.md)에 연결한다.

## 인계

커밋·병합 결과는 #29에 연결한다. 전체 worker/timer와 DB·실제 화면 부하는 별도 검증이다. 정상 ID 재사용은 기존 세션의 disconnectStarted 설정 후 ST_FREE가 되는 현재 경로를 전제로 한다. 임의로 살아 있는 registry 항목을 덮어쓰는 운영 경로를 허용하는 설계가 아니다.

남은 위험: 전체 registry mapped shared_ptr/게임 데이터 동시성, 서버 종료 시 drain, 부분 송신 재시도, 실제 DB 계정 접속 부하. 이번 수정만으로 #10/#19의 실제 원인이 확정되거나 장시간 동접 안정성이 보증되지는 않는다.

다음 작업: 반복 접속/해제와 패킷 분할을 DB 없이 실제 worker 경로로 통과시키는 서버 통합 하네스의 범위를 별도 명세로 정한다.
