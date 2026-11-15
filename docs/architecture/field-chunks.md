# 넓은 필드와 청크·충돌 (#54)

2026-11-16. [명세·실행 결과](../specs/field-chunks.md), [월드·DB 입구](world-and-database.md).

## 자료와 위치

Clubhouse는 24×18 하나, SunnyPeak는 256×256이다. [field_terrain.hpp](../../shared/field_terrain.hpp)가 풀/흙길/물/절벽/다리를 결정하며 walkable도 같은 자료에서 나온다. 시작20×18은 동쪽 연결 통로 외 기존 충돌과 아트를 보존한다. 고정된 지형이므로 매 실행이나 캐시 퇴거로 바뀌지 않는다. 62,640개 통행 칸이 시작점과 연결되어 있는지 BFS로 검사한다.

지역 범위 계약이 달라져 공유 프로토콜을 v4로 올렸다. 패킷 배치는 v3과 같아도 옛 v3 클라를 로그인에서 거절한다. DB legacy X=32+localX, Y=localY는 유지한다. 실제 DB 열/계정/프로시저는 바꾸지 않았다. camera(Position)은 지역별 크기로 끝을 clamp한다. 옛 Y만 받는 cameraY(int)는 지역을 구분할 수 없어 과거 시작 화면 테스트 전용이다.

## 서버 조회와 상태

[field_chunks.hpp](../../shared/field_chunks.hpp)는 16×16 불변 지형과 제한 LRU다. world_map.cpp의 캐시는 최대16청크이며 mapMutex로 보호한다. 클럽하우스는 기존 충돌 함수로 조회한다. 전체 월드 map_data 배열은 제거했다. 옛 int 조회 API는 위치를 decode하여 canonical Position API를 호출한다.

씨앗의 deadline은 지형 LRU 밖의 고정 배열이다. 청크가 퇴거되었다 다시 생성되어도 이미 수집한 씨앗이 복원되지 않는다. restoreSeeds가 기한을 판정해 기존 자료를 다시 활성화한다. 잠금 순서는 world→map이며 맵 조회에서 세션/world/DB를 역으로 잡지 않는다.

지도 응답은 여전히18×4타일 조각4개다. 네트워크 조각 크기와 지형 캐시16×16은 별개다. 클라가 좌표/충돌을 결정하거나 전체 청크를 서버에 생성 요청하는 구조는 아니다.

## 클라 가시 범위

[FieldRenderer](../../TP_2023192003_CLIENT/client_sample/field_renderer.hpp)는 기존 Sunny Peak 시트의 픽셀을 참조한다. 시작 화면을 그리고, 확장 구역의 가시 청크만 batched quad로 그린다. 한 화면에 최대6청크·288타일, 메타데이터 캐시는12청크다. 월드 크기의 텍스처를 생성하지 않는다. 목재 다리 난간은 단순 경계 geometry이며 새 raster가 아니다.

client.cpp는 onScreen(Position,region,camera,18,16,margin=1)으로 다른 지역/화면 밖 객체 draw를 제외한다. 한 칸 여유는 경계에 걸친 캐릭터가 갑자기 사라지지 않게 한다. 서버의 region-sector 시야 후보와 VIEW_RANGE=7은 유지하며, AI는 기존 관찰자 기반 활성/수면 정책을 사용한다. 새 NPC/대량 몬스터 배치는 추가하지 않았다.

## 경로 예산과 검증

findPathAStar(Position,Position,maxExpanded=1024,stats)는 최대 확장 노드 예산을 갖는다. 예산에 도달하면 빈 경로/통계를 반환하며 worker가 먼 필드 전체를 무제한 탐색하지 않는다. 이는 전역 장거리 길찾기의 완성 기능이 아니라 서버 AI의 유한 작업 경계다.

run-field-chunks.cmd: 모든 칸 지형/충돌·BFS·LRU·씨앗 퇴거/리스폰·DB codec·탐색 예산과 실제 TCP/IOCP4worker의 청크 경계/다리/물/끝/활성/수면/귀환을 검사한다. DB는 대역이다.

run-field-preview.cmd: 시작/다리/절벽/먼 모서리·충돌 진단·두 해상도와 전역 카메라 샘플의 가시 작업 상한을 검사한다. 생산 렌더러 출력이며 실제 OS/DB 게임 캡처나 FPS 측정은 아니다. 전체 빌드/회귀와 UI 검토 결과는 명세에 기록한다. 이 단계는 지형 기반이며 완성된 콘텐츠 밀도/세밀한 레벨 아트/새 NPC/실제 DB 장시간 부하를 의미하지 않는다.
