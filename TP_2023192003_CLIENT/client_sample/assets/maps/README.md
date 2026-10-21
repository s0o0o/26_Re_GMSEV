# Club House 시작 맵

사용자 첨부 원본을 `clubhouse-sheet.png`로 복사했다. 원본 픽셀과 크레딧은 보존한다.
Hamtaro: Ham-Ham Heartbreak / Club House Ripped by Atlanta.
출처: https://www.spriters-resource.com/game_boy_advance/hamtarohamhamheartbreak/asset/19715/
이미지 및 게임의 권리는 원 권리자에게 있다.

`clubhouse_renderer.hpp`가 (505,323)에서 360×274 영역을 런타임 추출하고,
좌상단 배경색을 투명 처리해 384×288 캔버스에 넣는다.
원본 16픽셀을 게임 좌표 한 칸(화면 50픽셀)으로 표시한다.
지형 규모는 24×18칸이다. 충돌은 `shared/clubhouse_map.hpp`에 별도로 정의한다.
원본 그림만으로 충돌·포털이 자동 생성되는 방식이 아니다.

현재는 아래층 거실만 사용한다. 문/계단은 다음 지역으로 연결되지 않는다.
Sunny Peak, Sunny Cave 및 커마용 첨부 파일은 이번 작업에서 변경하거나 적용하지 않았다.
