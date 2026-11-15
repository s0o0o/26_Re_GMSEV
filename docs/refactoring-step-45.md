# 45단계: 지역 내부 좌표 통신 v2 (2B-2A)

이슈 [#49](https://github.com/s0o0o/26_Re_GMSEV/issues/49), 기준1515f2c, [사전 명세](specs/region-wire-v2.md).

## 기존 문제와 이번 범위

필드의 내부(10,10)이 통신에도 기존 전역(42,10)으로 전달됐다. 이 방식은 지역 배치 오프셋을 클라·서버·스트레스가 함께 알아야 해 지역 확장 시 얽힌다. 이번에는 **wire만 region+local로 전환**하고 게임상태/렌더러의 legacy 의존은 명시적인 임시 어댑터로 남겼다. 2B-2 전체 완료나 새 필드 완성이 아니다.

## 처리 흐름과 위치

1. `shared/protocol.hpp`: 로그인 요청과 초기정보 version=2. 초기정보/이동/입장/지도에 region 필드, 좌표는 local. 수신 길이부터 확인한 뒤 버전/지역/범위를 검사한다. 지도는18×4chunk가 지역 범위를 벗어나면 거부한다.
2. `session_packets.cpp`: GameObjectState.location()을 packet_position::write로 직렬화한다. 전역(42,10)은 region2/(10,10). 잘못된위치는 패킷을 보내지 않는다. 지도 타일 읽기는 기존 맵을 쓰되 전송 chunk원점은 local이다.
3. `myserver.cpp`: 잘못된로그인버전은 DB조회/생성 전에 실패패킷으로 거부한다. v1길이는 framing검사에서 차단된다.
4. `shared/legacy_packet_adapter.hpp`: 유효한 wire를 내부복사본으로만 변환한다. 일반클라 `ProcessPacket`과 기존GUIstress `ProcessPacket`이 렌더/기존GUI좌표 로직 앞에서 사용한다. 콘솔stress는 공통패킷검증을 사용해 v2위치를 검사한다.
5. DB X/Y와 기존codec는 그대로다. 기존캐릭터 데이터 migration이 없으며 지역상태의 단일원본 전환은 다음2B-2B다. C2S공격 x,y는 서버가읽지않는 기존 비권위필드이며 이 변경의 위치정보로 사용하지 않는다.

기존 CP949 client.cpp/myserver.cpp/NetworkModule.cpp는 byte roundtrip을 확인한 인코딩 보존 패치로 수정했다. 새 헤더/문서는UTF-8.

## 검증 결과

- 전체38회귀 통과. 새 wire검사는 두지역 전체칸왕복·같은local다른지역·invalid지역/음수/범위·v1길이·잘못된버전·chunk경계·원래수신버퍼불변을 검사한다.
- 서버writer는 실제field정보/이동/입장/지도local과 invalid무송신 확인. 기존동시성검사는 과거임의음수/범위밖 좌표 대신 유효한 지역내 x+y=17 불변식으로 유지했다. TCPworker/DB대역은 portal·leave·view·저장과 잘못된버전 DB호출0/같은연결재시도를 검사한다.
- 일반클라 생산코드loopback은 v2입장/필드↔실내이동/잘못된버전실패복귀/잘못된좌표·지도상태불변 통과. 주민·화면크기/F11·월드캐릭터 검사도 통과.
- 기존GUIstress 실제 ProcessPacket 단위검사를 보강하여 region2/local10→legacy42, 이동local11→legacy43, 원래wire버퍼불변, invalid지역수신종료/좌표불변까지 별도 최종재실행 통과.
- 서버·일반클라·기존GUIstress x64 Debug/Release(6개), 콘솔 /W4 빌드 통과. 기존C4244/MSB8028/LNK4098 경고는 남아 있다.
- 실제DB혼합300명15초traceON:300로그인/peak300/전원유지/본인이동응답300/EOF300/저장성공반환300·실패0/클라오류0/서버송신오류0. 총21074ms, drain557ms, 로그인완료큐최대24ms. 측정말미미응답이동차이44는 모든명령응답지연보장과 구분한다.
- 기존900001000~900001299 재사용·저장, 두범위각300 총600보존. 신규계정/삭제/스키마변경 없음. v2로120초·장시간/일반exe실제OS창/과거GUI크래시 재현은 이번 미실행.

미리보기 `build/tests/region-wire-v2-preview.png`는 **생산 맵 렌더러+실제 wire codec/DB codec 테스트 출력**이다. 실제DB접속 게임캡처나 새필드목업이 아니다. 같은local(10,10)이 다른지역에 표시되고 legacy/DB좌표가 유지됨을 생성/육안 확인했다.

## 배포와 되돌리기

서버·일반클라·GUI/콘솔stress를 모두 이커밋으로 함께 빌드한다. 구/신 바이너리 혼용은 지원하지 않는다. 정식solution출력: `TP_2023192003/x64/{Debug,Release}/TP_2023192003.exe`, `TP_2023192003_CLIENT/x64/{Debug,Release}/TP_2023192003_CLIENT.exe`, `stress_test/x64/{Debug,Release}/STRESS_TEST.exe`, 콘솔 `build/tests/bounded_load.exe`. 예전 project직접빌드의 project하위x64 복사본과 혼동하지 않는다. 롤백은 전체바이너리를 동일한 이전커밋으로 빌드·교체하며 DB는그대로 둔다.

다음: **2B-2B canonical 지역위치 상태 전환**. GameObjectState·이동/충돌·AI/전투·섹터·클라카메라의 local소유를 정리하고 DB경계에서만 기존좌표로 변환한다. 이후 #10기존GUI 재현 → 거점/필드 귀환 → 청크필드 → 옷장/커마. #41잔여부하조건은별도열어둔다.
