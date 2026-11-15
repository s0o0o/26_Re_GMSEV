# 43단계: 일반 실행파일 시작과 실제 로그인 재시도

부모 [#19](https://github.com/s0o0o/26_Re_GMSEV/issues/19), 수정 [#47](https://github.com/s0o0o/26_Re_GMSEV/issues/47), [명세](specs/native-login-validation.md).

## 기존 문제와 변경 위치

일반 Release exe를 자산 폴더 밖에서 시작하면 상대 경로 `neodgm.ttf`를 찾지 못해 Font Loading Error/exit -1로 종료됐다. 이전 테스트창은 자산 폴더로 이동해서 실행하므로 이 문제를 가렸다. 과거 사용자 10053의 원인과 동일하다고 단정하지 않는다.

`client_asset_paths.hpp`는 exe의 부모와 정해진 저장소/배포 후보를 최대6단계 확인한다. 폰트·메인 이미지·두 캐릭터·두 맵의 최소 표식이 함께 있는 루트를 선택하고 기존 VS 작업 폴더는 마지막 대안으로 둔다. 전체 자산 검사는 기존 로더가 담당한다. `client.cpp::main`은 GetModuleFileNameW와 filesystem 경로로 루트를 정한 뒤 폰트/자산 로더를 시작한다. 없으면 진단 후 종료한다. C++17 filesystem 사용을 위해 vcxproj의 모든 구성에 표준을 명시했다. CP949 client.cpp는 인코딩을 보존했다.

## 실제 실행 증거

computer-use 절차로 **일반 Release exe의 실제 OS 창**에 계정·닉네임·커마를 입력했다. 생산 렌더러 테스트창/설계 목업이 아니다.

1. 승인된 기존 테스트 계정900001299, 닉네임UI, Bijou 선택.
2. `silent_login_fixture.cpp`는 loopback3000에서 58바이트만 받고 응답하지 않는다(DB 없음). 약10초 후 실제 클라에 No login response 안내와 선택 유지 확인.
3. fixture 종료 뒤 실제 DB 서버 시작. **같은 클라 프로세스**에서 Enter 재시도→GameScreen (13,12), Bijou/UI. 서버 connectionId7.
4. Right→(14,12), Escape 종료→EOF/저장 성공. 일반 exe 재실행·같은 계정/외형 선택→(14,12) 복원, connectionId8. 다시 정상 종료 저장.

실제 DB는 승인된 기존 계정의 닉네임/외형/위치를 저장했다. 추가 생성·삭제·스키마 변경 없음. 기존600개 전용 계정 보존. 클라 전체 원시 송수신 로그를 수집한 것은 아니며 fixture 카운터·화면·서버 connectionId/단계 trace를 연결했다.

캡처(로컬 build/tests): native-login-timeout.png / native-login-retry-success.png / native-login-movement.png / native-login-reconnect.png. 서버 native-real-server-trace.log의 connection7/8 모두 login_ready와 disconnect_saved=1, 오류 로그 비어 있음.

## 검증 결과

- 전체 `tests/run-regressions.cmd`: 37개 통과. 추가 자산 경로 검사는 배포 폴더 우선·저장소 탐색·한글 경로·기존 cwd 대안·부분/없는 자산 거부를 검사한다. 포함된300명 부하는 DB 대역 검사이며 실제 DB300명 재실행이 아니다.
- 클라이언트 x64 Debug/Release 최종 MSBuild 통과. 서버 Release는 실제 UI 검사 전 빌드 통과. 기존 C4244/LNK4098/MSB8028 등 경고는 남아 있다.
- 실제 UI의 AC-01~04 통과. 서버 connection7/8 정상 저장 및 실제 OS 캡처 확인.
- `run-login-screens-preview.cmd` / `run-display.cmd` / `run-world-character.cmd` 모두 통과. 이 셋은 생산 렌더러 테스트 출력이며 위 일반 exe 실제 OS 캡처와 구분한다.

한계: 과거10053의 최초 원인 확정, 모든 입력기·기기, 신규계정300/장시간/다른맵 부하는 미검증. #41 추가 부하와 #10 기존 GUI stress 재현은 별도 열린 작업이다. 이번엔 실 UI 기준을 충족한 #19와 시작 경로 수정 #47만 완료 대상으로 한다.

다음: #41 잔여 부하 조건의 계정/시간/맵 범위를 먼저 고정하고, 추가 데이터 변경이 필요한 조건은 승인 범위 안에서 진행한다. 이후 월드 로드맵2B-2 지역 내부 좌표/패킷 전환으로 이어간다.
