# 명세: 동시 이동·전투의 공통 월드 상태 보호

- 상태: 검증 완료 (아래 제한된 범위, 사전 명세)
- 이슈: [#32](https://github.com/s0o0o/26_Re_GMSEV/issues/32)
- 기준: `25547c4`, 2026-11-10. [접속 목록 명세](session-registry-concurrency.md) 이후 단계.

## 목적과 현재/목표 동작

registry 보호는 객체 수명만 보장한다. 좌표 두 개, HP/씨앗, 이름/외형, 시야와 sector 변경은 서로 다른 worker 및 패킷 빌더에서 동시에 접근할 수 있다. 기존 NPC combatMutex와 세션 completionMutex만으로 타인의 상태 읽기까지 보호하지 못한다.

이번 목표는 안전성 우선의 **단일 재귀 월드 상태 잠금**이다. 이동·공격·입장·퇴장·AI 작업 실행과 게임 상태를 읽는 패킷 생성/주변 검색을 같은 경계로 묶는다. registry 잠금은 여전히 짧은 포인터 복사 전용이다. 운영 처리량 향상을 주장하지 않으며, 추후 영역별/객체별 분리는 별도 검증한다.

## 범위·영향·제외

- 포함: 공통 `WorldStateLock`, 게임 패킷/주변 검색/Lua 좌표 읽기 보호, AI world→NPC 순서, DB 저장용 값 복사 후 world 잠금 해제, 실제 4-worker 동시 이동·동일 NPC 공격 검증.
- 서버 내부만 변경. 클라이언트·프로토콜·DB 스키마·게임 규칙은 유지한다.
- 제외: 실제 DB 계정/부하, 장시간 처리량, Lua 타이머 전체 실시간 실행, 정교한 lock sharding, 배포·전체 서버 shutdown.
- 부트스트랩과 테스트 준비에서 아직 공개하지 않은 객체의 초기화는 단독 실행이다. `GameObjectState`를 사용하는 신규 runtime 경로도 이 잠금 규칙을 따라야 한다.

## 잠금·오류 경계

순서: **자기 세션 completion → world → NPC combat/Lua/시야/sector/transport**. world를 잡은 채 다른 세션 completion을 얻지 않는다. AI는 NPC 잠금 전에 world를 잡는다. timer producer는 atomic 플래그/불변 ID만 읽고 world를 잡지 않는다. DB lookup은 world 밖, 결과 공개/저장 snapshot 복사는 world 안, 실제 DB update는 world 밖에서 수행한다.

로그인 실패, 전송 오류에 따른 종료, 사망과 예약된 AI, ID 재사용, 이동 중 logout에 대해서도 기존 소유권/세대 검사를 유지한다. 재귀 잠금은 기존 중첩 호출을 지원하기 위한 것이지 임의의 잠금 순서를 허용하는 뜻은 아니다.

## 완료 기준

| ID | 관찰 가능한 결과 | 검증 방법 |
| --- | --- | --- |
| AC-01 | 이동·전투·패킷 snapshot·AI 진입이 공통 잠금에 연결되고 역순 NPC→world가 없는 생산 호출 순서 | 코드 대조·기존 회귀·잠금 단위 검사 |
| AC-02 | 실제 4 worker에서 여러 접속의 연속 이동이 정확한 좌표/토큰으로 처리되고 시야·sector가 일관됨 | DB 대역 TCP 하네스, 교차 이동 및 종료 후 snapshot |
| AC-03 | 같은 NPC에 동시 공격해도 HP/씨앗 소모가 일치하고 사망 1회, 사망 뒤 HP/씨앗 변화 없음 | 생산 processPacket 경로 + 실제 전투/사망 응답 및 world snapshot |
| AC-04 | 로그인/공격/종료 저장은 잠금 안에서 복사한 값을 사용하고 실제 DB 함수 호출 때 world depth=0 | DB 대역 단언, 잠금 추적 API·저장 값 검사 |
| AC-05 | 기존 회귀·서버 Debug/Release 통과, 클라이언트·프로토콜 불변 | runner·MSBuild·diff |

## 계획과 검증 결과

| 작업 | AC | 상태 |
| --- | --- | --- |
| T-01 잠금 헤더·패킷/주변/AI 보호 | AC-01 | 구현·검사 통과 |
| T-02 로그인/공격/종료 DB snapshot·잠금 해제 | AC-04 | 구현·실행 통과 |
| T-03 실제 이동·공격 하네스와 잠금 테스트 | AC-02~04 | 실행 통과 |
| T-04 회귀/빌드/문서 검증 | AC-05 | 실행 통과 |

### 사전 기록

명세 커밋 `98f6ece`는 코드 확인 후 구현 전에 작성했다. 아래 결과는 이후 이번 브랜치 구현을 대상으로 한다. 최종 구현·병합 SHA는 #32에 연결한다.

### 2026-11-10 결과

환경: Windows/MSVC x64, assertions 활성. 실제 DB 없음.

- AC-01: [world_state.hpp](../../TP_2023192003/TP_2023192003/world_state.hpp)의 `WorldStateLock`을 추가했다. 기존 presence 잠금을 이 잠금으로 통합하고, 생산 게임 요청·주변 검색·좌표 API·패킷 생성·재생성·AI 처리에 연결했다. AI의 world 획득은 NPC combatMutex보다 먼저다. 중복 attackMutex는 제거했다. timer producer는 atomic 상태/불변 ID만 읽는 기존 방식을 유지한다. 코드 경로를 대조했으며 모든 미래 경로까지 자동 보호하는 private 데이터 구조로 바꾼 것은 아니다.
- AC-01: [world_state_test.cpp](../../tests/world_state_test.cpp) 통과. writer가 좌표와 HP/씨앗 쌍을 쓰는 중 yield를 넣고 reader 2개가 snapshot과 실제 이동/스탯 패킷을 만든다. 4,000개 이동·4,000개 스탯 패킷에서 불일치가 없고, 중첩/defer/unlock 후 thread-local 잠금 깊이가 맞는지 검사한다. 스탯 패킷의 누락된 id 초기화도 이 경계에서 보완했다. 단위 검사 값은 합성 상태이며 게임 맵 검증과는 별개다.
- AC-02/03: [실제 worker 하네스](../../tests/server_worker_test.cpp)의 [월드 모드 runner](../../tests/run-server-world.cmd) 통과. 실제 4 worker/4 TCP peer가 208개 이동 요청을 처리하며 x=39↔40 왕복으로 sector 변경 경로도 실행한다. 그중 두 peer는 총 17개 공격 요청, 다른 두 peer는 동시에 이동한다. 정지 NPC의 HP 5에 정확히 5회 유효 타격, 관찰자마다 동일한 HP 4→3→2→1→0 결과와 사망 1회, 전체 씨앗 40→35를 검사한다. 사망 뒤 추가 공격은 HP·씨앗을 바꾸지 않는다. 클라이언트가 damage/seedNum=9999를 주장해도 서버 값을 사용한다.
- 시야는 사망 NPC를 포함하지 않고 다른 플레이어 3명으로 유지되며, 좌표·move_time·HP·씨앗·sector와 종료 저장 값이 일치하는지 검사한다. 상태 snapshot은 테스트용 완료를 생산 worker에 게시하여 수집하므로 실제 클라이언트 UI 검증은 아니다.
- AC-04: 로그인 lookup/update·공격 update·종료 update의 DB 대역 진입에서 `WorldStateLock::heldByCurrentThread()==false`를 단언했다. 저장은 world 안에서 복사한 구조체를 사용한다. 종료 후 대역 저장 값이 최종 위치·HP·씨앗과 일치했다. 실제 ODBC 지연/실패·성능을 검사한 것은 아니다.
- Lua 검증은 테스트용 `event_player_move` 함수에서 실제 API_get_x/y를 호출하는 smoke 경로다. 잘못된 인수의 Lua pcall 오류 뒤 잠금 깊이가 남지 않음도 확인했다. RAII 잠금은 luaL_check* 인수 검증 뒤에 생성한다. 실제 npc.lua 전체와 실시간 timer producer/chase/respawn을 함께 돌리지는 않았다.
- 최종 월드 하네스와 snapshot 단위 검사를 각각 10회 추가 반복해 통과했다. 추가 2,080개 이동/170개 공격 요청이며 처리량 벤치마크나 장시간 동접 안정성 증거는 아니다.
- AC-05: `tests\run-regressions.cmd` 전체 21개 통과, 서버 x64 Debug/Release와 DB 없는 `--startup-check` 검증. 기존 C4244/LNK4098 경고 유지. 클라이언트·공유 프로토콜 diff 없음, 테스트 실행 파일의 ODBC/SQL import 없음 확인. 변경 문서 UTF-8/상대 링크와 diff 검사는 [30단계](../refactoring-step-30.md)에 연결한다.

### 한계와 다음 단계

안전성 우선의 coarse gate이므로 서로 다른 위치의 게임 상태 처리도 기다릴 수 있다. 경로 탐색·시야 계산·비동기 송신 제출도 해당 구간에 있어 실제 처리량/지연 측정이 필요하다. DB I/O는 밖이지만 세션 completion 직렬화와 worker의 DB 대기는 여전히 존재한다. 교착/데이터 경합의 수학적 증명이나 TSAN 검증은 아니다.

다음은 실제 Lua AI/타이머와 이동·전투·재생성을 함께 실행하는 테스트 범위를 명세로 정한다. 실제 DB 부하와 운영 처리량은 별도 승인·환경에서 검증한다. 이번 결과로 #10/#19의 과거 원인을 확정하지 않는다.
