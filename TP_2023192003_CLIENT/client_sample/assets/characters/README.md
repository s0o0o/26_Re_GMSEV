# 캐릭터 리소스

사용자가 제공한 Hamtaro: Ham-Ham Heartbreak의 Hamtaro·Bijou 시트를 사용한다.
출처: https://www.spriters-resource.com/game_boy_advance/hamtarohamhamheartbreak/
원본 시트 표기: Ripped by Jackster. 게임 캐릭터와 원본 이미지의 권리는 원 권리자에게 있다.

`source/`는 제공받은 시트의 보관본이다. 다운로드 폴더 원본은 수정하지 않았다.
`hamtaro/`, `bijou/`는 게임용 192×48 아틀라스와 눈·목도리 레이어다.
48×48 프레임 순서는 뒤/앞/왼쪽/오른쪽이며, 오른쪽은 왼쪽 정지 포즈를 좌우 반전했다.
이 작업은 정지 포즈 4방향 추출이며 걷기 애니메이션 전체 추출은 아니다.

첫 42행에서 투명하지 않은 영역의 높이를 구한다. 가로 추출 범위(양 끝 포함):

| 캐릭터 | 뒤 | 앞 | 왼쪽 |
|---|---|---|---|
| Hamtaro | 123–153 | 9–39 | 86–116 |
| Bijou | 122–153 | 5–41 | 85–116 |

최근접 보간 약 1.25배, 발바닥 기준 y=46 정렬로 픽셀과 투명도를 유지한다.
Classic eyes / No scarf / 기본 털색 조합은 추출한 원본과 픽셀 단위로 일치한다.
비쥬 색 변경은 밝은 털 일부에만 적용해 파란 리본을 유지한다.

재생성: VS x64 Native Tools에서 저장소 루트의
`tools\rebuild-character-resources.cmd` 실행.
추출 좌표는 `tools/extract_character_sheets.cpp`, 레이어 생성은
`tools/make_hamster_layers.cpp`에서 관리한다.
기존 `assets/appearance/`와 `character.png`는 보존했다.

## 월드와 커마의 표시 크기 분리 (#45)

위1.25배 아틀라스는 커마/로딩용으로 유지한다. 실제 월드에서는 `world_character.hpp`가 합성된 아틀라스에서 원본 픽셀당 대표 샘플을 선택해 별도 native 아틀라스를 만들고, 이를 주민과 같은 정수2배로 그린다. PNG 재작성·보간 블러는 없다. 프레임은48×48/발기준y46이며 본인/타 플레이어에 공통 적용한다. 오른쪽 반전 프레임의 역샘플 위치는 별도로 계산한다.

추출 폭/1.25배 생성 규칙을 바꾸면 `world_character.hpp`와 원본 픽셀 비교 테스트도 함께 갱신한다. `tests/run-world-character.cmd`는 기본8프레임의 alpha/보이는RGB를 원본 시트와 비교하고90외형의 실루엣·nearest 설정·발 위치를 검사한다. 맵50/16배와 전체화면 viewport는 기존 동작을 유지한다.
