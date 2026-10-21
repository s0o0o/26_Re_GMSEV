# 3단계: 분리한 패킷 코드의 오류 수정

이슈: https://github.com/s0o0o/26_Re_GMSEV/issues/5

## 무엇이 문제였나?

`sendMovePacket`과 `sendAddPlayerPacket`은 `clients.find(id)`의 결과가
`end()`인지 확인하지 않고 `it->second`를 읽었다. 대상이 없으면 잘못된
메모리 접근이 발생할 수 있었다. 이제 대상이 없거나 포인터가 비어 있으면
알림 전송을 건너뛴다. 모든 동시 접근이나 세션 재사용 문제를 해결한 것은 아니다.

퇴장과 공격 성공 알림은 ID와 인자로 전달받은 체력만 필요했다.
사용하지도 않는 세션 조회를 제거했다. 퇴장한 대상이 저장소에 없어도
수신자의 시야 목록에서 제거하고 퇴장 알림을 보낼 수 있다.
미구현인 몬스터 공격 알림은 여전히 미구현이며 불필요한 조회만 제거했다.

## 맵 패킷의 메모리와 좌표

이전에는 맵 한 화면을 보낼 때마다 `new sc_packet_mapInfo`를 4번 호출하고
해제하지 않았다. 이제 `sc_packet_mapInfo p{}`라는 지역 변수를 사용한다.
`registerSend`가 호출 중에 별도 OVER_EXP 버퍼로 복사하므로 함수가 끝나도
송신 버퍼는 유지된다. OVER_EXP 자체의 완료 처리와 해제 문제는 별도 과제다.

누락된 `left_x`를 설정하고, 시작 좌표를 구할 때 화면 크기를 int로 변환했다.
맵 왼쪽 위에서 필요한 음수 좌표를 unsigned 연산 없이 계산한다.
맵 밖 타일은 기존처럼 8로 보낸다.

주변 몬스터 목록은 개수가 양수인데 포인터가 null이면 전송하지 않는다.
개수가 0이면 memcpy 없이 빈 목록 패킷을 보낸다.

## 테스트

`tests/session_packets_test.cpp`는 실제 소켓 대신 전송 패킷을 메모리에 모은다.
없는 ID, null 세션, 저장소에 없는 대상의 퇴장·공격 알림, null 목록,
맵 (0,0)의 음수 시작 좌표와 경계 타일을 검사한다.

Visual Studio x64 Native Tools Command Prompt에서 저장소 루트 기준:

```bat
cl /nologo /EHsc /std:c++17 tests\session_packets_test.cpp TP_2023192003\TP_2023192003\session_packets.cpp TP_2023192003\TP_2023192003\session_registry.cpp TP_2023192003\TP_2023192003\world_map.cpp /Fe:TP_2023192003\x64\Debug\session_packets_test.exe /Fo:TP_2023192003\x64\Debug\ /link ws2_32.lib
TP_2023192003\x64\Debug\session_packets_test.exe
```

출력 디렉터리는 서버 Debug 빌드로 먼저 생성한다. NDEBUG 옵션은 사용하지
않아야 assert 검사가 실행된다. 실제 네트워크·DB·게임 접속 테스트를 대체하지는 않는다.
