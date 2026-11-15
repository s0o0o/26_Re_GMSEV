# 프로젝트 구조 안내 — 여기서 시작

갱신: 2026-11-16, #51. 오래전에 작성한 코드를 다시 읽는 개발자를 위한 **현재 구조** 안내다. 작업별 과거 기록은 [refactoring-step 문서](../refactoring-step-47.md), 다음 계획은 [작업 큐](../work-backlog.md), 변경 전 계약은 [SDD](../sdd.md)로 분리한다.

## 읽는 순서

1. [서버 IOCP와 연결 수명](network-iocp.md): 접속→수신→패킷→게임처리→송신→종료, 함수와 잠금.
2. [월드·좌표·DB](world-and-database.md): 위치의 단일 원본, 지역/지도/포털, 저장 경계.
3. [클라·NPC·Lua](client-and-npc.md): 화면/리소스, NPC 행동 결정과 실제 실행의 구분.
4. [빌드·실행·검증](build-and-test.md): 어떤 프로그램을 빌드하고 어떤 테스트가 실제 DB를 쓰는가.

## 프로그램과 책임

| 구성 | 소스 입구 | 역할/통신 |
| --- | --- | --- |
| 게임 서버 | [myserver.cpp](../../TP_2023192003/TP_2023192003/myserver.cpp) | TCP3000, Windows IOCP worker, 게임 상태/이동/전투/DB |
| 일반 게임 클라 | [client.cpp](../../TP_2023192003_CLIENT/client_sample/client.cpp) | SFML 논블로킹TCP, 입력/패킷반영/렌더링. IOCP 클라가 아님 |
| 기존 GUI 스트레스 | [NetworkModule.cpp](../../stress_test/STRESS_TEST/NetworkModule.cpp) | 여러 테스트 연결의 IOCP 송수신, 내부좌표는 아직legacy adapter |
| 제한형 콘솔 부하 | [bounded_load.cpp](../../stress_test/STRESS_TEST/bounded_load.cpp) | 논블로킹Winsock, 최대300연결/시간제한/로그인·이동·종료 결과 |
| 공유 계약 | [protocol.hpp](../../shared/protocol.hpp) | packed v2 패킷/길이·지역검증. 서버와 모든클라가 같은버전을빌드해야함 |

## 핵심 관계

```text
일반 클라 / 스트레스 도구
    └─ TCP 바이트 → 서버 IOCP worker
                       ├─ processPacket → 월드/전투/시야 → 응답 패킷 → TCP
                       ├─ 동기 ODBC 조회/저장
                       └─ 타이머가 게시한 NPC/월드 작업
```

IOCP는 통신 완료 처리 방식이지 게임 패킷 규칙이나 DB 비동기 엔진이 아니다. 여러worker가 있어도 월드 잠금과 DB mutex가 작업을 직렬화할 수 있다.

## 현재와 미완료를 구분하기

- 구현: v2 region/local 통신, 일반클라·서버의 Position 단일원본, 지역별섹터, 서버판정포털, 주민idle/산책, local카메라.
- 호환경계: DB X/Y, baked map_data/field_content의기존자료, 옛int테스트API. 위치를별도전역필드로중복저장하지않고경계에서변환한다.
- 미완료: GUIstress 내부local전환/과거크래시검증(#10), 귀환UI/큰필드청크/옷장확장/추가지역. DB region열은 도입하지않았다.
- 남은부하: #41 신규생성·다른맵/전투·장시간·종료여유계측. 이전300명단기통과가이조건전체를입증하지않는다.

## 문서 갱신 규칙

구조를 바꾸는 작업은 **코드·사전명세·이 구조 문서**를 같은 작업에서 갱신한다. 함수/모듈 추가, 패킷/좌표 변경, 스레드/잠금/DB 경계 변경, 빌드·테스트 실행 방식 변경이 대상이다. 관련 페이지의 갱신 날짜와 이슈를 바꾸고 실제 소스 링크가 유효한지 확인한다. 과거 실행 기록은 덮어쓰지 않는다.

예: 새 귀환 요청을 추가하면 network 문서에 요청/응답, world 문서에 서버목적지·전환실패, client 문서에 화면상태 초기화, test 문서에 검증경로를 함께 갱신한다. 구현하지 않은 계획은 완료 목록에 쓰지 않는다.
