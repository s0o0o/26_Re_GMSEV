# 서버 IOCP·패킷·연결 수명

갱신: 2026-11-16, #54. [구조 입구](README.md).

## 용어와 큰 흐름

TCP는 순서있는 **바이트 스트림**이고, 게임패킷은 그 바이트를 로그인/이동 등으로 해석하는 프로젝트 계약이다. IOCP는 Windows에 맡긴 비동기입출력의 **완료**를 worker가 받아 처리하는 방식이다. 연결마다 기다리는 스레드를 하나씩 만드는 방식이 아니다.

```text
main → AcceptEx 요청 → 완료큐 → workerThread(OP_ACCEPT)
     → GAMESESSION + 소켓등록 → registerRecv/WSARecv
     → 완료큐 → OP_RECV → 패킷분리/검증 → processPacket
     → 패킷작성함수 → registerSend/WSASend → OP_SEND 완료정리
```

## 실제 파일·함수

| 위치 | 찾아볼 함수/타입 | 책임 |
| --- | --- | --- |
| [myserver.cpp](../../TP_2023192003/TP_2023192003/myserver.cpp) | main | DB/월드초기화, Winsock, bind/listen, IOCP/worker/timer시작 |
| 같은 파일 | workerThread | GetQueuedCompletionStatus로완료를꺼내종류별분기 |
| 같은 파일 | processPacket | 로그인/이동/공격/채팅요청처리 |
| [game_session.cpp](../../TP_2023192003/TP_2023192003/game_session.cpp) | registerRecv/registerSend/closeTransport | 비동기요청과소켓종료 |
| [iocp_operation.hpp](../../TP_2023192003/TP_2023192003/iocp_operation.hpp) | OVER_EXP/COMP_TYPE | OVERLAPPED·버퍼·작업종류·원래연결owner |
| [session_packets.cpp](../../TP_2023192003/TP_2023192003/session_packets.cpp) | sendSessionLoginPacket/sendMovePacket/sendAddPlayerPacket/sendMapInfo | 상태를wire v6로직렬화 |
| [session_completion.hpp](../../TP_2023192003/TP_2023192003/session_completion.hpp) | classifySessionCompletion/releaseSendCompletionWithoutLock | 실패/옛연결/정상송신완료분류 |
| [session_registry.hpp](../../TP_2023192003/TP_2023192003/session_registry.hpp) | SessionRegistry | ID별연결등록/조회/스냅샷 |

## 1. 준비와 접속

main은 DB와 월드를 초기화하고 WSAStartup → WSASocket(WSA_FLAG_OVERLAPPED) → bind/listen을 수행한다. CreateIoCompletionPort로 큐를 만들고 서버 소켓을 연결한다. AcceptEx로 접속 대기를 요청한다. worker 수는 현재 hardware_concurrency×2로 설정되어 있다. 이것이 성능 최적값이라는 보장은 아니다.

접속 완료 OP_ACCEPT에서 GAMESESSION을 만들고, 접속 소켓을 세션 ID completion key로 IOCP에 연결한다. registerRecv로 첫 수신을 요청하고 다음 접속을 위해 AcceptEx를 다시 건다.

## 2. 수신과 framing

registerRecv는 작업 객체에 수신 버퍼와 이전 조각을 보관한 뒤 WSARecv를 호출한다. WSA_IO_PENDING은 정상 진행 중이지 실패가 아니다. recvPending으로 연결당 중복 수신 요청을 막는다.

worker의 GetQueuedCompletionStatus는 처리 바이트 numBytes, 등록 key, 작업 OVERLAPPED 포인터를 준다. 완료가 실패했어도 작업 포인터가 있을 수 있으므로 반환값과 포인터를 함께 분류한다.

OP_RECV는 기존 조각과 새 바이트를 합쳐 [길이 1바이트, type 1바이트, body]를 반복 분리한다. 길이·타입 검증 뒤에만 processPacket을 호출한다. 한 패킷이 여러 수신으로 나뉘거나 여러 패킷이 한 수신에 합쳐져도 처리하며, 남은 조각은 다음 registerRecv로 넘긴다. 수신 요청은 한 번 걸고 영구 수신되는 것이 아니라 완료마다 다시 등록한다.

## 3. 로그인과 내부 큐 작업

processPacket(C2S_P_LOGIN)은 버전·외형·닉네임 검사 → 동기 DB 조회/필요시 생성 → 위치 복원·상태 준비를 수행한다. 성공 후 PostQueuedCompletionStatus로 OP_LOGIN_OK를 직접 게시한다. **그 작업을 큐에서 꺼낸 worker**가 본인 정보·로그인 완료를 보내고 ST_INGAME으로 전환하여 지도·시야 등록과 주변 ENTER를 처리한다. 처음 로그인 요청을 처리했던 스레드와 같을 필요는 없다. ST_ALLOC은 소켓만 준비된 상태이지 게임 입장 완료가 아니다.

