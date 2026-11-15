# 기존 GUI 스트레스 도구 — 실행과 종료

갱신: 2026-11-16, #52. [구조 입구](README.md). 별도 콘솔 실행기가 아니라 기존 OpenGL/IOCP 프로그램의 현재 구조다.

## 소스와 상태

- [DrawModule.cpp](../../stress_test/STRESS_TEST/DrawModule.cpp): WinMain에서 OpenGL 창을 만들고 메시지/그리기 루프를 실행한다. NetworkRunFinished()가 되거나 사용자가 종료하면 ShutdownNetwork()를 먼저 호출하고 창을 해제한다. 실패는 종료코드2, 성공은0이다. main도 WinMain 결과를 전달한다.
- [NetworkModule.cpp](../../stress_test/STRESS_TEST/NetworkModule.cpp): 연결 생성·6개 IOCP worker·수신조립/검증·이동 producer·상태 스냅샷·종료를 담당한다.
- CLIENT.position은region/local이다. legacy packet adapter를 사용하지 않는다. 각 연결은 본인의 MOVE만 반영하며, 다른 연결에서 온 동일 객체 방송으로 위치를 덮어쓰지 않는다.
- stateMutex는 ID 매핑·위치·소켓 제출/닫기·화면 스냅샷의 공유 접근을 직렬화한다. 이것은 안정성 우선의 공통 잠금이지 최대 처리량을 보장하는 설계가 아니다.

## 안전한 실행 범위

[gui_load_options.hpp](../../stress_test/STRESS_TEST/gui_load_options.hpp)는 `GMSEV_GUI_CONFIRM=1`, `GMSEV_GUI_ACCOUNT_START=900000000` 또는 `900001000`, 시도1~300을 확인한다. 없거나 범위가 틀리면 접속하지 않고 실패 종료한다. 예전처럼 계정0부터 접속하지 않는다. 기존 attempts 파서의 기본은30이며 GUI 승인 검사가 추가 상한300을 적용한다.

권장 실행기는 [run-approved-gui-load.ps1](../../tools/run-approved-gui-load.ps1)이다. 사용자 승인 후 `-ConfirmAccountWrites`로 실행하며, 두 전용 범위 COUNT300을 읽고 30→100→300을 순서대로 실행한다. 각 단계는 새 서버/GUI 프로세스이고 전원 접속 뒤15초를 유지한다. GUI 전체 제한은60초이므로 성공하려면45초 안에 전원 접속해야 한다. 외부 GUI 프로세스 제한은90초다.

이 실행기는 실제 DB 캐릭터 저장을 발생시킨다. 새 계정 범위를 임의로 추가하거나 기존 계정을 삭제하지 않는다. 이미3000번 listener가 있으면 중단하고, 본 작업이 띄운 프로세스만 정리한다.

## 판정과 수신

초기 AVATAR_INFO는 위치/ID 준비일 뿐이다. 일치하는 LOGIN_OK를 받은 뒤에만 connected/accepted를 올린다. 중복 세션 ID 매핑과 잘못된 패킷은 거부한다. 본인 이동의 nonzero token 응답을 한 번 이상 확인한 연결 수를 moved로 센다. 모든 이동 명령 응답 지연을 측정하는 것은 아니다.

Test_Thread는 접속과 주기 이동을 수행하고, 목표 인원 유지·accepted·moved·거절/오류 조건을 판정한다. GUI는 검사 중 비활성 창이어도 DrawGLScene을 수행한다. 로그의 WINDOW_CREATED/SCENE_DRAWN/FRAME_RENDERED는 창·그리기 호출·버퍼 교환을 구별한다. 이는 실제 OS 스크린샷이나 마우스/키보드 사용 검증과 다르다.

## 종료 순서

```text
running=false → producer join → 모든 소켓 닫기
→ pendingIo 완료/취소 수거(최대10초)
→ null completion으로 worker 깨우기 → worker join
→ 정상 drain이면 IOCP/WSA 해제 → GUI_RESULT/종료코드
```

pendingIo는 WSARecv/WSASend 제출 전에 증가하고, 즉시 실패 또는 IOCP 완료 수거에서 감소한다. RECV 버퍼는 고정 CLIENT 슬롯, SEND 버퍼는 완료까지 살아 있는 heap 객체다. 전역 중지 뒤 의도한 취소 완료는 유지 중 전송 오류와 구분한다.

10초 안에 수거되지 않으면 성공으로 처리하지 않는다. worker를 멈춘 뒤 미완료 버퍼/핸들을 일찍 해제하지 않고 실패 종료로 프로세스 수명에 맡긴다. 이 실패 경로는 정상 자원 정리 보증이 아니며, 강제 timeout 주입은 별도 검증이다.

여기서 drain은 **GUI의 pending I/O 수거**다. TCP를 half-close하고 서버 EOF까지 읽는 콘솔 도구의 drain과 다르다. 일괄 close 뒤 서버가 이미 보내던 데이터의 송신 오류가 남을 수 있다. GUI 오류0과 서버 오류0을 혼동하지 말고 GUI_SHUTDOWN 시각 전후로 구분한다.

## 결과와 한계

`GUI_RESULT`는 attempts/accepted/moved/closed/errors/pending/drained와 최종판정을 남긴다. 단계별 종료코드와 서버 생존, 실제 scene/swap 로그를 실행기의results.json에 남긴다. [render-gui-load-report.ps1](../../tools/render-gui-load-report.ps1)은 이 측정값과 서버 trace에서 도표를 만들며 OS 캡처를 만들지는 않는다.

현재 조건에서 과거 크래시가 미재현되어도 당시 원인·덤프가 없는 이상 동일 원인 해결을 확정하지 않는다. 짧은 단계 검사는 장시간·모든 오류 주입·GUI 사용자 조작·최대 수용량을 보증하지 않는다. 점군의 지역별 패널/색상 구분 등 UI 개선도 별도다.
