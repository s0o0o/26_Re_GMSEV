# 귀환 UI — 기존 화면 확장

모드: Operate. 사용자 합의: 서버가 정하는 거점/필드 왕복, R키 또는 버튼, 기존 픽셀 세계와 화면 맞춤 보존. 새 화면/시각 세계를 제안하는 작업이 아니다. PRODUCT.md/DESIGN.md는 없으며 현재 client.cpp·ClubhouseRenderer·neodgm.ttf와 기존 렌더링 테스트가 시각 근거다.

## Direction contract

THESIS: 탐험 중 돌아갈 곳을 한 번의 조작으로 찾는다. 모달이나 새 메뉴를 만들지 않는다.

OWN-WORLD: 기존 픽셀 지도, neodgm 글꼴, 크림 바탕·갈색 글자와 금빛 강조를 그대로 확장한다. 상태는 색만이 아니라 문장으로 전달한다.

STORY: 거점에서는 필드로 나가고, 필드에서는 공용 거점으로 돌아온다. 대기 중에는 중복 조작을 막고 실패하면 다시 시도할 방법을 알려준다.

FIRST VIEWPORT: 논리 900×800 화면 우상단 (650,12)에 230×44 버튼. 가운데 지역 이름과 하단 프로필은 유지한다. 상태는 바로 아래 두 줄 이내. 기존 letterbox 화면 좌표 변환으로 클릭을 판정한다.

FORM: 기존 화면의 국소 확장, code-led. 개념 추첨/새 comp는 해당하지 않는다. 기존 생산 렌더러 지역 미리보기를 비교 근거로 사용한다.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

완료 적용 범위: 기존 체계를 확장하므로 DESIGN.md를 새로 만들거나 원본 이미지를 재생성하지 않는다. 생산 렌더러 테스트의 기본/hover/pressed/pending/chat/거절/timeout/연결 종료와 다중 해상도 출력을 확인하고 독립 UI 검토를 받는다. 새 shipping raster는 없다.
