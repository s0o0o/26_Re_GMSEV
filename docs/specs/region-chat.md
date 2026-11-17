# 지역 채팅과 프로필 위 자동 배치 #64

2026-11-17 사전명세 작성 후 구현·검증 완료. 기존 C2S/S2C 채팅 패킷(30바이트 message)을 재사용한다. DB/계정/경제·필드활동/주민 추가는 이 이슈 범위 밖이다.

- AC-01: 같은 지역 ingame 접속자만 수신, 서버 sessionId를 발신자로 사용. NUL/UTF-8/제어문자/공백-only/29바이트 제한과 500ms 발신 간격을 서버에서 검사. 패킷 크기/버전 불변.
- AC-02: Tab 또는 왼쪽 입력행 클릭으로 입력, Enter 전송/Esc 취소. Unicode TextEntered→UTF-8, 코드포인트 단위 삭제/바이트 제한, 모달/반복키/포커스/끊김/새로그인 보호. 입력중 이동·단축키 차단. 실제 Windows IME 조합은 수동 검증과 주입 검사 구분.
- AC-03: 왼쪽 최근 기록 최대64건/화면5줄·최근순, 알려진 이름/그외 ID 대체(미등록 object 생성 금지), 3초 월드 말풍선. 과거 대화 DB 저장 없음.
- AC-04: 채팅은 왼쪽 하단, 프로필 예약영역이 있으면 그 top에서12px 위를 하단으로 사용. 내 프로필 P 표시/숨김·몬스터 프로필0~5개 변화 시 약0.18초 이동, 안정후 절대 겹치지 않음. 몬스터 프로필은260×60/66px행으로 내 프로필 위에 쌓는다. 기존900×800 logical view/letterbox 유지. 내 프로필 기본 표시 유지. 전환 중 잠깐 교차할 수 있으며 이는 z-order 대체가 아닌 실제 위치 이동이다.
- AC-05: 순수 검증/생산 클라 입력·수신·TCP 전송/배치 전환·크기별 렌더·서버 경로/기존 관련회귀/제품 빌드, 독립 UI 검토, 구조/AI/이슈·main 반영 증거.

## Direction contract

THESIS: 왼쪽 대화 기록이 내 프로필의 실제 공간을 양보하며 한 덩어리로 이동한다. 레이어 올리기 아님.

OWN-WORLD: 기존 cream/brown/neodgm와 사각 패널·짧은 키 안내, 새 이미지나 시각 정체성 없음.

STORY: Tab 입력→Enter 전송→같은 지역 기록·말풍선 확인. P로 프로필 표시가 달라지면 채팅이 그 위로 이동.

FIRST VIEWPORT: 왼쪽15px/폭360px, 기록5줄+입력행; 프로필 top 기준12px 간격. 29 UTF-8 byte 제한/지역 채널을 드러낸다.

통합 범위 보강: 기존 안내HUD/주민prompt도 왼쪽 채팅 영역을 피하도록 오른쪽 슬롯으로 옮긴다. 지역이 바뀌어도 최근 기록은 같은 접속 동안 유지하고, 끊김/새로그인은 기록을 지운다. 새 필드활동/주민대사확장은 후속이다.

FORM: 기존 native SFML HUD 국소 확장/code-led. 사용자 배치 지시를 직접 적용, concept roll/새 DESIGN 없음.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

## 검증 이력

2026-11-17 기준 origin/main d6576e1 위 feature 작업. 최초52회귀/별도7명령/서버·클라4빌드는exit0이나독립리뷰가UI수정을요청했다. UTF-8BOM없는새CPP의949해석빌드실패,추가fixture의인자/near매크로충돌빌드실패는수정후재실행했다. 고정size MONSTER_NEAR fixture거부/숨긴창검은readback은잘못된증거로판정하고수정·recapture했다. 최종결과는아래후속증거에남긴다.

- AC-01: 실제TCP/IOCP4worker·게임DB대역/외형memory adapter의기존wardrobe fixture에서sessionId강제/region분리/rate/NUL거부PASS. region/rate일부는결정적직접제어다. 실제DB/운영부하아님.
- AC-02/03: `run-chat.cmd` Unicode이벤트/UTF-8·삭제·길이·포커스/모달·실제TCP·미등록sender삽입없음·기록64상한·disconnect검사. 순수규칙은전체회귀52번째에포함. 실제WindowsIME조합·일반exe수동플레이는미검증.
- AC-04/05: free/profile/monster/crowded×세크기12장 + empty/offline/max-input/limit/send-failure/rate/integrated/moving/bubble9장,총21생산컴포넌트fixture장면을생성한다. moving은한전환프레임이며부드러움/FPS계측영상이아니다. 오류상태는동일패널의주입된send결과/시각으로검사한다. 전체client_main/실제DB캡처라고보고하지않는다.

최종 실행: `build/tests/verify-chat-final.cmd` exit0, 전체52회귀/추가7명령(chat,town-walk-quest,field-map,wardrobe-ui,travel-client,display,sprite-frames)/서버·클라x64 Debug/Release4빌드 통과. 로그는 `chat-regressions.log`, `chat-final-*.log`, `chat-build-*.log`. 마지막배우fixture48×48단일프레임초기화만보강후 `run-chat.cmd`와 `run-wardrobe.cmd`도exit0 재확인했다. 제품코드는최종4빌드와동일하다. 기존변환/LNK4098 warning은해결대상으로확대하지않았다.

독립 `chat_finish_review`는21PNG전부재열람·수정코드검토후이전4지적을모두resolved/ship으로판정했다. 판정범위는해당수정과fixture증거이며실제Windows IME/일반exe전체수동플레이/실제DB부하는미실행이다. 이들은다음검증후보다. 구조13문서/115링크·원본NPC21hash·CP949기존소스roundtrip·diff check통과. 최종커밋/main포함·안전정리·종료증거는이슈 #64 종료댓글에연결한다.
