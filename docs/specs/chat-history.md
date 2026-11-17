# 긴 채팅·기록 탐색 #65

2026-11-18 사전명세 작성 후 구현·검증 완료. 기준 main c29f135. #64의 지역/rate/프로필 공간 추적 유지. DB/계정/실제DB부하/새채널 없음.

- AC-01: v6 message[120]/119 UTF-8 bytes(한글 최대39음절)로 확대. uint8 packet size255 이하/BUF_SIZE300 유지. 서버·일반클라·스트레스 도구 함께 빌드, 이전v5 로그인은거부. UTF-8 경계/잘못된문자/지역/sessionId/500ms 검증 유지.
- AC-02: 기록64메시지/화면5행은 유지하되 긴 메시지는42표시칸(한글2칸/ASCII1칸)으로 줄바꿈. 이름+본문 함께 줄바꿈, 입력은끝부분 표시해 글자를축소하지않고119byte까지확인/코드포인트삭제 가능.
- AC-03: 채팅영역휠·PageUp/PageDown으로행기준이전기록,최신버튼/End로복귀. 스크롤중수신은보고있던내용보존/새메시지수표시,64개제거경계에서유효범위clamp. 모달/끊김/새로그인reset/프로필이동·해상도 유지.
- AC-04: Windows Computer Use로 DB없는생산ChatPanel 실행창의실제키입력/스크롤을검사한다. type_text Unicode·확정TextEntered 주입과 실제한국어IME조합은별개다. 도구/환경불가시실패·미검증을기록하고사용자도움이필요한검증을완료라고쓰지않는다.
- AC-05: 순수규칙/생산클라TCP·긴수신/scrollanchor/rate·UI/다중크기렌더·worker DB대역·전체회귀·제품6빌드·독립리뷰/구조/AI/이슈/main절차.

## Direction contract

THESIS: 말이 길어져도 글자를 줄이지 않고 읽고, 지난 대화를 읽는 동안 새 대화가 화면을 밀어내지 않는다.

OWN-WORLD: 기존cream/brown/neodgm·왼쪽360×174패널/프로필12px간격 유지. 새이미지/정체성/DESIGN 변경 없음.

STORY: Tab→긴한글입력/Enter→자동줄바꿈,휠/PageUp으로이전내용→새메시지표시→최신/End복귀.

FIRST VIEWPORT: 기존5행기록+입력행 그대로. 스크롤중헤더오른쪽에최신버튼·새메시지수. 입력은끝부분을보여주고실제byte제한표시.

FORM: 좁은기존UI확장/code-led,사용자가정한프로필배치를보존한다.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

적용 범위: 위 Skill 공통종료문구의 DESIGN.md는 이번 기존UI국소확장에서 새로 만들거나 바꾸는 산출물이 아니다. 기존 시각체계를 보존하고 독립검토·구조/AI/검증 기록을 갱신하는 것으로 문서 게이트를 충족한다. 새 shipping raster도 생성하지 않는다.

## 2026-11-18 실행 증거

- AC-01: UTF-8 ASCII119/한글39음절117+ASCII2=119/120byte거부와유효/잘못된scalar 검사. 실제TCP/4 IOCPworker·DB대역의기존wardrobe fixture에서119byte채팅의1byte분할송신복원·같은지역수신/sessionId/rate/NUL검사통과. v5로그인실제거부검사. 실제DB아님.
- AC-02/03: `tests/run-chat.cmd` 안의SFML문자열history검사와생산client.cpp fixture에서wrap/수신anchor/64개퇴출/clamp/unread/End·최신클릭·휠/PageUp5행·PageDown복귀/모달·reset/한글삭제·바이트한계/S2C119byte/실제TCP전송검사통과. 긴한글fixture가최초119byte를넘어assert실패했고범위내문장으로수정후재실행통과했다.
- AC-04: Computer Use skill/sky로DB없는입력probe를실행했다. 입력경합보호와Hangul키미지원으로자동조합재현은미검증. OS확정한글이벤트/로컬echo를관찰했고, 사용자가검증창에직접입력하여의도한글자가잘나왔다고수동확인했다. 기본입력probe확인으로한정하며일반게임exe·모든IME조합/받침취소/포커스/F11 전환검증으로확대하지않는다. probe는로컬echo만하고10분자동종료한다.
- AC-05: `build/tests/verify-history-final.cmd` exit0. 전체52회귀/별도7명령(chat,town-walk-quest,field-map,wardrobe-ui,travel-client,display,sprite-frames)/서버·클라·stress GUI x64 Debug/Release6제품빌드통과. `history-regressions.log`, `history-final-*.log`, `history-build-*.log`. 이후테스트만보강하고 `verify-history.cmd`(history+run-chat) exit0재확인. SFML history검사는별도창검사명령에포함되며전체52개에중복계수하지않는다. 변환/LNK4098등warning은잔존한다.
- 최신/이전+새메시지/최신복귀×3크기9장과max-input/limit/integrated3장,총12PNG를생성·직접열람했다. `chat-state-long-{latest,scrolled,returned}-{900x800,1280x720,800x900}.png`. 생산renderer fixture이며실제게임수동OS/DB장면아님.
- 독립 `history_finish_review`: UI·scroll필수수정없음,문서FINISH적용범위혼동을지적해범위주석으로해소. 비차단PageUp/Down직접assert제안채택. 최종verdict **ship**,문서지적resolved/추가수정없음. 검토자는테스트를재실행하지않았으며12PNG/코드·명세검토범위다.
- 현재구조/AI/백로그동시갱신. 원격main포함·안전branch-d·커밋/이슈종료증거는 #65 종료댓글에연결한다. 다음은필드탐험지상호작용콘텐츠이며전체게임IME조합의추가수동검증은별도한계로유지한다.
