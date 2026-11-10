# 28단계: 실제 서버 worker 통합 테스트

이슈 [#30](https://github.com/s0o0o/26_Re_GMSEV/issues/30), [사전 명세와 AC별 결과](specs/server-worker-harness.md).

## 어디까지 했고 무엇이 달라졌나

27단계에서는 작업이 원래 접속을 보관하도록 고쳤지만, 테스트가 완료 판정과 fragment 보관을 직접 호출했다. 이번에는 테스트 실행 파일에 생산 `myserver.cpp`를 포함해 AcceptEx·worker·파서·로그인·이동·종료를 실제 TCP로 통과시킨다. **DB 함수만 메모리 대역**이며 실제 DB 구현은 링크하지 않는다.

- `tests/server_worker_test.cpp`: 두 연결, 분할 로그인/연속 이동, 잘못된 패킷, 실패 후 재시도, 30회 순차 재접속, 오래된 완료 주입, 시야·모든 섹터와 참조 해제 확인.
- `tests/run-server-worker.cmd`, `tests/run-regressions.cmd`: 단독 실행과 전체 회귀 17개에 연결.
- `myserver.cpp`: 테스트 전용 관찰 지점은 `GMSEV_WORKER_TEST`에서만 컴파일한다. 내부 IOCP 종료 신호로 worker를 join할 수 있게 했다. 실제 게임 패킷이나 정상 서버의 DB 설정을 바꾸지 않는다.

## 발견해서 고친 문제

로그인 전 MOVE에도 서버가 이동·맵 응답을 보내 첫 로그인 응답의 순서가 어긋나는 것을 재현했다. `processPacket()`에서 로그인 요청은 ST_ALLOC, 게임 요청은 ST_INGAME에서만 처리하도록 바꿨다. 로그인 후 재로그인으로 캐릭터 상태를 덮어쓰는 경로도 거부한다.

이것이 과거 로그인 시간 초과의 실제 원인이라는 뜻은 아니다. #19와 부하 이슈 #10은 유지한다.

## 검증과 한계

사전 명세 `f973898` → 실패 재현 → 수정 → 회귀 순서로 진행했다. 실제 worker 테스트, 전체 회귀 17개, 최종 하네스 10회 추가 반복(순차 재접속 300회), 서버 Debug/Release·DB 없는 startup-check 통과. 기존 빌드 경고는 남아 있다.

종료/ID 재사용 뒤 오래된 LOGIN_OK·0바이트 SEND는 순서를 제어하기 위해 인위적으로 IOCP에 게시했다. 일반 접속/수신/송신/close는 실제 소켓이다. 모든 weak_ptr 만료는 pending 참조와 registry의 정상 참조를 구분한 뒤 확인했다.

**동접 30명 테스트가 아니다.** 최대 peer 2개와 worker 1개이며, 실제 DB·Lua AI·타이머·게임 창·다중 worker 경합·장시간 부하는 실행하지 않았다. ODBC import가 없는 테스트 바이너리를 확인했다. 변경 Markdown UTF-8·상대 링크와 `git diff --check`를 검사하고 CP949 소스 인코딩을 보존했다.

명세는 `doc-coauthoring`의 맥락 수집·정제·독립 독자 검토로 다듬었다. 독자 검토가 지적한 완료 주입 방식과 해제 시점을 명시했다.

## 다음 작업

다중 worker·동시 접속/종료에서 registry와 세션 상태 접근이 안전한지 명세를 먼저 작성하고 검증 범위를 확장한다. 실제 DB 부하 검증은 별도 안내 후 진행한다.
