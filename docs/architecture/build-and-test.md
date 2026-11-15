# 빌드·실행·검증 안내

갱신: 2026-11-16, #54. [구조 입구](README.md).

## 개발 환경과 출력

Windows/MSVC x64 개발환경을사용한다. 솔루션을빌드하면아래공식출력경로가생긴다. project직접빌드의project하위x64복사본과혼동하지않는다. SFML2.5.1패키지와Lua헤더/라이브러리는저장소의기존설정을사용한다. 다른기기의VS버전/경로는동일하다고가정하지않는다.

```bat
msbuild TP_2023192003\TP_2023192003.sln /p:Configuration=Release /p:Platform=x64
msbuild TP_2023192003_CLIENT\TP_2023192003_CLIENT.sln /p:Configuration=Release /p:Platform=x64
msbuild stress_test\STRESS_TEST.sln /p:Configuration=Release /p:Platform=x64
```

Debug도Configuration만바꿔빌드한다. 출력은각각`TP_2023192003/x64/Release/TP_2023192003.exe`, `TP_2023192003_CLIENT/x64/Release/TP_2023192003_CLIENT.exe`, `stress_test/x64/Release/STRESS_TEST.exe`다. 일반클라는주변자산루트를찾지만서버는npc.lua등의상대경로가있으므로서버작업폴더를`TP_2023192003/TP_2023192003`로둔다. 서버정상실행은프로젝트DSN/DB가필요하다.

## DB 없는 검사부터

- `tests/run-regressions.cmd`:현재43개프로그램. 청크/충돌·귀환 규칙/실제worker, 단위·loopback+DB대역·모의TCP부하를포함한다. 실제DB300명실행이아니다.
- `tests/run-field-chunks.cmd`:모든 필드 칸·연결성·제한 캐시·수집 상태·경로 예산과 실제TCP/IOCP4worker+DB대역 이동 검사.
- `tests/run-field-preview.cmd`:가시 청크·타일·캐시 예산, 시작/다리/절벽/끝·충돌 진단과 해상도별 생산 렌더러 출력. 실제 OS 입력/DB 화면/FPS 측정이 아니다.
- `tests/run-travel-client.cmd`:생산클라의 버튼/R키·부분응답·실패/timeout·해상도별 렌더러 출력. 실제 OS 입력/DB 캡처와 구분한다.
- `tests/run-login-screens-preview.cmd`:생산클라파서/커마/loopback·이미지. 일반exe OS입력검증과구분한다.
- `tests/run-region-preview.cmd`:생산맵렌더러/지역·카메라·DBcodec출력. 실제DB조회아님.
- `tests/run-town-preview.cmd`:주민이미지/생산클라입장·보간·퇴장. `run-display.cmd`는화면크기/F11, `run-world-character.cmd`는픽셀스케일/외형검사.
- 서버`--startup-check`:DB/네트워크없이몬스터/맵/주민초기화검사. production실행과같은작업폴더에서실행한다.
- `tools/check-architecture-docs.ps1`: 구조 문서의 UTF-8·상대 링크·입구 규칙 검사다. 함수 설명이 코드와 같은지는 별도 독자/코드 검토가 필요하다.

검사는x64 Native Tools Command Prompt에서workspace루트기준으로실행한다. 산출물은Git무시되는`build/tests`에생긴다. 모의검사에고의실패 RESULT가있어도전체프로그램이그거부를기대하고통과할수있으니마지막종료코드/판정을함께읽는다.

## 실제 DB 부하는 별도 데이터 변경

[run-approved-load.ps1](../../tools/run-approved-load.ps1)은기존GUI와다른제한형콘솔도구를사용한다. 사용자의승인이있을때만실행한다. 실제로그인/이동/종료는캐릭터저장을발생시키며COUNT검사는그저읽기다.

승인된기존범위는900000000~900000299와900001000~900001299,각300개다. 새계정생성조건재현에는새범위승인이필요하다. 기존계정을자동삭제하여빈범위를만들지않는다. `check-load-range.ps1`로전후개수/중복을확인한다.

실행기는localhost3000전용/최대300연결/HoldSeconds2~300(기본15)/로그인45초/drain5초다. outer제한은로그인45+유지+여유15초다. 이미listener가있으면중단하며소유프로세스만정리한다. 서버전체graceful shutdown 검증은아니다. mixed는먼저접속한peer가나머지로그인중에도이동하고,separate는전원로그인후이동한다. 조건을섞어성능개선이라고보고하지않는다.

`analyze-load-trace.ps1`는단계대기·EOF·저장성공반환을요약한다. 성공반환은별도의durable DB readback검증과다르다. `render-load-report.ps1`의PNG는실측도표이지게임캡처가아니다. #10은기존GUI도구를직접검사해야하며새콘솔통과만으로닫지않는다.

## 결과를 남길 위치와 유지보수

기존GUI도구는 [GUI 수명 안내](gui-stress.md)의 별도 실행기 `tools/run-approved-gui-load.ps1 -ConfirmAccountWrites`를 쓴다. 콘솔 실행기와 혼동하지 않는다. 30/100/300 각15초·scene/swap/종료코드·pending수거를 확인하며 실제 DB 저장이 발생한다. GUI의 일괄 close 뒤 서버 송신 오류는 종료 전/후를 구분한다. `tools/render-gui-load-report.ps1`은 실측 도표이며 OS 캡처가 아니다.

실행시점/기준commit/명령/계정범위/성공·실패/미실행은`docs/test-results/`와작업설명에기록한다. 구조설명은이폴더를갱신하고과거실행결과는덮어쓰지않는다. 빌드/회귀와성능부하를동시에돌리지않는다. CP949기존소스수정은인코딩을보존하고새문서는UTF-8로쓴다. 실제게임캡처·생산렌더러테스트출력·설계목업을구분하여미리보기를제공한다.
