# 옷장 UI — 기존 커마 확장

모드: Operate. 기존 SFML 픽셀 UI·neodgm·크림/갈색/금색·블러·중앙 정렬을 유지한다. 사용자 승인: 외형 전용 DB 테이블. 신규 시각 세계/아이템 경제/개인 집은 아니다. 기존90조합을 게임 안에서 변경한다.

## Direction contract

THESIS: 미리 입어 보고 저장을 확정한다. 미리보기와 실제 외형이 섞이지 않는다.

OWN-WORLD: 기존 캐릭터/방향별 레이어와 사용자 Accessory Box 아이콘을 사용하고 기존 커마와 같은 패널·글꼴·팔레트를 따른다. 새 raster를 만들지 않는다.

STORY: 공용 거점에서 W/옷장 버튼 → 카테고리/아이템 →4방향 비교 → 적용 또는 취소. 저장 대기/실패/timeout/연결 종료를 문장으로 알려준다.

FIRST VIEWPORT: 900×800 논리 화면 중앙 (115,70,670,660) 패널. 위4방향 미리보기, 가운데4카테고리와 항목, 아래 상태/적용/취소. 거점의 진입 버튼은 귀환 상태 아래 우상단에 둔다. 논리 클릭 좌표와 기존 letterbox를 유지한다.

FORM: 기존 커마의 국소 확장/code-led. 새 comp/개념 추첨은 해당하지 않는다. 적용 전 서버 외형은 바뀌지 않고 저장 중에는 중복 입력을 막는다. 원래 로그인 선택 UI는 유지한다.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

검증: 편집/저장중/완료/거절/timeout/연결 종료·각 카테고리·실제 키/클릭·다중 화면을 생산 renderer/loopback으로 검사하고 독립 검토를 받는다. 기존 체계를 확장하므로 새 DESIGN 파일을 만들지 않는다. 원본 출처/해시는 assets/wardrobe에 보관한다.
