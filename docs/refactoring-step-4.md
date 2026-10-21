# 4단계: 몬스터 길찾기 분리

이슈: https://github.com/s0o0o/26_Re_GMSEV/issues/6

## 기존 문제와 변경

myserver.cpp에 A*의 Node, 우선순위 비교, 좌표 검사, 경로 탐색이 모두
들어 있었다. 이 구현을 pathfinder.cpp로 옮기고 외부에서는 pathfinder.hpp의
findPathAStar(startX, startY, goalX, goalY)만 사용하도록 했다.
Node 등 구현에만 필요한 이름은 익명 namespace로 숨겼다.

기존에는 시작점과 이웃 노드마다 new Node를 호출했지만 delete가 없었다.
이제 vector<unique_ptr<Node>>가 생성된 노드를 소유한다. 탐색 성공, 실패,
예외로 함수를 빠져나갈 때 모두 정리된다. 부모를 가리키는 Node*는 경로를
따라가는 용도이며 소유권은 없다. vector가 커져도 별도로 할당된 Node의
주소는 유지된다.

이미 방문한 좌표는 다시 확장하지 않도록 했다. 이동 비용이 모두 1이고
맨해튼 거리를 쓰는 기존 상하좌우 탐색 규칙을 유지한다. 동률 경로 중
선택되는 경로가 달라질 수는 있다.

시작점/목표가 맵 밖이거나 장애물(타일 3)이면 빈 경로를 반환한다.
따라서 기존의 장애물 시작점에서 출발하던 동작은 허용하지 않는다.

## 흐름

1. 기존 추적/공격 처리가 몬스터와 플레이어 좌표를 전달한다.
2. pathfinder.cpp가 우선순위 큐로 후보를 탐색한다.
3. 도착하면 부모 연결을 따라 좌표를 모은 뒤 순서를 뒤집는다.
4. 시작점부터 목표까지의 좌표 배열을 돌려주고 탐색 노드를 해제한다.
5. myserver.cpp의 기존 호출자가 반환 경로를 사용한다.

전역 map_data를 읽는 구조는 유지했다. 맵 동시 변경 문제, 탐색 시간 제한,
최선 거리 테이블을 통한 후보 수 최적화는 이번 변경에 포함하지 않았다.

## 검증

tests/pathfinder_test.cpp에서 잘못된 좌표, 시작=목표, 장애물 우회 최단 거리,
경로 각 칸의 인접성, 막힌 목표, 도달 불가를 검사한다.
Visual Studio x64 Native Tools Command Prompt에서 루트 기준으로 실행한다.
출력 디렉터리는 서버 Debug 빌드로 먼저 생성한다.

```bat
cl /nologo /EHsc /std:c++17 tests\pathfinder_test.cpp TP_2023192003\TP_2023192003\pathfinder.cpp TP_2023192003\TP_2023192003\world_map.cpp /Fe:TP_2023192003\x64\Debug\pathfinder_test.exe /Fo:TP_2023192003\x64\Debug\
TP_2023192003\x64\Debug\pathfinder_test.exe
```

assert를 실행하려면 NDEBUG를 정의하지 않는다. 실제 게임 추적 동작과
메모리 사용량을 장시간 측정하는 테스트를 대체하지는 않는다.
