# 지역별 독립 섹터 인덱스 (2B-1)

- 이슈 [#38](https://github.com/s0o0o/26_Re_GMSEV/issues/38), 사전 명세 기준 87884f1, 2026-11-13.
- 상태: 2B-1 구현·검증 완료. [로드맵](world-display-roadmap.md)의 2B 첫 구현.

## 목적과 범위

현재 전역 좌표를 SECTOR_SIZE로 나눈 2차원 배열에 모든 지역 객체를 넣고 검색 후 region을 필터한다. 이를 `(RegionId, localSectorX, localSectorY)`를 키로 하는 희소 인덱스로 바꾸어 같은 local 좌표의 다른 지역도 물리적으로 다른 bucket에 보관한다.

ID→소속 키를 별도 보관해 이동/포털 변경을 한 잠금 안에서 처리하고 ID당 한 소속만 허용한다. 종료는 전체 격자 스캔 없이 해당 소속에서 제거한다. 로그인/몬스터/주민/부활/이동/종료와 worker fixture를 새 경계로 통합한다. 후보 스냅샷을 반환하고 registry 접근/네트워크/Lua를 인덱스 잠금 안에서 실행하지 않는다. 순서는 기존 world→index이며 index 내부에서 world를 잡지 않는다.

전역 X,Y 상태·패킷·DB 형식은 이번에는 유지하고 location()에서 지역 내부 좌표를 받아 인덱싱한다. get_user_info 프로시저의 실제 정의/DDL은 저장소에 없다. 실제 DB 접속이나 임의 스키마 변경은 하지 않으며 **지역 내부 좌표 전송/저장까지 완료했다고 보고하지 않는다.** 후속 2B-2에서 DB 계약/마이그레이션과 버전 구분을 별도 명세한다. 큰 필드/귀환 UI/새 캐릭터도 제외한다.

## 완료 기준

| ID | 기준 | 검증 계획 |
| --- | --- | --- |
| AC-01 | 같은 local 좌표·sector의 두 지역이 다른 bucket/후보로 분리, 지역/섹터 경계 검색·invalid 거부 | 인덱스 단위/서로 다른 region의 동일좌표 양성·음성 대조 |
| AC-02 | 이동/포털/중복 등록/종료의 ID 단일 소속과 빈 bucket 정리 | 단위/동시 업데이트·후보 조회/스냅샷 일관성 |
| AC-03 | 생산 로그인/주민 산책/몬스터/부활/포털/종료의 새 등록 경로와 시야 유지 | 실제 worker/TCP 기존 회귀, 새 index 스냅샷·소속 확인 |
| AC-04 | 서버/클라/스트레스 빌드, 전체 회귀 및 DB 없는 startup-check | 실제 실행/미검증 구분 |
| AC-05 | 실제 인덱스의 지역별 결과를 표시한 기존 맵 렌더 미리보기, 설명/후속 경계 | PNG 생성/육안 확인 |

실제 DB 부하/스키마 변경/프로토콜 버전 전환/장시간 실제 타이머 구동은 이번 검증 범위가 아니다. 인덱스의 단일 mutex 자체는 대규모 성능 개선을 보장하지 않으며 실제 부하는 별도로 측정한다.

## 실행 결과 (2026-11-13)

| AC | 실행/코드 증거 |
| --- | --- |
| AC-01 | region_sector_index_test.cpp: 동일 local (10,10)의 두 지역 bucket/후보 분리, 경계 x19↔20, 범위0/INT_MAX, 잘못된 region/좌표/ID/음수 범위 거부 통과. |
| AC-02 | 중복 put/교차 지역 이동/invalid 갱신 시 이전 소속 보존/중복 remove/빈 bucket 제거 통과. 4 writer 각 3,000회와 동시 snapshot reader, 공용 ID 경쟁 갱신에서 bucket별 중복 없음 확인. 이 단위와 실제 sector worker를 각각 10회 추가 반복했다. Race sanitizer 검사는 아니다. |
| AC-03 | 기존 4-worker 이동·전투/지역 포털/Lua/주민 산책/접속 종료 회귀 통과. 새 실제 TCP fixture에서 실내 local sector 경계 이동, 포털, 동일 local좌표 두 지역 후보 분리, 종료 후 bucket 0 확인. 실제 serviceRespawns에 만료한 NPC를 넣어 이전 실내 bucket→필드 home 재등록과 ENTER 확인을 별도 보강 실행했다. 이전 실내 위치는 테스트용 강제 상태이지 정상 몬스터가 실내에 출현한다는 뜻이 아니다. |
| AC-04 | 전체32개 회귀 통과. 최종 부활 보강 후 지역2개 검사 재실행 통과. 서버/클라/스트레스 x64 Debug/Release 성공, DB 없는 실제 Lua/맵/주민 startup-check 성공. 기존 C4244/MSB8028/LNK4098 경고는 남아 있다. |
| AC-05 | tests/run-region-preview.cmd 실행, 생산 Index와 기존 맵 렌더러의 region-sectors-preview.png 생성·육안 확인. 지역 내부 좌표와 sector가 같아도 다른 지역 후보는 제외됨을 실제 인덱스로 계산해 표시했다. 게임 HUD 변경/접속 캡처가 아니다. |

기존 전역 sector 배열/잠금/크기 상수/직접 접근 참조 제거를 rg로 확인했다. 기존 필드39↔40은 새 local index에서 동일 sector이므로 과거 테스트 주석을 수정하고, 실내19↔20의 실제 경계 fixture를 추가했다. CP949 소스 인코딩 유지, diff check 통과. 실제 DB 접속·DDL/저장 프로시저 검사·프로토콜 전환·실제 고동접 부하는 미실행. [사용자 설명](../refactoring-step-36.md).
