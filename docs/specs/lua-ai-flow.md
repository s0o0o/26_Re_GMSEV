# 명세: Lua AI 역할 점검과 안전한 배회 정책 연결

- 상태: 구현·검증 완료 (2026-11-10)
- 이슈: [#33](https://github.com/s0o0o/26_Re_GMSEV/issues/33)
- 기준: `674dce4`, 2026-11-10. [월드 상태 명세](world-state-concurrency.md) 이후 단계.

## 목적·기존 동작

초기 `9b23f41`부터 npc.lua는 실제 로드/호출됐다. 그러나 현재까지 script 변경 이력이 없고, 하는 일은 같은 칸의 플레이어에게 HELLO를 보내는 것이다. 배회는 C++ rand, 추격은 C++ A*다. Lua dir/move_cnt와 event_npc_run은 실제 이동 경로에 연결되지 않았다. 이전 단계에서 수정한 Lua stack/API 문제와 이번 시작 시점에 남은 문제를 구분한다.

현재 남은 문제: callback 계약 검증 부족, 매번 getglobal/pcall/stack 정리의 중복, 정상 인사의 반복 전송 가능성, 미사용 RUN 함수, no-view 수면 누락, 경로 길이 1/없음에서 추격 예약 끊김. 오류가 반복되거나 무한 Lua 루프면 worker/world 잠금에 영향을 줄 수 있다.

## 범위·목표·영향

- `npc_script.hpp`에 보호 호출/stack 원복/handler 검사/명령 수 예산/좁은 기본 라이브러리 로드를 모은다. Lua state 수명은 공유 소유권으로 닫힌다.
- 실제 npc.lua에 `choose_direction()`을 추가해 C++ 배회가 그 결과를 사용한다. collision·world·A*·네트워크·타이머는 C++ 권한으로 유지한다. Lua의 남은 미연결 RUN 코드는 정리하되 C++ 예약 이벤트는 미사용으로 명시한다.
- 인사는 같은 칸에서 접속별 한 번으로 제한한다. 없는 좌표는 nil, ID 범위/발신 NPC 위조는 거부한다.
- 스크립트 오류는 NPC당 한 번 진단 후 script 사용을 중단하고 C++ 배회 fallback을 사용한다. 무한 루프는 instruction hook으로 끊는다. 전체 메모리/실시간/악성 코드 sandbox를 보장하는 것은 아니다. 프로젝트의 신뢰된 스크립트만 사용한다.
- 수면/추격의 다음 예약을 보완하고 명시적 timer 한 단계 실행으로 생산 worker 경로를 검증한다.
- 서버 내부만 변경. DB·클라이언트·프로토콜·전투 수치·AI 감지 거리와 이동 주기는 유지한다.

## 잠금·경계

기존 completion→world→NPC combat→Lua mutex 순서를 유지한다. Lua 인수 검증은 C++ RAII guard 전이다. C API에서 Lua 오류를 발생시킬 때 잡은 guard를 건너뛰지 않는다. 실패·반환형 오류·범위 밖 방향·종료한 대상·이전 generation·잘못된 script 파일을 검사한다. 기본 라이브러리는 base/math/string/table/utf8만 열고 os/io/package/debug는 열지 않는다. instruction budget은 C 라이브러리의 긴 실행이나 메모리 고갈까지 제한하지 않는다.

## 완료 기준

| ID | 관찰 가능한 결과 | 검증 |
| --- | --- | --- |
| AC-01 | 실제 npc.lua의 계약/방향 0..3/인사 접속별 1회가 동작하고 C++ 배회가 Lua 방향을 따름 | 실제 파일 Lua 단위 + 생산 이동 경로 |
| AC-02 | 반복 callback, 오류·잘못된 반환·누락 함수·무한 루프에도 stack 높이가 복원되고 worker가 이어 동작 | 문자열/파일 fixture, instruction hook, NPC 오류 1회 fallback |
| AC-03 | API 좌표 부재 nil·범위 검증·메시지 발신자/수신자 검사가 유지됨 | 실제 보호 호출/C API와 stack 검사 |
| AC-04 | 플레이어 없음이면 수면, no-path/same-tile 추격에서 배회 재개, stale generation 거부 | 실제 timer dispatcher 한 단계 → IOCP worker 검사 |
| AC-05 | 기존 전체 회귀, 서버 Debug/Release·DB 없는 startup-check 통과 | runner·MSBuild·정상 actual npc.lua |

## 계획·검증 결과

| 작업 | AC | 상태 |
| --- | --- | --- |
| T-01 script 호출 모듈·수명/계약/예산 | AC-01~03 | 구현, 단위/실제 C API 검사 통과 |
| T-02 Lua 방향 연결·인사/수면/추격 예약 | AC-01, AC-04 | 구현, 생산 함수/worker 실행 통과 |
| T-03 실제 파일·생산 worker/timer 경로 검사 | AC-01~04 | 아래 실행 증거 참조 |
| T-04 회귀/빌드/설명 기록 | AC-05 | 23개 회귀, 서버 Debug/Release, startup-check 통과 |

### 실행 증거와 한계

- AC-01: `npc_script_test.cpp`가 실제 npc.lua의 방향 1,000회와 동일 접속 인사 1회/새 접속 추가 1회를 확인. `server_worker_test.cpp` Lua 모드가 같은 RNG 상태에서 예상 방향을 구하고 실제 timer dispatcher→4-worker 이동 좌표와 비교했다.
- AC-02: 누락/문법 오류/잘못된 반환/문자열 아닌 오류/최상위 및 callback 무한 반복, stack sentinel와 hook 복원 검사 통과. 실제 worker의 무한 반복은 NPC 1회 비활성화 후 C++ fallback, 다음 tick에서 Lua 호출 횟수 1 유지, 플레이어 이동 응답까지 통과.
- AC-03: 실제 C API의 없는 좌표 nil/음수 ID/잘못된 인수/발신자 위조 거부와 stack/잠금 복원 확인. 실제 NPC 좌표 API는 실제 스크립트 및 기존 world fixture callback 경로에서 사용했다. 모든 수신자 경계 조합을 네트워크로 전수 검사하지는 않았다.
- AC-04: same-tile 추격→배회, 마지막 접속 종료→수면, 활성 NPC의 오래된 generation timer 제거를 실제 함수/IOCP에서 실행. no-path 분기는 same-tile과 같은 `path.size() <= 1` else를 사용함을 코드 확인했지만, 별도의 막힌 맵 fixture는 미실행. timer 생산의 단일 단계를 호출했으며 무한 timer thread의 실시간 장기 실행 검증은 아니다.
- AC-05: `tests/run-regressions.cmd` 23개 통과. MSBuild 서버 x64 Debug/Release 성공, 실제 npc.lua로 `--startup-check` 성공(DB/네트워크 없음). 기존 C4244/LNK4098 경고는 남아 있다.
- Lua 단위/통합 실행 파일 각각 10회 추가 반복 통과. 테스트를 위한 강제 접속 종료/accept 취소 로그는 예상 경로다.

실제 DB·장시간 동접 부하·20/30초 실시간 복구 대기·클라이언트 그래픽 육안 검증은 제외했다. instruction hook은 메모리/벽시계 시간/오래 실행되는 C 라이브러리 호출을 제한하지 않는다. 최종 커밋과 main 반영은 #33에 기록한다. 자세한 설명은 [31단계 기록](../refactoring-step-31.md).
