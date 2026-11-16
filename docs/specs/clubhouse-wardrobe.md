# 클럽하우스 옷장·커스터마이징 (자율 큐3)

2026-11-16, 기준 main d426613, #55. 상태: DB·옷장 UI 구현 후 최종 검증 중. 큐1 귀환 #53, 큐2 필드 기반 #54는 main 포함 확인·안전 브랜치 삭제·이슈 종료 완료. 아래 과거 기록과 현재 후속 실행을 구분한다.

## 현재 문제와 범위

기존 HamsterCustomizer는 로그인 전 캐릭터2·털색5·눈3·목도리3의90조합을 고른다. 서버는 로그인 요청의 외형을 세션에 담지만 game_database의9열 조회/위치·능력치 저장에는 외형 저장 경로가 없다. 클럽하우스에서 갈아입기·적용/취소·영속 저장·주변 플레이어 변경 통지가 없다.

- 기존 원본/캐릭터 크기·픽셀 세계/블러·중앙 정렬을 보존한다. 캐릭터·털색·눈·목도리 카테고리와 아이템·4방향 착용 미리보기, 공용 거점 내 옷장 조작을 추가한다. 개인 집/새 NPC·대량 몬스터·새 아이템 경제/소유권은 제외한다.
- 제공된 Accessory Box Icons/Dress Up/Accessories as Costumes 원본의 현재 보관 여부를 확인하고 빠진 자료를 보관한다. 모두 즉시 착용 가능하다고 주장하지 않으며 실제 사용/보관용을 명시한다. 기존 방향별 레이어와 실제 게임 표시가 일치하는 항목부터 연결한다.
- draft 선택은 본인 실제 외형을 바꾸지 않는다. 적용은 서버가 지역/값/요청ID를 검증하고 저장 성공 후 확정한다. 취소는 원래 외형을 유지한다. 대기·거절·timeout·연결 종료·귀환/지역 변경을 검사한다.
- 새 요청/결과/외형 변경 통지와 프로토콜 버전 정렬로 본인·현재 관찰자·나중 입장한 플레이어가 같은 외형을 보게 한다. 클라가 타인의 ID/외형을 임의로 지정하지 못하게 한다.

## 열린 결정 — 영속 저장

2026-11-16 사용자 답변: **DB에 별도 외형 테이블 추가 승인**. 이후 “마저해줘”로 계속 진행을 요청했다. 아래 답변 전 기록은 과거 상태다. 이제 dbo.PLAYER_APPEARANCE 신규 테이블(USER_ID PK 및 캐릭터/털색/눈/목도리 값·범위 제약)만 추가한다. 기존 SERVER_TABLE1/프로시저를 ALTER/DROP하거나 계정을 새로 만들지 않는다. 같은 이름의 기존 테이블이 있으면 먼저 계약을 검사하고 불일치 시 중단한다. migration과 production 시작의 schema검사는 분리하며, 서버가 부팅 때마다 DDL을 실행하지 않는다.

ODBC 저장/조회는 기존 DB mutex를 공유하고 world 잠금 밖에서 실행한다. 원자적 parameterized upsert 후 외형 확정, 로그인에서 저장된 외형 우선 복원, 기존 외형 행이 없으면 로그인 선택을 기본으로 사용한다. 읽기 오류는 Missing으로 취급하지 않는다. 실제 검증은 기존 승인 전용계정만 재사용하고 계정 생성·기존 상태 삭제를 하지 않는다.

사용자에게 비동기 질문을 보냈다: **별도 외형 DB 테이블 추가 승인(권장)** 또는 **DB 변경 없이 서버 파일 저장**. 아직 답변 전이다. 현재 위치/능력치 DB 열을 외형 값으로 재사용하거나 로컬 설정만 저장하고 서버 영속화라고 보고하지 않는다.

답변 전에는 옷장 UI·공통 상태/프로토콜·테스트 가능한 저장 인터페이스 경계를 진행할 수 있다. 실제 DDL/프로시저/계정 변경이나 저장 형식 결정을 실행하지 않는다. 영속화 AC는 미완료이며 이 결정이 끝까지 남으면 사용자 방향을 요청하고 전체 목표 완료로 표시하지 않는다.

## 완료 기준