PostQueuedCompletionStatus는 DB를 비동기로 만드는 함수가 아니다. myserver.cpp의 doTimer가 게시하는 몬스터 이동·추격·월드 tick도 같은 큐에서 worker가 처리한다. 큐에는 OS 통신 완료와 앱이 만든 내부 작업이 함께 있다. 월드 tick의 실제 처리는 같은 파일의 serviceRespawns다.

## 4. 송신·버퍼·종료

registerSend는 패킷을 OVER_EXP의 버퍼로 복사한 뒤 WSASend를 요청한다. 지역변수 패킷은 함수가 끝나면 사라지지만 비동기 작업 버퍼는 완료까지 살아야 한다. 정상 전체 송신 완료는 작업 객체를 해제하고 끝난다. 송신 완료는 상대 게임 화면 반영이나 앱 수신 확인(ACK 패킷)을 뜻하지 않는다.

현재 부분 송신 완료는 재전송 큐로 이어가지 않고 연결 오류로 정리한다. 즉시 제출 실패는 완료 통지가 오지 않는 경로라 그 자리에서 작업을 해제한다. 접수된 작업은 완료 경로에서 해제한다.

myserver.cpp의 disconnect는 중복 호출을 막고 월드 시야·섹터에서 제거 → 소켓 닫기 → ST_FREE → 게임에 입장했던 상태의 DB 저장 순서로 처리한다. DB는 world 잠금 밖에서 실행한다. closesocket 후에도 이미 진행 중인 작업 완료가 올 수 있다. 각 작업의 shared_ptr owner가 원래 연결을 살려두며 ID 재사용 후 새 연결과 혼동하지 않게 한다.

### 상태와 식별자 정의 위치

- [iocp_operation.hpp](../../TP_2023192003/TP_2023192003/iocp_operation.hpp)의 STATE: ST_FREE(종료/비활성), ST_ALLOC(연결 확보·입장 전), ST_INGAME(게임 입장).
- [game_session.hpp](../../TP_2023192003/TP_2023192003/game_session.hpp)의 connectionId: 같은 세션 ID를 다시 쓰더라도 접속 회차를 구분한다. owner와 함께 연결 수명 추적에 사용한다.
- [monster_state.hpp](../../TP_2023192003/TP_2023192003/monster_state.hpp)의 generation: 이전 사망·추격 상태에서 만든 AI 이벤트를 새 상태에 적용하지 않기 위한 번호다. connectionId와 같은 개념이 아니다.

## 동기화와 병목

#64/#65 지역 채팅은 기존 world gate 안에서 UTF-8/119byte/연결별500ms와같은지역ingame수신자를검사한다. 클라이언트제공id가아닌sessionId를사용한다. 패킷v6/message120으로확장했고DB호출없음. [채팅 흐름](chat.md).

- transportMutex: 소켓제출/종료와송수신요청상태.
- completionMutex: 같은연결의완료처리직렬화. 정상전체SEND/이미종료중SEND는부가게임작업없이해제하는빠른경로가있다.
- WorldStateLock: 위치·시야·전투주변의월드일관성. 순서는연결완료→world→NPC/view/sector/transport.
- databaseMutex: ODBC호출직렬화. world밖이어도worker자체는동기DB호출동안묶인다.

IOCP가 잠금 경합·느린 DB·잘못된 세션 수명을 자동으로 해결하지는 않는다. #41의 추가 부하 조건과 전체 서버 graceful shutdown은 아직 별도 검증이다. 운영 종료·IOCP 완전 drain 명령은 현재 구현되어 있지 않다.

## #53 귀환 요청과 native 부분 송신

귀환은 v3에서 추가했고 현재 공유 패킷은 옷장 계약 v6다. C2S_P_TELEPORT(12)는 action/requestId, S2C_P_TRAVEL_RESULT(20)는 판정 결과다. 외형 요청21/결과22/변경23과 로그인 외형 조회·오류 처리는 [옷장](wardrobe.md), 귀환은 [전환 흐름](travel.md), 지형은 [청크/범위 계약](field-chunks.md)을 참조한다. 이는 OS 송신 완료와 다른 앱 응답이다. native PacketOutbox의 부분 송신 재시도는 서버 IOCP의 부분 송신 오류 정책을 바꾸지 않는다.
