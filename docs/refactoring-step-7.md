# 7단계: 로그인 DB 오류와 계정 없음 구분

이슈: https://github.com/s0o0o/26_Re_GMSEV/issues/9

## 이전 문제

get_DB_login_info가 bool을 반환해서 계정이 없을 때도, SQL 실행에 실패할
때도 false였다. 호출자는 false이면 무조건 신규 생성을 시도했다.
또한 Prepare/Execute 실패 경로에서 조회용 statement 핸들을 해제하지 않았다.

## 변경된 흐름

조회 결과를 LoginLookupResult의 Found, NotFound, Error로 구분한다.
SQLFetch의 SQL_NO_DATA만 NotFound로 취급한다. 할당·준비·매개변수 바인딩·
실행·컬럼 바인딩·조회 오류는 Error이며 ODBC 진단을 출력한다.
SQL_SUCCESS_WITH_INFO는 성공으로 인정하고 필수 컬럼의 NULL은 오류로 처리한다.
모든 컬럼이 유효할 때만 출력 인자에 값을 복사한다.

load_or_create_user는 조회/생성 동작을 전달받아 아래 순서로 실행한다.

1. Found이면 기존 캐릭터로 로그인한다.
2. Error이면 생성하지 않고 실패 응답을 보낸다.
3. NotFound이면 한 번 생성하고 재조회한다.
4. 생성 또는 재조회 실패 시 로그인하지 않는다.

조회용 Statement 지역 객체의 소멸자가 SQLFreeHandle을 호출하므로
중간 반환과 정상 반환 모두 핸들을 정리한다.

실패 시 로그인 처리 안에서 disconnect를 호출하지 않고 연결을 유지해
재시도할 수 있게 했다. 이 경로에서 미초기화 캐릭터를 저장하는 호출을
피하고 실패 패킷을 전송 등록한 직후 소켓을 닫는 것도 피한다.

## 테스트와 한계

tests/login_lookup_test.cpp에서 기존 계정, DB 오류, 신규 생성 성공,
생성 실패, 생성 후 조회 오류, 생성 후에도 행이 없는 경우를 검사한다.
각 경우 조회/생성 호출 횟수도 검사하며 실제 서버가 같은 함수를 사용한다.
실제 DB에 쿼리를 실행하거나 장애를 주입한 테스트는 아니다.

서버 x64 Debug/Release 빌드로 ODBC 구현과 호출부 링크를 확인한다.
이전 정수 변환 경고는 남아 있다. 접속/쿼리 실행 검증은 미실시다.

공유 DB 연결 동시 접근과 동일 ID의 동시 신규 생성 경쟁은 아직 남아 있다.
계정 생성 실패를 성공으로 추정해 로그인하지는 않는다. 연결이 나중에
끊겼을 때의 기존 disconnect 저장 정책도 별도 개선이 필요하다.

테스트 실행: x64 Native Tools Command Prompt에서 저장소 루트 기준.
출력 디렉터리는 Debug 서버 빌드로 생성하고 NDEBUG는 정의하지 않는다.

```bat
cl /nologo /EHsc /std:c++17 tests\login_lookup_test.cpp /Fe:TP_2023192003\x64\Debug\login_lookup_test.exe /Fo:TP_2023192003\x64\Debug\
TP_2023192003\x64\Debug\login_lookup_test.exe
```
