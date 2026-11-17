# 탐험지 조사 #66

2026-11-18 사전명세. 3큐1/3,기준main baf5513. 기존5ID/지형을쓰고DB/패킷/보상변경없음.

- AC-01: 활성접속의본인확정위치·SunnyPeak/valid/walkable·Manhattan2이내/중간통로에서E/클릭으로조사. 잘못된ID/비활성/다른지역·거리·막힌경로거부.
- AC-02: 장소별짧은설명·좌표·접속중한계·조사완료표시. 조사와기존방문Record를구분. 거점왕복보존/이탈닫기/끊김·새로그인reset.
- AC-03: E반복열닫기방지,Esc/닫기,채팅/주민/지도/옷장/귀환상호차단/입력소비. 월드/네트워크는계속동작. 프로필·채팅과prompt비겹침/letterbox 유지.
- AC-04: 순수규칙+생산입력/패킷·세크기5장소렌더/관련회귀·클라2빌드/독립검토/구조AI·main증거.
- AC-05: 추가사용자요청: 왼쪽하단내프로필을10%축소(초상화/프레임/동적글자함께)하고왼쪽·하단앵커유지. 채팅/몬스터stack은실제축소된프로필top을따른다. 채팅은전체stack위12px,몬스터목록은내프로필위기존8px여백/66px행간을유지한다. 원본이미지/데이터는유지하고세크기렌더검사.

## Direction contract

THESIS: 표지를지나치는것에서멈춰그장소를짧게읽는경험으로확장한다.

OWN-WORLD: 기존cream/brown/neodgm·필드원본/표지,새raster/정체성없음.

STORY: 근접→오른쪽E조사prompt→제목·장소설명·좌표·완료→닫고이동.

FIRST VIEWPORT: prompt395,584/300×38,조사패널395,390/490×230. 왼쪽채팅/프로필은가리지않는다.

FORM: 기존HUD국소확장/code-led. 5개기존지형·명소ID단일원본.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

적용범위: 새DESIGN/이미지생성아니라기존체계보존·구조/AI/검증문서갱신으로완료한다.

## 실행 결과

2026-11-18 구현/검증완료. `verify-field-content.cmd` exit0: 관련5명령(field-content/chat/town-walk-quest/field-map/sprite-frames)·클라Debug/Release2빌드. 순수규칙/생산자기MOVE·E반복/비활성·채팅차단/지도모달/클릭열닫기/지역/끊김·profile변환assert통과. 전체53개는runner등록만했고이번단계미실행,3큐최종통합에서실행한다. 실제DB/전체exe수동OS/부하·FPS없음.

5장소×3크기15PNG+prompt1PNG직접열람. fixture생산renderer출력이며읽기모달뒤배우가가려지는것을원본sprite잘림으로보고하지않는다. 독립 `investigation_finish_review`는16장/코드·명세확인후간격명세불일치(몬스터8px/채팅12px)를지적했다. 명세/구조문서에구분후후속verdict ship/지적resolved,추가UI수정·재캡처불필요. 커밋/원격main/정리·종료는이슈 #66.
