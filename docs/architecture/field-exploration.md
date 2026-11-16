# 필드 장식·지도·접속 중 탐험

2026-11-17 · #58/#59/#60 · [구조 입구](README.md), [3단계 큐](../specs/field-exploration-queue.md).

## 자료의 단일 원본

- [field_terrain.hpp](../../shared/field_terrain.hpp): 기존 실제 지형·충돌. 이번3단계에서 변경하지 않았다.
- [field_landmarks.hpp](../../shared/field_landmarks.hpp):5개 고정ID·지역내좌표·이름, 결정적 grass 장식. 서버 게임 상태나DB 테이블이 아니다.
- [field_renderer.hpp](../../TP_2023192003_CLIENT/client_sample/field_renderer.hpp): 기존 atlas의꽃/잔디·구역색조·가시표식/그루터기. 장식은충돌을추가하지않는다. DrawStats chunks≤6/tiles≤288/details≤288/landmarks≤5,LRU12.
- [field_map_panel.hpp](../../TP_2023192003_CLIENT/client_sample/field_map_panel.hpp): 실제지형을256×256 texture로첫사용1회생성하고현재위치/거점출구/동일5개표식과범례를표시. 월드전체텍스처를매프레임만드는것이아니다.
- [field_discovery.hpp](../../shared/field_discovery.hpp):접속활성+5비트방문상태. 현재위치가유효SunnyPeak/walkable/표식거리2이내일때만비트를추가한다. 방문은경제/보상/영속저장과무관하다.

## 처리 흐름

생산 client.cpp의검증된본인 AVATAR_INFO → Record.start → moveAvatar → Record.observe 순서다. 이후 본인 ENTER/MOVE에서만 observe한다. 타플레이어이동·클라키입력·단순moveAvatar 호출은기록하지않는다. 중복AVATAR_INFO는같은접속기록을보존한다.

키/마우스 → handleFieldMapInput → 열린지도면소비 → 닫힌경우옷장/귀환/채팅/이동으로진행한다. 지도열림은게임일시정지가아니라입력보호다. 서버수신/NPC/월드갱신은계속된다. M반복은release가있기전재토글하지않으며기존전역F11처리는유지한다. 지도클릭은텔레포트가아니다.

거점이동:지도닫기/기록유지. gameConnectionLost 또는 beginLoginConnection:지도/Record초기화. 로그인한저장위치가표식주변이면새접속의첫방문으로기록할수있다. 지도범례와진행수는같은Record를읽고상태글자/색을함께표시한다. ‘접속 중 기록 / 재접속 시 초기화’ 문구는실제수명계약이다.

## 검증·한계

`tests/run-field-landmarks.cmd`는62640도달·5개고유ID·grass장식조건, `tests/run-field-map.cmd`는생산입력/패킷/세션정리와렌더를검사한다. `field_discovery_test.cpp`는거리/막힘/중복/5비트상한·초기화를검사하며전체회귀에포함한다. 지도/상태PNG는생산렌더러검사이며수동OS/실제DB·부하검사가아니다. 현재최종실행결과와한계는 [탐험 명세](../specs/field-discovery.md)에날짜별로남긴다. #41/#10잔여를이UI작업으로해결했다고보지않는다.