| ID | 완료 증거 |
| --- | --- |
| AC-01 | 옷장 버튼/단축키·카테고리 아이콘/항목·4방향 미리보기·블러/중앙정렬·다중창 크기·키/클릭·draft 적용/취소 검증. 실제 사용/보관 원본과 출처/해시 구분 |
| AC-02 | 서버는 공용Clubhouse/유효 외형/자신의 연결/요청ID만 수락. 중복 재전송·invalid·다른 지역·저장 실패는 변경/통지를 중복 실행하지 않음. 저장 I/O는 WorldStateLock 밖 |
| AC-03 | 성공 후 본인/관찰자 변경 통지, 새 입장 외형, 클라 실제 월드4방향/크기/레이어 정렬 일치. 적용 전 낙관 변경 없음 |
| AC-04 | **승인된 저장 방식**으로 저장/재접속/서버 재시작 복원·기존 계정 기본값·읽기/쓰기 실패·잘못된 자료 검사. 실제 DB 검증과 대역 구분. 저장 선택 전 미완료 |
| AC-05 | 실제 TCP/IOCP4worker+DB대역 및 생산 native loopback/UI/렌더 상태·실패 회귀, 전체 회귀·제품6빌드·미리보기/독립검토·구조 문서 갱신 |
| AC-06 | main 원격 포함 확인·안전 브랜치 삭제·이슈 종료와 세 단계 목표 전체 완료 감사를 수행. 미결 저장/미검증 기준이 있으면 목표 완료로 바꾸지 않음 |

다음 구현 전 확인: HamsterCustomizer의 선택 getter만 있고 setter/스냅샷 API가 없다. 게임 화면과 로그인 커마의 입력 처리가 섞이지 않도록 옷장 draft 상태를 분리한다. 현재 서버 외형은 로그인 요청만 받아 설정되므로 재접속 보존 계약을 먼저 명시해야 한다.

## 진행 기록 — 2026-11-16 (미병합)

- AC-02/03 공통: Value/Journal/ClientState와 v5 요청21/결과22/변경23을 구현했다. 실제 TCP/IOCP4worker 검사에서 미설정/실패/예외·값/거점 제한·재전송·본인/관찰자 변경·새 입장 ENTER를 통과했다. Store.save에서 WorldStateLock 미보유를 assert했고 예외 뒤 이동도 통과했다.
- Store 인터페이스의 production 기본은 **미설정**이다. 실제 DB/파일을 만들지 않고 적용 요청에 StorageUnavailable을 보낸다. 성공 경로는 메모리 대역으로만 검증했다. Store.read/로그인 복원은 미연결이며 AC-04는 미완료다.
- native OBJECT에 확정 외형 메타데이터/텍스처 갱신을 연결했다. 생산 client.cpp 검사에서 draft 비낙관 변경, 본인·주변·늦은 ENTER, 방향 rect 보존, 다른 지역/없는 객체 무시, 지역 전환·연결 종료 검사 통과. build/tests/wardrobe-wire-preview.png는2캐릭터×4방향의 생산 렌더러 텍스처 테스트 출력이며 옷장 UI나 실제 OS/DB 캡처가 아니다.
- 사용자 원본3장 Accessory Box Icons/Dress Up/Accessories as Costumes를 모두 직접 확인하고 assets/wardrobe/source에 변경 없이 복사했다. 원본/복사본 SHA256 일치 확인, SHA256SUMS/README 기록. 카테고리 아이콘은 UI 연결 예정이고 나머지 모든 액세서리가 착용 가능하다는 뜻은 아니다.
- 단위/worker/native 명령: tests/run-wardrobe.cmd, tests/run-wardrobe-client.cmd 실행 통과. 기존 run-travel.cmd도 v5로 실행 통과. 전체 회귀는45개로 확장해 실행 중이며 결과를 아직 통과로 기록하지 않는다. 제품6빌드·옷장 UI 검토·영속 저장·main 반영은 미실행/미완료다. 종료 error64/Accept completion 로그를 오류0으로 확대하지 않는다.
- 다음: 기존 로그인 선택을 변경하지 않는 별도 옷장 draft 패널을 구현한다. 기존 Customizer의 공개 texture/worldTexture를 Value로 조회하면 private 선택 setter 없이도4방향 미리보기를 만들 수 있다. UI 스킬의 기존 픽셀/블러/중앙정렬·상태/다중 화면 검증 절차를 적용한다. 저장 방식 질문은 답변 대기이며 새 DB권한을 임의로 사용하지 않는다.

후속 관찰: tests/run-regressions.cmd가 exit0으로 종료되어 **45개 전체 회귀 통과**를 확인했다(build/tests/wardrobe-regressions.log). 앞의 “실행 중” 기록과 구분한다. production 영속 Store/로그인 복원·옷장 UI·제품6빌드·main 반영은 여전히 미완료다.

## 승인 후 구현·실행 (2026-11-16)

