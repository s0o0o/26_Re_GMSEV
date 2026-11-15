# 넓은 필드 표시 — 기존 게임 확장

모드: Experience, 게임 내 탐험. 사용자 제공 Sunny Peak 픽셀 아트와 기존 귀환/지역 HUD를 유지한다. 새 시각 세계나 UI 재설계가 아니다. 원본 시트와 기존 생산 렌더러 미리보기가 근거다. PRODUCT/DESIGN은 새로 만들지 않는다.

## Direction contract

THESIS: 시작 화면을 떠나 실제로 이어지는 지형을 걷는다. 배경 한 장을 늘리거나 빈 공간에 놓지 않는다.

OWN-WORLD: 제공된 Sunny Peak 시트의 풀·물·바위·목재·흙을 원본 픽셀/nearest 필터로 재사용한다. 크레딧·분리 배경색은 사용하지 않는다.

STORY: 시작 구역 동쪽으로 나가 초원을 지나 강의 다리를 건너고 절벽의 통로를 찾는다. 귀환 버튼으로 공용 거점에 돌아온다.

FIRST VIEWPORT: 기존 900×800 카메라와 지역/귀환 HUD 유지. 시작20×18은 동쪽 연결부 외 원본 그대로, 확장부는 shared 지형과 같은 청크에서 보이는18×16만 배치한다. 캐릭터 크기는 유지한다.

FORM: 국소 확장/code-led. 컨셉 추첨·새 comp는 해당하지 않는다. 지형·충돌 일치와 연결성, 가시 청크 예산, 시작/다리/절벽/먼 모서리/다른 해상도 생산 렌더러 출력을 검토한다.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

적용 범위: 새 shipping raster는 없다. 원본 시트를 코드에서 참조하며 새로운 디자인 시스템 파일을 만들지 않는다. 미리보기는 실제 OS 입력/DB 캡처가 아닌 렌더러 테스트 출력으로 표시한다.
