# 지역 ID와 기존 좌표 호환 경계 (2A)

- 상태: 2A 구현·검증 완료. 사전 명세 기준 81ef299, 2026-11-11.
- 이슈: [#35](https://github.com/s0o0o/26_Re_GMSEV/issues/35). [로드맵](world-display-roadmap.md).

## 현재와 목표

현재 실내/야외는 52×18 전역 좌표의 x<32/x>=32로 추정한다. 섹터 조회, 시야 거리, 공격, Lua 메시지와 전투 fan-out의 지역 조건이 명시적이지 않다. 지역을 추가하거나 간격을 줄일 때 거리만으로 다른 지역 객체가 섞일 수 있다.

이번에는 공통 RegionId(Invalid/Clubhouse/SunnyPeak), 지역 내부 좌표, descriptor와 기존 좌표 왕복 변환을 도입한다. 위치의 단일 원본은 여전히 기존 X,Y이고 region은 공통 변환으로 계산한다. 별도 region 필드와 X,Y가 불일치하는 이중 상태를 만들지 않는다. 범위 밖/두 지역 사이 빈 공간은 Invalid이며 서로 Invalid인 좌표도 같은 지역으로 취급하지 않는다.

## 범위와 제외

- 지역별 시야·주변 섹터 후보 필터, NPC 감지/추격/알림, 공격과 전투 fan-out/enter 패킷을 같은 지역으로 제한.
- 기존 포털을 지역 좌표 기반 변환에 연결. 지역 이탈 시 이전 NPC 추격 generation을 무효화하고 시야에서 제거. 기존 패킷 및 DB 저장 좌표를 그대로 유지하므로 클라/스트레스 바이너리 프로토콜 호환.
- A*는 다른 지역 목적지를 거부. 클라이언트 배경/이름 판정도 공통 지역 정의를 참조.
- 지역 내부 좌표 전송, 별도 DB region 열, 실제 지역별 sector 컨테이너 재구성, 크기 확대, 귀환 버튼/새 마을/개인 집은 **후속 2B/3단계**다. 2A를 전체 지역 마이그레이션 완료라고 보고하지 않는다.
- 미리보기는 기존 생산 렌더러/리소스의 클럽하우스와 필드를 그린 테스트 이미지. 실제 DB 접속 게임 스크린샷이나 새 필드 완성본이 아니다.

## 완료 기준

| ID | 기준 | 검증 계획 |
| --- | --- | --- |
| AC-01 | 유효 지역/좌표 왕복, 같은 local 좌표의 다른 지역 구분, 빈 공간/음수/범위 거부, 기존 포털 왕복 | shared 단위 검사 |
| AC-02 | 거리 내에서도 다른 지역은 시야/섹터 후보/감지/공격/메시지 대상으로 사용하지 않음 | 실제 생산 함수와 C API/worker fixture, 같은 지역 양성 대조 |
| AC-03 | 포털 이탈 시 이전 관찰자 leave, 추격 해제/generation 변경, stale AI 거부 | DB 대역 실제 worker/TCP 전환 검사 |
| AC-04 | 다른 지역 전투/enter fan-out 거부, A* 교차 지역 거부 | 실제 packet writer/fan-out/pathfinder 검사 |
| AC-05 | 서버/클라/스트레스 빌드, 전체 회귀, 지역 미리보기 생성/육안 확인 | 실행 명령/결과 구분 기록 |

공통 world 잠금과 기존 completion→world→combat/Lua 순서를 유지한다. Lua 인수 오류 검사도 RAII 잠금보다 먼저 유지한다. 실제 DB 계정 생성/부하 실행 없이 대역 테스트로 검증한다. 실제 DB 접속/고동접/프로토콜 및 DB 지역 분리는 이번 검증 대상이 아니다.

## 실행 증거 (2026-11-11)

| AC | 구현과 실행 결과 |
| --- | --- |
| AC-01 | shared/world_regions.hpp, world_layout.hpp. world_regions_test: 두 지역 전체 칸 왕복/경계/Invalid/동일 local 좌표의 다른 지역/포털 통과. |
| AC-02 | myserver.cpp, world_regions::inRange. 실제 4-worker fixture의 같은 지역 시야/주변 검색/메시지 양성 대조, Invalid 좌표의 거리 내 시야/섹터 후보 거부, 포털 후 다른 지역 Lua 메시지/알림 차단과 실내 공격 거부 통과. 현재 유효 지역의 전역 간격이 VIEW_RANGE보다 커서 가까운 유효 다른 지역의 시야 상황은 공통 모델에서 검사하고 실제 시야 함수는 Invalid 경계로 검사했다. 같은 지역 전투는 기존 world worker 회귀로 확인. |
| AC-03 | cancelRegionChases와 기존 MOVE 처리. 실제 TCP 포털 응답, 이전 observer LEAVE/목록 제거, NPC generation 증가, stale OP_MONSTER_CHASE 거부 통과. 타이머 producer thread의 장시간 실시간 실행 검증은 아니다. |
| AC-04 | session_packets.cpp/world_events.hpp/pathfinder.cpp. 지역별 enter/전투 fan-out 및 다른 지역 A* 거부/같은 지역 A* 성공을 실제 함수 fixture로 확인. |
| AC-05 | 전체 27개 회귀 통과. 서버/클라/스트레스 x64 Debug/Release 성공, DB 없는 startup-check 성공. 최종 보강한 지역 worker 별도 재실행 및 10회 반복 통과. 기존 생산 렌더러 기반 PNG 생성/육안 확인. |

초기 단위 테스트 include 누락과 이전 loopback fixture의 암묵적 실내 좌표/필드 NPC 기대 실패는 수정 후 재실행했다. 기존 C4244/MSB8028/LNK4098 경고는 남아 있다. 실제 DB 연결/부하/스키마 변경/DB 접속 게임 캡처는 미실행. 패킷/DB 데이터 형식과 물리 섹터 배열은 유지했으므로 2B 전환 완료로 보지 않는다. [사용자 설명과 이미지 재생성](../refactoring-step-33.md).