추가 진단 계획(실행 전): 기존 승인 900000000~900000299 재사용의 300명 mixed/15초/drain 실패를 원본 로그로 보존한다. 새 외형 조회 전후 opt-in trace를 추가하고, 이전 main d426613 서버와 같은 버전의 부하 도구를 별도 빌드하여 동일 조건 비교한다. 이 비교는 #41의 모든 잔여 조건 해결을 뜻하지 않으며, 성공 조건을 낮춰 기존 실패를 지우지 않는다. 계정 생성/DDL/삭제는 추가하지 않는다.

### 추가 부하 진단 실행 결과 (2026-11-16)

조건: 실제 DB, 승인된 기존900000000~900000299 정확히300행 사전/사후 확인, mixed/15초/45초 로그인 제한/5초 drain/trace ON/monster3. 코드 빌드가 끝난 뒤 부하를 실행했다. 계정 생성/삭제·새 DDL 없음. 테스트가 만든 서버만 종료하며 전체 graceful server drain 주장은 하지 않는다.

| 실행 로그(build/tests 아래) | 서버 | 결과 | 핵심 관측 |
| --- | --- | --- | --- |
| load-20261116-105008-0e5bb0fa | 옷장 변경본, 외형 시간 계측 전 | 실패 | accepted274, login timeout1, hold미완료, drain timeout299. world wait 최대5446ms. 원본 실패 유지 |
| load-20261116-175607-3a061411 | 이전 main d426613/v4, 별도 빌드 | 통과 | accepted300/hold완료/EOF300/오류0, DB최대16ms, world wait1811ms |
| load-20261116-175651-60ee179c | 옷장 변경본/v5+외형 시간 계측 | 통과 | accepted300/hold완료/EOF300/오류0, 외형 read300건 최대3ms, world wait973ms |
| load-20261116-175752-b718c1a4 | 같은 변경본 반복 | 통과 | accepted300/hold완료/EOF300/오류0, 외형 read300건 최대4ms, world wait1351ms |

`analyze-load-trace.ps1`에 login_appearance_read를 추가했다. 부하 도구는 WSAPoll이며 FD_SET64개 제한 가설은 코드로 제외했다. 변경 전/후가 이번 비교에서 모두 통과하여 최초 실패의 인과는 확정하지 못했다. 외형 조회가 이번 두 통과 실행에서 짧았다는 관측이지 최초 실패 시에도 짧았다는 증명은 아니다. #41은 재현 변동·대기열/스케줄링·장시간 등 잔여 조건을 유지한다. 정상 로그인 회귀를 해결했다고 꾸미지 않으며, 다음 통합 판단에 이 한계를 명시한다.

별도 baseline 전체 archive 추출은 일부 한글 클라 PNG 경로에서 tar 오류를 냈다. 비교에 필요한 서버·공유 계약·부하 소스는 추출되었고 Release 서버 및 동일 v4 부하 클라 빌드는 성공했다. 오류가 없었다고 기록하지 않는다. 실제 실행 서버는 baseline 산출물이며 원래 작업 트리/제품 exe를 이전 버전으로 덮어쓰지 않았다.

