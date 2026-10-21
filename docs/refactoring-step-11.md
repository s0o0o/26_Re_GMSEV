# 11단계: 눈·목도리 리소스 제작과 외형 선택

이슈: https://github.com/s0o0o/26_Re_GMSEV/issues/14

## 사용 방법

로그인 화면에서 기존 털 5색에 눈 3종과 목도리 3종을 조합한다.

- 눈: Classic / Round / Sleepy. 클릭 또는 F1/F2/F3.
- 목도리: 없음 / 빨강 / 파랑. 클릭 또는 F4로 순환.
- 털 색상: 색상 칸 클릭 또는 좌/우.
- 미리보기 방향: 위/아래.
- 숫자 ID → 닉네임 → Enter 로그인 흐름은 같다.

총 45가지 조합을 만들 수 있고 서버가 외형 값을 검증해 다른 플레이어에게도
전달한다. 선택값은 접속 중 적용되며 DB 영구 저장과 HUD의 기존 프로필
이미지 변경은 이번 범위가 아니다. 재시작하면 기본 외형부터 고른다.

## 어떤 리소스를 만들었나

실제 게임용 PNG는 client_sample/assets/appearance에 있다.
body-eyeless, eyes-classic, eyes-round, eyes-sleepy, scarf-none, scarf-red,
scarf-blue의 7개이며 각각 투명 배경의 192×48이다. 48×48 칸 4개의 방향은
뒤/앞/왼쪽/오른쪽이다. 원본 character.png는 그대로 보존했다.

이미지 생성 스킬의 기본 내장 도구로 concept-reference.png를 만들고
디자인을 확인했다. 이 시트는 참고용이며 런타임에서 로드하지 않는다.
실제 작은 게임 레이어는 생성 시트의 크기를 억지로 줄이지 않고 기존
캐릭터 눈·목 좌표에 맞춘 tools/make_hamster_layers.cpp로 제작했다.
tools/rebuild-hamster-layers.cmd로 동일한 PNG 7개를 재생성할 수 있다.
직접 편집한 레이어를 재생성하면 덮어쓰므로 먼저 보존해야 한다.

원본에서 눈 영역을 몸체와 기본 눈 레이어로 분리하고, 다른 눈은 같은
위치에 투명 레이어로 그렸다. 뒷모습에는 눈을 그리지 않는다. 목도리는
입 아래에 두고 원본 몸체 밖으로 나가지 않게 잘랐다.

## 코드 흐름

1. HamsterCustomizer가 몸체/눈/목도리 레이어를 로드한다.
2. 몸체 → 눈 → 털 색상 치환 → 목도리 순서로 합성한다.
3. 원본 실루엣의 투명도를 유지하고 45개 텍스처를 캐시한다.
4. 로그인 패킷에 fur_color, eye_style, scarf_style을 담는다.
5. 서버가 각 값의 범위를 검사하고 GameObjectState에 보관한다.
6. 본인 정보와 타인 등장 패킷에 외형을 담아 동일한 합성 텍스처를 표시한다.

패킷 크기가 변경되었으므로 서버·클라이언트·스트레스 도구를 모두 최신으로
빌드해야 한다. 예전 실행 파일과 혼용하면 호환되지 않는다. 실행할 때는
기존 이미지/폰트뿐 아니라 assets/appearance 폴더도 필요하다.

## 검증

- 서버·클라이언트·스트레스 도구 x64 Release 빌드 통과. 기존 경고는 남아 있다.
- tests/run-regressions.cmd의 회귀 프로그램 6개 통과.
- 본인/타인 패킷의 눈·목도리 전달과 잘못된 번호 검사를 추가했다.
- tests/run-customizer-preview.cmd: 기본 조합이 원본 이미지와 정확히 같은지,
  45가지 조합의 크기·투명도가 유지되는지, 선택 키와 범위 밖 값의 기본 처리 검사.
- build/tests/hamster-eyes-scarf.png와 hamster-layer-contact-sheet.png로 실제
  게임 렌더러의 미리보기 및 4방향 레이어 정렬을 시각 확인했다.
- 이 과정에서 발견한 투명 경계 픽셀 문제를 수정했다.
- 테스트 실행기의 음수 실패 종료코드 감지를 보완해 assert 실패가 묻히지 않게 했다.
- 실제 다중 클라이언트 접속, 한글 IME, DB 영구 저장 검증은 미실시다.

## 이미지 생성 기록

도구: 내장 image_gen, 투명 배경. character.png를 스타일 참고로 제공했다.
최종 프롬프트:

```text
Use case: stylized-concept. Asset type: concept reference sheet for a small pixel-art hamster RPG, not a production sprite sheet. Image 1 is a reference for the existing golden hamster identity and cute pixel-art palette, not an edit target. Create one clear transparent-background design sheet containing three front-facing small chubby golden-and-white hamsters in the same style: first has large shiny round eyes and no clothing, second has small bead-round eyes and a simple red knitted scarf, third has sleepy closed crescent eyes and a simple blue knitted scarf. Keep pink ears, white belly, stubby feet. Crisp pixel art, no text, no labels, no scenery, no watermark. Scarf is tiny and does not obscure face or feet. This is a visual design reference; retain the hamster identity.
```
