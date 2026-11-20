# Pashmina 절벽·그루터기 의뢰 #72

2026-11-21 사전명세. 기준 main8d077fc. 기존 주민101의 대화/Q를 확장하며 기존 Boss 의뢰를 보존한다. 새 NPC/행동정책·DB·계정·패킷·보상/영구저장 없음. 기존 원본·idle/산책을 유지한다.

- AC-01: 활성 접속/유효 Pashmina 근접 대화에서 독립 수락/보고. 절벽3 inspected + 그루터기4 completed 두 조건, 방문만으로 불가. 수락전 같은접속 기록 인정/보고별도·중복거부. 기존 Boss 의뢰는 변하지 않는다.
- AC-02: 기존 크림 대화의 Pashmina 초상화·Q 상태/본문/접속한계·보상없음, J 수첩 목표행/다음 행동·HUD에 연결. 동시 활성 의뢰는 Boss 먼저, Boss 완료 후 Pashmina, 둘 다 없으면 기존 일반 안내. 의뢰가 Ready면 해당 주민에게 보고 안내한다.
- AC-03: press-release/모달·live NPC ID/type/지역·거리/LEAVE 보호와 거점 왕복 유지, 끊김·새로그인 초기화. 상태 정의의 장소/주민 ID는 reset 후에도 보존. 수첩 읽기로 완료 불가.
- AC-04: 규칙/우선순위·생산 E/F/Q·클릭·반복/동시의뢰/잘못된NPC·이탈·초기화·읽기불변,4단계×3크기 대화12/활성수첩3/HUD3 렌더. 전체회귀/관련검사/클라2빌드·독립검토·현재구조AI·원격종료. 실제DB/수동OS·장기성능 미검증 구분.

## Direction contract

THESIS: 같은 필드를 주민마다 다른 시선으로 살피는 작은 이야기. 새 창 없이 기존 대화/Q와 수첩 다음 행동을 확장한다.

OWN-WORLD: cream/brown/neodgm·Pashmina 원본 idle 초상화/기존 대화 위치와 J 표,90% 프로필·채팅 보존.

STORY: Pashmina E/Q → 절벽3 E 조사 → 그루터기4 E/F → R귀환/Pashmina E/Q보고.

FIRST VIEWPORT: 기존100,440/700×220 대화 본문 두줄·Q 버튼, 수첩/하단 HUD 기존 슬롯에 다음 행동 표시. 별도 의뢰창/레이아웃 교체 없음.

FORM: native SFML Operate 국소확장/code-led. 기존 체계 유지·새래스터/DESIGN 변경 없음.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

이번 범위는 기존 체계 보존·구조/AI 문서 갱신이며 새 DESIGN 작성이 아니다.

## 검증 연결

AC-01/03: pashmina_experience_test의 별도3/4조건·수락전기록/중복/정의보존 reset, 생산 UI 테스트의 E/F/Q·버튼 외부release·다른NPC/LEAVE 보고거부·왕복/끊김·새로그인·씨앗불변. 새 규칙/생산 입력 검사 단독 exit0 확인.

AC-02/04: 순수 preferred 검사에서 동시활성 Boss 우선/Boss 보고 뒤 Pashmina 전환. 생산 J에서 Pashmina 목표3 선택/읽기불변과 대화4단계×3크기12장·수첩3/HUD3 총18장을 직접 각각 열람했다. 파일은 build/tests/pashmina-dialogue-{0,1,2,3}-{900x800,1280x720,800x900}.png 및 pashmina-{journal,hud}-1-{같은3크기}.png다. 지도+패널/원본 idle초상화의 생산렌더러 fixture이며 월드 actor·채팅/프로필을 모두 그린 게임 OS 캡처가 아니다. tick 완료 시각은 주입했다.

독립 pashmina_finish_review는18장 각각 열람/명세·코드/테스트 읽기 후 **ship**, 필수수정/선택제안 없음. 기존 체계 보존/변경 범위에 한한 판정이다. 전체58/관련13/클라2빌드 최종 결과와 원격 포함·이슈 종료는 확인 뒤 추가한다. 실제DB/수동OS/장기성능/서버·스트레스 제품 빌드는 미실행이며 기존 빌드 경고도 유지한다.

최종 `tests/run-content-queue-checks.cmd` 실제 내부/바깥 VERIFY_EXIT=0: 전체58회귀·관련13명령·일반클라x64 Debug/Release2빌드 통과. 로그 build/tests/resident-queue-*.log. NPC21 SHA256·CP949roundtrip·구조14문서139링크·diffcheck 통과. 실제 DB 실행기/새서버 권한은 사용하지 않았다. 원격 포함/구현·병합커밋·안전정리·종료 근거는 이슈72 최종 댓글에 기록한다. 다음 후보는 접속중 주민 의뢰 목록이며 별도 명세부터 진행한다.
