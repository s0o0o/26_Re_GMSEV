# 명세: 접속 목록 동기화와 다중 worker 입장·퇴장

- 상태: 검증 완료 (아래 제한된 범위, 사전 명세)
- 이슈: [#31](https://github.com/s0o0o/26_Re_GMSEV/issues/31)
- 기준: `94f27f0`, 2026-11-10. [이전 worker 검증](server-worker-harness.md), [SDD 절차](../sdd.md).

## 목적·현재 동작·목표

현재 concurrent_unordered_map은 구조적 접근을 지원하지만 map 값인 같은 shared_ptr의 조회/대입까지 보호하지 않는다. 반복자로 값을 계속 참조하거나 숫자 ID를 조회할 때 교체가 겹치면 안전하지 않다. 슬롯 선택과 등록도 별도 단계다.

목표는 `get/set/snapshot/allocatePlayer/clear` API 안에서 map 접근을 잠그고 **복사한 shared_ptr만** 반환하는 것이다. 작업 중 접속 객체를 유지하되 그 객체의 게임 데이터까지 자동으로 잠근다는 의미는 아니다. 슬롯 선택과 ST_ALLOC 등록은 한 임계 구역에서 처리한다. 입장 브로드캐스트와 퇴장 정리는 별도 presence 잠금으로 직렬화해 정리 후 늦은 입장 추가를 방지한다. DB 저장은 presence 잠금 밖에서 유지한다.

## 범위·제외·영향

- 포함: registry API와 모든 호출부 전환, 원자적 플레이어 슬롯 할당, 입장/퇴장 경계 잠금, registry 경합 테스트, 실제 4-worker의 고정 위치 로그인·퇴장 배치 테스트.
- 제외: 이동/전투/Lua/NPC/타이머의 게임 데이터 전체 동기화, 실제 DB·UI·장시간 부하·운영 동접 상한. 이번 다중 worker 시나리오에서는 로그인 이후 좌표를 변경하지 않는다.
- 서버만 변경. 클라이언트·공유 프로토콜·DB 스키마는 바꾸지 않는다. DB 대역 하네스를 유지해 실제 계정 데이터는 변경하지 않는다.
- 잠금 순서: 세션 completion → presence → 시야/sector/NPC/transport의 기존 국소 잠금. registry 잠금은 포인터 복사/등록에만 사용하고 다른 잠금·송신·DB 호출을 잠금 내부에서 실행하지 않는다.

## 오류·경계와 완료 기준

| ID | 관찰 가능한 결과 | 검증 방법 |
| --- | --- | --- |
| AC-01 | 없는 ID 조회는 삽입하지 않음; 조회/snapshot은 교체·clear 후에도 원래 객체를 유지; 읽기 참조/반복자 노출 없음 | registry 단위 검사·API 호출부 검색 |
| AC-02 | 여러 스레드의 동시 get/set/snapshot/clear가 완료되고 포인터 ID/초기값이 유효; 모든 외부 참조 해제 후 소멸 | 시작 barrier가 있는 경합 테스트·weak_ptr |
| AC-03 | 여러 스레드의 플레이어 할당에서 서로 다른 ID/ST_ALLOC 확보, FREE 슬롯 재사용 시 새 connectionId; 용량 초과·null/이미 등록된 객체 거부 | 원자적 할당 단위 테스트 |
| AC-04 | 실제 4 worker, 최대 16개 TCP 연결이 고정 위치 로그인·동시 종료를 배치 반복하고 누락·유령 시야·sector 잔여·pending 참조 없이 종료 | 기존 worker 하네스의 다중 worker 모드, DB 대역, 상태 snapshot |
| AC-05 | 기존 단일 worker·전체 회귀 및 서버 Debug/Release 통과; DB/클라/프로토콜 변경 없음 | runner·MSBuild·diff |

원자적 registry 접근만으로 모든 게임 데이터 경합이 해결됐다고 주장하지 않는다. 슬롯 재사용은 정상 disconnect가 ST_FREE로 전환한 뒤만 허용한다. snapshot은 목록 시점의 복사이며 이후 객체 상태 변화까지 고정하지 않는다.

## 구현 계획

| 작업 | AC | 상태 |
| --- | --- | --- |
| T-01 registry API·조회/반복/대입 호출부 전환 | AC-01~03 | 구현·검사 통과 |
| T-02 원자적 슬롯 등록·presence 경계 | AC-03~04 | 구현·검사 통과 |
| T-03 경합 단위 검사·4-worker 하네스 확장 | AC-02~04 | 실행 통과 |
| T-04 회귀·빌드·문서 검증과 결과 기록 | AC-05 | 실행 통과 |

## 검증 결과·인계

### 사전 기록

명세 커밋 `bc2125a`에서는 구현 전 코드 확인만 수행했다. 아래는 이후 이번 브랜치의 구현·실행 결과다. 최종 구현/병합 SHA는 #31에 연결한다.

### 2026-11-10 결과

환경: Windows/MSVC x64, assertions 활성, 실제 DB 미사용.

- AC-01: [SessionRegistry](../../TP_2023192003/TP_2023192003/session_registry.hpp)의 private unordered_map을 mutex로 감쌌다. get은 없는 키를 추가하지 않고 shared_ptr 복사만 반환한다. snapshot은 벡터 복사이며 map 반복자가 아니다. 교체·clear에서 물러난 객체는 registry 잠금을 푼 뒤 해제한다. 전체 호출부를 get/set/snapshot으로 전환하고 빈 조회를 방어했다. 기존 clients.find/end/operator[] 사용은 검색상 남아 있지 않다.
- AC-02/03: [session_registry_test.cpp](../../tests/session_registry_test.cpp) 통과. barrier로 시작하는 writer 4·reader 4·clear 1개 스레드가 각 1,000회 반복한다. 8개 할당 스레드 × 32개 = 256개 슬롯의 유일성을 확인했다. FREE 재사용 시 새 접속 번호, null/기등록 객체 거부, MAX_USER 전체 점유 시 -1 반환, 외부 복사 유지 및 weak_ptr 해제를 확인했다. 단위 테스트이며 데이터 경합 탐지기/수학적 안전성 증명은 아니다.
- AC-04: [생산 worker 하네스](../../tests/server_worker_test.cpp)를 [다중 worker runner](../../tests/run-server-multiworker.cmd)로 실행해 통과했다. 실제 worker 4개·최대 TCP 16개로 5개 배치, 총 80개 접속 수명을 검사한다. 앞 4개 배치는 동시 로그인/동시 종료, 마지막은 기존 8명 종료와 다른 8명 로그인을 겹친다. 후자의 최종 시야가 살아 있는 7명뿐인지 개수와 실제 ID 집합으로 확인한다. 좌표는 로그인 뒤 바꾸지 않는다.
- 다중 worker 모드는 테스트용 OP_LOGIN_FAIL 완료를 게시해 각 세션의 completionMutex 안에서 현재 시야/섹터 snapshot을 수집한다. 이는 실제 로그인 실패를 발생시킨다는 뜻이 아니다. TCP 입장 패킷의 ID 유일성과 종료 시 모든 섹터·시야 정리, 대역 저장 2회, pending 참조 drain·ID 재사용·weak 만료도 검사한다.
- 최종 overlap 하네스 및 registry 검사를 각각 단독 10회 추가 반복 → 추가 접속 수명 800개 통과. 이는 최대 동접 16명이며 동접 800명이나 장시간 부하를 의미하지 않는다.
- AC-05: `tests\run-regressions.cmd` 전체 19개 통과. 정상 서버 MSBuild x64 Debug/Release 및 DB 없는 `--startup-check` 검증. 기존 C4244/LNK4098 경고 유지. 클라이언트·공유 프로토콜 diff 없음. 테스트 바이너리 ODBC/SQL import 없음 확인.
- 구현 중 하네스에서 `<set>` 누락으로 컴파일 실패가 있어 헤더 추가 후 재실행했다. presence 경합은 코드 흐름에서 위험을 확인해 보호했고, 수정 전 운영 크래시나 실패 테스트로 원인을 확정한 것은 아니다. #10/#19 원인과 동일하다고 주장하지 않는다.

### 한계와 다음 작업

get/snapshot은 객체 수명만 유지한다. 객체의 좌표·HP·이름·다른 필드까지 잠그지 않는다. 이동/전투/Lua AI가 섞인 다중 worker 실행, 실제 DB 부하, 전체 서버 종료/드레인, 장시간 안정성은 미검증이다. snapshot은 O(N) 복사로 해당 시간 동안 등록/조회가 대기할 수 있어 처리량 측정도 별도다. live world의 임의 clear/set은 정상 재접속 절차를 대체하지 않는다.

다음은 이동·전투 중 객체 상태 접근 경계를 별도 명세로 다룬다. 문서·Git 인계는 [29단계 기록](../refactoring-step-29.md)과 #31에 연결한다.