- 사용자 DB 테이블 추가 승인 후 appearance-db-admin Inspect로 기존 테이블 없음 확인 → Apply로 dbo.PLAYER_APPEARANCE(5열/USER_ID PK/범위 CHECK) 생성 → Inspect 재검증 통과. 기존 SERVER_TABLE1/프로시저/계정 행의 생성·삭제·DDL 변경은 하지 않았다. 서버는 DDL을 실행하지 않고 initializeDB에서 계약을 검사해 OdbcStore를 설치한다.
- OdbcStore는 parameterized read/upsert, 공통 DB mutex,5초 query/lock timeout, XACT_ABORT/트랜잭션·UPDLOCK/HOLDLOCK과 오류 rollback을 사용한다. SQLINTEGER 범위/요청 계정·NULL/길이/중복 행을 검사하며 narrowing 이전 검증한다. 로그인은 world 밖에서 저장 외형을 읽고 Found 우선/Missing은 기존 선택/Error는 로그인 실패로 구분한다.
- 실제 DB check는 승인된 기존 전용계정 **900001299**만 사용했다. 계정이 정확히1개 있는지 먼저 확인하고, 외형 A={kind1,fur4,eyes2,scarf2}, B={0,1,1,1} 저장/갱신과 각 별도 프로세스의 read 복원을 통과했다. 잘못된 값 저장 거부 후 기존 값 유지도 확인했다. 새 계정은 없고 외형 행1개만 추가/갱신했다. 자동 삭제하지 않았다.
- 실제 production 서버와 생산 client.cpp 기반 검사에서 외형 저장 후 서버를 새로 시작해 재접속/옷장4방향 복원을 통과했다. build/tests/live-wardrobe-20261116-104143의 SaveA/RestoreA 로그·각 disconnect_saved value1이 증거다. 최종 전용 외형은 A다. 검사는 테스트가 생성한 서버만, 저장/EOF 증거 뒤 종료했다. 서버 graceful drain 검증은 아니다.
- 최초 실제 실행 래퍼는 클라가 PASS/서버 저장을 마쳤는데 PowerShell Process.ExitCode가 null이라 실패로 보고했다. 프로세스 Handle을 보유하고 Refresh 후 유효한 종료코드0을 확인하도록 수정해 전체 SaveA/새 서버 RestoreA를 재실행 통과했다. 실패 결과를 성공으로 단순 치환하지 않았다.
- 옷장 버튼/W,4카테고리·아이템·4방향,독립 draft 적용/취소·서버 판정 대기/완료/거절/timeout/종료,기존 창 좌표와 블러·중앙정렬을 연결했다. run-wardrobe-ui.cmd는 실제 SFML 키/클릭·TCP 요청/분할 응답·크기별 클릭 매핑과 상태 렌더 검사를 통과했다.
- UI 연결 중 전역 WardrobePanel의 sf::Texture 초기화로 main 진입 전 access violation이 발생했다. 시작 로그가 나오지 않는 것을 확인하고 함수 정적 객체로 수명을 바꾼 뒤 기존 wire와 새 UI 검사를 재실행 통과했다. 이는 이번 새 코드의 회귀이며 과거 #10 원인 확정이 아니다.
- 1차 시각 검사에서 post-swap 버퍼로 인한 검정 배경과 제목 아이콘의 잘림/회색 프레임을 발견했다. 실제 월드 프레임을 그린 뒤 overlay/swap 전에1회 캡처하도록 옮기고, 원본 전체 아이콘의 배경을 실행 중 분리했다. 2차 출력에서 블러/원본 아이콘/정렬을 확인했다. 새 PNG 자산은 만들지 않았다.
- build/tests/wardrobe-live-db.png·wardrobe-live-restore.png는 실제 DB/production 서버에 연결한 **생산 client.cpp 렌더러 출력**이며 공식 일반exe의 OS 입력 촬영은 아니다. wardrobe-category-0~3.png·wardrobe-states.png·wardrobe-1280x720.png/800x900/1920x1080은 대역/상태 렌더러 검사다.
- 추가된 appearance_row_test와 Store read 오류/예외/invalid Found 로그인 거절·저장 무변경, 메모리 저장 재접속 복원 검사로 전체46회귀를 최종 재실행 중이다. 제품6빌드·독립 UI 검토·현재 단계 main 포함은 아직 결과 확인 전이다.

## 최종 검증 확인 (2026-11-16)

앞선 진행 중 기록의 후속 결과다. `wardrobe-final-regressions-2.log` 전체46개 exit0. `verify-wardrobe-final.cmd` exit0으로 wire/UI/display/login/customizer/travel/field/town8개 GPU/창 검사 통과. 이후 독립 검토의 P3 두 건(저장 중 안내/연결 종료 분류 입력)을 수정하고 `wardrobe-review-ui.log`, `wardrobe-review-wire.log`를 재실행 통과했다. 종료 상태의 Tab/클릭/draft 불변 assertion을 추가했다.

서버/native/GUI의 x64 Debug·Release6개 제품 빌드 모두 성공(`wardrobe-*-final.log`). 빌드 묶음은 마지막 startup-check를 잘못된 루트 cwd에서 실행해 npc.lua 없음으로 exit1이었다. 이를 숨기지 않으며 올바른 서버 소스 cwd에서 같은 Release exe `--startup-check`를 재실행 exit0, 몬스터3/지도2/주민3 초기화 확인했다. 기존 컴파일/링크 경고는 남아 있다.

독립 `wardrobe_finish_review`는 실제 크기/카테고리/상태/DB연결·진입/기존 커마 PNG를 직접 확인하여 ship. P3 수정 뒤 새 states출력/header/assertion/PASS로그를 다시 확인하고 ship 유지. 기존 HUD 이름/SPEED 겹침은 별도 후속. 수동 OS 입력/모든 네트워크 오류 실제 발생 검증으로 확대하지 않는다.

AC-01~05는 정의된 옷장 범위의 코드/회귀/개별 실제DB/렌더·독립검토/문서 증거로 충족했다. 추가 부하의 원인 미확정과 #41 잔여는 위 결과 그대로 유지한다. AC-06 main 원격 포함/브랜치 정리/이슈 종료는 실제 Git 및 #55 종료 댓글에 기록한다. 적용 전후 성능 인과 확정은 완료 기준으로 추가하지 않았다.
