# 햄스터 외형 리소스

모든 게임용 PNG는 192×48, 투명 배경이며 48×48 칸 4개다.
방향 순서는 기존 character.png와 같은 뒤 / 앞 / 왼쪽 / 오른쪽이다.

| 파일 | 역할 |
| --- | --- |
| body-eyeless.png | 눈 영역을 비운 기본 몸체 |
| eyes-classic.png | 원본에서 분리한 기본 눈 |
| eyes-round.png | 작은 동그란 눈 |
| eyes-sleepy.png | 졸린 눈 |
| scarf-none.png | 완전히 투명한 선택 없음 레이어 |
| scarf-red.png | 빨간 목도리 |
| scarf-blue.png | 파란 목도리 |
| concept-reference.png | 이미지 생성 도구로 제작한 디자인 참고 시트. 게임에서 로드하지 않음 |

원본 character.png는 수정하지 않았다. 눈·목도리의 픽셀 좌표와 색상은
tools/make_hamster_layers.cpp에서 관리하며 tools/rebuild-hamster-layers.cmd로
PNG 7개를 다시 만들 수 있다. 실행은 x64 Native Tools Command Prompt에서 한다.
재생성은 위 7개 PNG를 덮어쓰므로 직접 픽셀 편집한 파일은 먼저 보존해야 한다.

합성 순서는 몸체 → 눈 → 털 색상 치환 → 목도리다. 털 색상 캐시는 실행 중
45가지 조합으로 생성한다. 눈이 없는 뒷모습에서는 눈 레이어가 투명하다.
목도리는 원본 몸체 바깥으로 나가지 않도록 잘라 낸다.

AI 시트는 컨셉 참고용이며 프로덕션 스프라이트로 샘플링하지 않았다.
실제 게임 레이어는 원본 팔레트와 좌표에 맞춘 재현 가능한 픽셀 소스로 제작했다.
시트 생성 프롬프트는 docs/refactoring-step-11.md에 기록한다.
