# Boss 조사·체험 후속 의뢰 #69

2026-11-20 사전명세. 상태: 구현·실행·독립 검토 완료. 기준 main05f46c9.

기존 산책 의뢰는 방문만 판정하고 #66–68 조사/체험과 연결되지 않는다. 기존 의뢰를 바꾸지 않고 Reported 이후 Q를 새로 눌러 받을 수 있는 두 번째 의뢰를 추가한다. 꽃길1의 inspected와 강가2의 completed 두 조건을 읽는다. 같은 접속 수락 전 기록도 인정하되 수락/보고는 분리한다. 로컬 접속 중 의뢰이며 서버 권위·보상·DB 저장이 아니다.

- AC-01: 기존 방문 의뢰 완료 전 후속 수락 불가. 활성 접속+유효 Boss 대화에서만 수락/보고; 방문만으로 완료 불가. 꽃길 조사+강가900ms 체험 완료 시 Ready, 보고1회/중복 거부.
- AC-02: 기존 대화 패널의 Q 문맥을 후속 단계로 전환하고 두 조건·진행·보고·접속 초기화/보상없음을 알린다. 원래 방문 의뢰·주민 idle/초상화·레이아웃 보존.
- AC-03: 거점 왕복 유지/끊김·새 로그인 초기화, 키 반복으로 이전 보고→후속 수락 또는 수락→보고 연쇄 불가. 기존 입력/근접/모달 검사 재사용.
- AC-04: 순수 규칙/생산 입력·수명/4단계×3크기 렌더, 관련 기존 검사/클라Debug·Release, 독립 검토·구조/AI 기록 후 main 반영. 실제DB/수동OS 플레이·부하 미실행을 명시한다.

## Direction contract

THESIS: 이미 다녀온 길에서 무엇을 살폈는지 주민과 이야기한다. 별도 퀘스트 창 대신 기존 대화의 다음 이야기를 확장한다.

OWN-WORLD: 기존 cream/brown/neodgm, Boss 원본 idle 초상화와 Q/G/다음/닫기 자리 보존.

STORY: 산책 보고 → 새 Q 수락 → 꽃길 E·강가 E/F → 귀환/Boss 보고.

FIRST VIEWPORT: 기존100,440/700×220 대화, 본문 두 줄에 두 행동·하단 Q 상태 버튼. 왼쪽 프로필과 대화의 기존 위치 관계를 바꾸지 않는다.

FORM: native SFML Operate/기존 화면 국소 확장/code-led. 새 래스터·새 정체성/DESIGN 변경 없음.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

이 문장의 적용은 기존 체계 보존·구조/AI 문서 갱신이며 새 DESIGN 작성이 아니다.

## 실행 증거

`build/tests/verify-experience.cmd` 최종 묶음 exit0: 관련5명령(resident-experience/town-walk-quest/field-content/field-journal/chat) 및 일반 클라 x64 Debug/Release2빌드. 순수 후속 상태와 생산 own패킷/E/F/Q/기존방문선행·반복/거점왕복·보고/씨앗불변/끊김/새로그인 assert 통과(AC-01/03). 네 단계×세 크기12PNG를 직접 두 번 열람했고 후속 분류 문구를 ‘체험’으로 수정한 뒤 동일 검사를 재실행했다(AC-02/04).

이미지는 `build/tests/experience-dialogue-{0,1,2,3}-{900x800,1280x720,800x900}.png`다. 생산 렌더러 fixture에 own패킷과 E/F/Q를 주입하고 체험 완료 clock은 결정적으로 주입했다. 실제 게임 전체 OS 입력/실제DB·부하/성능 검증은 아니다. 기존56회귀에 순수 테스트를 등록했으나 전체 실행은 이번3큐 최종 단계에서 수행한다. 기존 C4244/MSB8028 등 빌드 경고는 남아 있다.

최초 컴파일 실패(include순서/fixture enum인수)는 수정 후 재실행한 위 결과와 구분한다. 바깥 셸의 종료만 믿지 않고 LASTEXITCODE 명시 전달·내부 로그/PASS/PNG 생성으로 확인했다. 독립 판정과 커밋/main 포함·정리/종료 근거는 #69 최종 댓글 및 아래 후속 기록에서 확인한다.

독립 `experience_finish_review`가 12장 개별 열람·코드/명세/로그 확인 후 **ship**, 필수 수정 없음. offer/ready에서 기존 페이지 표기1/2가 보이는 점은 비차단 선택 개선으로 보류했다. 기존 화면 체계를 보존한 결과이며 전체 OS 플레이 검토가 아니다. CP949 roundtrip/구조14문서130링크/diff-check 통과.
