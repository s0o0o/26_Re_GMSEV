# 명세: Lua AI 역할 점검과 안전한 배회 정책 연결

- 상태: 구현 중 (사전 명세)
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
| T-01 script 호출 모듈·수명/계약/예산 | AC-01~03 | 예정 |
| T-02 Lua 방향 연결·인사/수면/추격 예약 | AC-01, AC-04 | 예정 |
| T-03 실제 파일·생산 worker/timer 경로 검사 | AC-01~04 | 예정 |
| T-04 회귀/빌드/설명 기록 | AC-05 | 예정 |

구현 전에는 코드와 초기 이력만 확인했다. 실행 증거와 한계/최종 커밋은 이후 이 명세와 #33에 기록한다. 실제 DB·장시간 부하·실제 시간 20/30초 대기 검증은 제외한다. 다음 작업은 이번 검사에서 드러난 남은 경계를 기준으로 정한다.
