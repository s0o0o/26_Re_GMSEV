# 실제 DB 메타데이터와 로그인 계약

2026-11-13, 이슈 [#39](https://github.com/s0o0o/26_Re_GMSEV/issues/39). 프로젝트 DSN의 실제 메타데이터를 읽었다. DSN, 인증 정보, 계정 데이터는 기록하지 않는다.

## 확인한 내용

`get_user_info` 입력은 `@Param1 int` 1개이며, 결과 메타데이터는 아래 순서다. `SERVER_TABLE1`의 같은 이름 컬럼도 모두 SQL Server `int`(4바이트)다.

| 결과 순서 | 이름 | NULL 허용 |
| --- | --- | --- |
| 1 | USER_ID | 아니오 |
| 2 | USER_X | 아니오 |
| 3 | USER_Y | 아니오 |
| 4 | USER_HP | 아니오 |
| 5 | USER_LEVEL | 아니오 |
| 6 | USER_EXP | 아니오 |
| 7 | USER_SEED_NUM | 아니오 |
| 8 | USER_MAX_HP | 예 |
| 9 | USER_DAMAGE | 예 |

현재 연결 권한으로 표시된 기본키/유일 키 컬럼은 0개였다. 유일성이 확인되지 않았다는 뜻이며, 실제 중복 데이터가 있다거나 키가 절대 없다고 단정하지 않는다. 계정 행이나 프로시저 본문은 조회하지 않았다.

## 실행 경계

VS x64 개발자 명령 프롬프트에서:

```bat
cl /nologo /EHsc /std:c++17 tools\inspect-db-contract.cpp /Fo:build\tests\ /Fe:build\tests\inspect-db-contract.exe
powershell -File tools\inspect-db-contract.ps1
```

진단은 ODBC 읽기 전용 연결을 요청하고 로그인/쿼리 timeout을 설정한다. SQL은 `sys.columns`, `sys.parameters`, `sys.indexes`/`sys.index_columns`, `sys.dm_exec_describe_first_result_set_for_object`의 메타데이터 SELECT로 제한한다. 읽기 전용 옵션 자체를 보안 격리라고 간주하지 않는다. 도구는 프로시저 호출, 계정 조회/생성/저장, DDL을 실행하지 않는다.

## 게임 코드의 처리

1. 결과 열 개수·이름·정수 타입을 검사한다.
2. 한 행을 임시로 읽고 길이/NULL/요청 ID/스탯 범위를 검사한다. 기존에도 NULL은 거부했으며 이번에는 검사를 공통 계약으로 묶었다.
3. 추가 행이 없음을 확인한 뒤에만 호출자의 출력 값을 갱신한다. 데이터 오류는 `Error`, 실제 0행만 `NotFound`다. 오류를 신규 계정 생성으로 연결하지 않는다.
4. 위치는 지역 좌표 codec를 거쳐 기존 X,Y로 저장한다. invalid 저장은 거부하고, 기존 invalid 위치 복원은 기존 정책처럼 클럽하우스 시작점으로 복구한다.

스탯은 현재 게임 객체의 SHORT 범위(0~32767), 최대 HP 양수, HP≤최대 HP를 요구한다. 범위를 넘는 데이터를 임의 보정하지 않고 로그인을 거부한다. nullable 스키마가 실제 NULL 행 존재를 의미하지는 않는다.

## 다음 마이그레이션 전 확인

지역 ID+지역 내부 좌표의 직접 상태/패킷/DB 저장은 아직 미구현이다. 새 버전 모델, 기존 좌표 호환/전환 및 rollback을 먼저 명세해야 한다. 유일 키의 실제 정의와 중복 여부는 별도 읽기 전용 감사로 확인하고, 데이터 정리나 제약 추가는 별도 변경 작업으로 다룬다. 이번 다중 행 거부만으로 동시 계정 생성 경쟁이 해결되지는 않는다.
