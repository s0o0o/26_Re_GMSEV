# 31단계: Lua는 실제로 무엇을 했고, 이번에 무엇을 바꿨나

이슈 [#33](https://github.com/s0o0o/26_Re_GMSEV/issues/33), [완료 기준과 실행 증거](specs/lua-ai-flow.md).

## 결론: Lua는 사용했지만 이동 AI 전체는 아니었다

초기 커밋 `9b23f41`과 작업 시작 기준 `674dce4`를 확인했다. 기존 npc.lua는 초기 추가 이후 수정 이력이 없었다. 서버가 NPC마다 Lua VM을 만들고 파일을 읽었으며, 플레이어 이동 이벤트에서 Lua 함수를 실제 호출했다. 단, 학습형 AI가 아니라 규칙 기반 NPC 스크립트였다.

| 행동 | 기존 담당 | 이번 이후 |
| --- | --- | --- |
| 같은 칸 플레이어에게 HELLO | Lua event_player_move | Lua, 동일 접속에는 한 번만 |
| 임의 방향 배회 | C++ rand()%4 | Lua choose_direction 결과를 C++가 적용 |
| 벽/맵 충돌, 이동 패킷, 주기 예약 | C++ | C++ 유지 |
| 공격받은 NPC의 A* 추격 | C++ | C++ 유지, 다음 예약 누락 보완 |
| event_npc_run/dir/move_cnt | 이동에 연결되지 않은 코드 | Lua에서 제거, C++ RUN은 미사용 예약으로 명시 |

기존 흐름은 `npc.lua 로드 → set_uid → 플레이어 이동 → wakeUpMonster → IOCP OP_PLAYER_MOVE → event_player_move → 같은 칸이면 API_SendMessage`였다. 따라서 Lua를 전혀 쓰지 않은 것은 아니다. 다만 Lua에 있던 RUN 함수가 캐릭터를 실제 달리게 만들었다고 볼 호출 경로는 없었다.

초기 C API의 인수 과다 pop 같은 stack 문제도 확인했지만, 그 부분은 앞선 리팩토링에서 이미 수정된 내용이다. 이번 변경의 성과와 구분한다. 또한 이번 결과만으로 예전 동접 20~30명 종료나 로그인 timeout의 원인이 Lua였다고 확정할 수 없다.

## 문제와 변경 위치

### npc_script.hpp: 반복되던 Lua 실행 코드를 한곳으로

기존 myserver.cpp는 파일 로드/함수 조회/pcall/error pop을 직접 처리했다. 함수가 빠지거나 반환이 틀린 경우를 일관되게 검증하지 못했고, pcall만으로 무한 반복은 멈추지 않았다.

새 모듈은 필수 함수 set_uid/event_player_move/choose_direction을 검사하고, 호출 전 stack 높이를 저장해 성공·실패 뒤 원복한다. 함수 조회까지 보호 호출 안에서 수행한다. 10,000 VM 명령 단위 hook으로 과도한 실행을 오류로 끊고 기존 hook도 복원한다. VM은 공유 소유권의 마지막 참조가 사라질 때 닫힌다.

base/math/string/table/utf8만 열고 os/io/package/debug 및 파일 로딩 도우미를 열지 않는다. pcall/xpcall도 script에 노출하지 않아 예산 오류를 내부에서 삼키며 계속 반복하는 패턴을 막는다. 그러나 악성 스크립트를 위한 완전한 sandbox는 아니다. 메모리 한도나 C 함수 실행 시간 제한은 없으며 프로젝트의 신뢰된 코드만 사용한다. 보호 호출과 count hook의 의미는 [공식 Lua 5.4 매뉴얼](https://www.lua.org/manual/5.4/manual.html)을 기준으로 했다.

### npc.lua와 myserver.cpp: 행동 정책과 실제 이동 연결

`choose_direction()`은 0 오른쪽/1 왼쪽/2 아래/3 위를 반환한다. C++ `doMonsterRandomMoving()`이 이 값을 받아 맵 통과 가능 여부를 검사하고 좌표와 패킷을 갱신한다. Lua가 좌표를 마음대로 바꾸는 구조는 아니다.

`event_player_move(player, connection)`은 플레이어 ID뿐 아니라 접속 식별자도 받아 같은 접속에서 인사를 반복하지 않는다. 슬롯 ID가 재사용돼도 다른 접속이면 다시 인사한다. 좌표 조회 대상이 없으면 0 대신 nil이라서 정상 원점과 구분된다. C API는 ID 범위를 검사하고 메시지 발신자가 해당 Lua VM의 NPC인지 확인한다. Lua 인수 오류 검사는 C++ 잠금을 잡기 전에 한다.

런타임 오류는 `[AI] Lua disabled npc=... event=... error=...`로 NPC당 한 번 기록하고 해당 NPC는 C++ 임의 배회로 전환한다. 다음 tick에서도 고장 난 Lua를 계속 호출하지 않는다. 자동 재활성화/핫 리로드는 없으므로 script 수정 후 서버를 재시작한다. 시작 시 파일이 없거나 계약이 틀리면 서버 초기화를 실패시킨다.

### 수면과 추격 예약

`trySleepMonster()`는 주변 목록이 비면 오히려 바로 return하던 코드 때문에 아무도 없어도 활성 상태가 남을 수 있었다. 이제 수면으로 전환하고 추격을 해제하며 generation을 올려 이전 예약을 무효화한다. 수면 뒤에는 다음 배회 예약도 만들지 않는다.

추격에서 A* 경로가 없거나 길이 1(이미 같은 칸)이면 다음 이벤트를 만들지 않아 행동이 끊기던 분기를 `resumeNpcRoaming()`으로 연결했다. 타이머 생산은 `dispatchDueTimer()` 한 단계로 분리해 실제 IOCP 경계까지 검사 가능하게 했다. 오래된 generation/죽은 NPC/없는 NPC는 버리고, IOCP 제출 실패 시 작업 메모리를 해제한다.

## 검증과 다음 작업

23개 전체 회귀 프로그램, 서버 Debug/Release 빌드, DB 없는 실제 npc.lua startup-check가 통과했다. Lua 단위와 4-worker 통합은 각각 10회 추가 반복했다. 실제 파일의 방향 선택, 인사 중복 제한, 오류 stack 원복, 무한 반복 차단, C API 인수 거부, 실제 worker의 fallback 뒤 이동 응답, 같은 칸 추격 종료, 마지막 플레이어 종료 뒤 수면, 오래된 timer 제거를 확인했다. 상세 AC별 실행/코드 확인/미검증 구분은 명세를 따른다.

테스트 중 초기 실패는 NPC 알림 완료와 이동 완료를 같은 관찰 이름으로 기다리던 fixture 문제였다. 두 관찰을 분리한 뒤 재실행했다. DB는 대역이며 실제 계정/캐릭터 데이터를 변경하지 않았다. 기존 C4244/LNK4098 경고는 별도 정리가 필요하다.

다음 작업은 C++에 남은 NPC 이벤트·배회·추격 처리 코드를 전용 모듈로 분리하는 것이다. 이번에 만든 보호 호출과 실제 worker 회귀 검사를 유지한 채 main 파일의 AI 책임을 줄이는 순서가 적절하다. 성격별 NPC나 Lua 추격 정책 확대는 그 다음에 진행한다.
