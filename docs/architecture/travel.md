# 거점·필드 이동 (#53)

2026-11-16. [완료 기준·실행·main 반영 기록](../specs/hub-field-travel.md).

## 요청과 서버 판정

기존 no-op이던 `C2S_P_TELEPORT(12)`를 명시적 `Action + requestId`로 확장했다. 공유 프로토콜은 v3이며 서버·일반 클라·GUI/콘솔 도구를 함께 다시 빌드해야 한다. 옛 2바이트 요청은 길이 검사를 통과하지 않는다. `S2C_P_TRAVEL_RESULT(20)`는 요청 ID와 결과를 반환한다.

`processPacket → session.travel.request → transitionRegion` 순서다. EnterField는 SunnyPeak 내부 (10,10), ReturnHome은 공용 Clubhouse 내부 (11,12)로만 이동한다. 클라가 좌표/지역을 임의로 지정하지 않는다. 개인 집 기능은 아니다.

Journal은 연결마다 독립적이다. 양수·올바른 action만 받고, 동일 ID/action은 저장된 결과만 돌려준다. 과거 ID/잘못된 action, 이미 같은 지역, 막힌 목적지는 위치를 바꾸지 않는다. 포털 이동도 같은 전환 함수를 사용한다.

## 전환과 잠금

IOCP가 해당 연결의 completionMutex를 보유하고 processPacket/transitionRegion이 WorldStateLock을 잡는다. 목적지 검증 후 이전 시야의 LEAVE를 본인에게도 명시적으로 보낸다. detachWorldPresence가 이전 섹터·본인/관찰자 시야·추격 generation을 정리한다. Position 변경·섹터 등록 후 본인 MOVE → map → 새 관찰자/대상 ENTER → near 목록 순서로 보낸다. native는 새 map/ENTER보다 먼저 받은 본인 MOVE의 지역 변경 시 이전 플레이어/맵/전투 캐시도 버린다.

DB 저장은 이 함수에서 하지 않는다. 기존 로그인/종료 저장과 지역↔legacy DB codec 경계를 유지한다. 새 DB 열/계정을 만들지 않았다.

## 클라 상태와 버튼

handleTravelInput → ClientState.begin → send_packet/PacketOutbox → 서버 MOVE/결과 → TravelPanel 순서다. 서버 응답 이전에 위치를 바꾸지 않는다. 채팅 중에는 요청하지 않는다. R키는 KeyReleased까지 잠가 누른 채 왕복하지 않도록 한다. pending/연결 종료 중에는 Escape 외 KeyPressed(이동·공격·Tab 포함)를 소비한다.

ClientState는 일치 ID만 처리하고 5초 timeout 후 재시도를 허용한다. 지연된 결과가 새 pending을 완료시키지 않지만 권위 있는 MOVE는 정상 반영한다. PacketOutbox는 부분 송신의 남은 바이트를 다음 프레임에도 재시도하며 패킷 순서를 유지한다. 로그인은 기존 별도 송신 경로다. gameConnectionLost는 outbox·pending·채팅을 정리하고 연결 종료를 표시한다. 예전처럼 수신 오류에서 즉시 exit하지 않는다. 재접속은 종료 후 재실행이며 자동 재접속은 아니다.

travel_panel.hpp는 900×800 논리 좌표 우상단 버튼과 상태를 그린다. 클릭은 기존 창의 mapPixelToCoords로 변환하여 letterbox 위치와 맞춘다. 기존 픽셀 지도·글꼴·팔레트는 유지한다.

## 검증 경계

tests/run-travel.cmd는 규칙/클라 상태/outbox 단위 검사와 실제 TCP·IOCP worker 4개의 이동/관찰자/중복/추격 무효화/저장을 검사한다. DB는 대역이다. run-travel-client.cmd는 생산 client.cpp/파서를 포함하여 TCP 요청/분할 응답, R키/클릭 이벤트와 상태별·해상도별 생산 렌더러 미리보기를 검사한다. 실제 OS 입력이나 실제 DB 접속 캡처는 아니다. 전체 회귀·6빌드 최종 결과는 명세에 기록한다.
