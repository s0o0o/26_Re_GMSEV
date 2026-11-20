# 주민 의뢰 목록 #73

2026-11-21 사전명세, 기준mainf1f4d79. 여러 의뢰의 상태를 한눈에 보기 위해 J수첩 안의 읽기전용탭을 추가한다. 새 창/패킷/서버/DB/수락·보고 기능은 없다.

- AC-01: Boss방문/Boss체험/Pashmina체험3행의 잠금/수락가능/진행/보고준비/보고완료·진행량/상세를 기존 상태에서 요약. Boss체험은 방문의뢰 보고 전 잠금. 같은접속 기록 의미 유지.
- AC-02: J열기/Tab·버튼 탭 전환/상하·행 선택/외부release·키반복·Esc 보호, 열람으로 수락/보고/조사/체험 변경 불가. 기존 탐험탭·왼쪽채팅/프로필 위치 보존.
- AC-03: 거점왕복 유지/끊김·새로그인 탭/선택 초기화, 기존모달 배타/키해제 전체전달 유지.
- AC-04: 순수요약·생산 router/읽기불변·3상태×3크기9렌더·관련검사/클라2빌드·독립검토·구조AI·원격반영. 실제DB/수동OS 미검증 구분.

## Direction contract

THESIS: 수첩 안에서 장소 기록과 주민의 부탁을 나눠 읽는다. 수락 버튼을 늘리는 대신 현재 상태와 다음 행동만 보여준다.

OWN-WORLD: cream/brown/neodgm·오른쪽 기존490폭 패널·기존 닫기/왼쪽 채팅과90%프로필 보존.

STORY: J→Tab주민목록→세 부탁/진행 확인→닫고 직접 주민에게 E/Q.

FIRST VIEWPORT: 기존395,170/490×490,세행265/335/405와선택상세490,하단Tab/닫기. 별도모달/전체화면 재설계 없음.

FORM: native SFML Operate 기존화면 국소확장/code-led. 새래스터/정체성/DESIGN변경 없음.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

기존체계 보존·구조/AI문서 갱신이며 새 DESIGN 작성 범위가 아니다.

## 실행 결과

기준mainf1f4d79/feature작업본, 관련4명령(resident-quest-log/field-journal/content-boundaries/pashmina-experience)·클라x64 Debug/Release2빌드 실제 VERIFY_EXIT=0. AC-01 순수요약/잠금·진행량, AC-02 생산J/Tab반복·행/외부release·읽기불변, AC-03 끊김/새로그인 탭초기화 통과. AC-04 build/tests/quest-log-{0,1,2}-{900x800,1280x720,800x900}.png9개 각각열람. 합성상태/지도+패널 생산렌더러출력이며 전체actor/HUD/실제OS게임캡처가 아니다. 전체59는등록만·5큐끝통합실행. 실제DB/서버·스트레스제품빌드/수동OS·FPS는미실행. 구조14문서142링크/diffcheck통과. 원격포함/정리·종료근거는이슈73최종댓글.

독립 quest_log_finish_review **ship**,9장각각열람/코드·명세·로그확인·필수수정없음. 잠금/보고준비 상세는코드확인이며 별도상세렌더로주장하지않는다. 기존체계보존/문서갱신으로마무리하고 전체59는명세대로최종5단계에서실행한다.
