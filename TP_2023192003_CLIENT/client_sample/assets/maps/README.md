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

아래층 거실의 (10,14) 금색 발판은 Sunny Peak로 연결된다. 문/계단은 아직 미연결이다.

`sunny-peak-sheet.png`는 사용자 첨부 원본 보관본이다. Ripped by Jayman357.
원본에서 (921,1,320,288)을 잘라 20×18칸의 야외로 사용한다.
월드 x=32부터 배치해 실내와 구분하며 (42,12) 금색 발판으로 귀환한다.
충돌/출입구/카메라는 `shared/world_layout.hpp`에서 관리한다.
Sunny Cave 및 커마용 첨부 파일은 이번 작업에서 변경하거나 적용하지 않았다.
