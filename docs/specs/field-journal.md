# 탐험 수첩 #68

2026-11-18 사전명세,자율3큐3/3. #66/#67와기존방문Record를읽기만한다. 새DB/패킷/보상/임의이동/영구저장없음.

- AC-01: J/버튼으로활성접속의수첩열기,5장소방문/조사/체험각각표시·총계/접속한계. 선택행/상세설명,방문만/조사만/완료/전체완료구분.
- AC-02: 첫체험미완료ID를다음후보로표시하고필드의Manhattan좌표거리/거점에서는R필드안내. 목표자동이동/최단경로라고주장하지않는다. 전체완료면축하문구·보상없음.
- AC-03: J반복/닫기·Esc/행클릭/상하선택·상호모달/채팅·귀환·끊김/새로그인보호. 수첩읽기로조사/체험완료불가. 기록거점왕복유지/재접속초기화.
- AC-04: 순수요약/생산J/버튼·선택/모달·수명·4상태×3크기12렌더/전체55회귀·추가관련검사·클라2빌드·독립검토/구조AI/3큐원격감사.

## Direction contract

THESIS: 방문과행동의차이를한표에서읽고다음산책지를고른다.

OWN-WORLD: 기존cream/brown/neodgm/오른쪽패널,왼쪽채팅/90%프로필보존.

STORY: J열기→5행상태표/선택상세→남은장소확인→닫고직접이동.

FIRST VIEWPORT: 오른쪽395,170/490×490,5행/3상태열·선택설명·다음후보·닫기/접속한계. 행선택은읽기만.

FORM: 기존HUD국소확장/code-led,no newraster/DESIGN. 기존지형/ID·기록재사용.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

적용범위는기존체계보존·구조AI문서갱신이며새DESIGN작성아니다.

## 구현·실행 증거

2026-11-18, 기준 main2c2c200의 feature/탐험-수첩 작업본.

| 완료 기준 | 구현·실제 실행 |
|---|---|
| AC-01 | FieldJournalPanel의 5행/3상태·총계/선택 설명, shared/field_journal.hpp 순수 집계. 생산 클라 fixture의 빈 기록/방문만/혼합/전체 완료 4상태 렌더. |
| AC-02 | first incomplete ID와 SunnyPeak Manhattan 거리, 거점 R 안내/전체 완료 문구. 순수 테스트에서 거점 -1/같은 좌표0/전체 완료 next0 확인. 자동이동·실제 길찾기 아님. |
| AC-03 | 생산 client.cpp J fresh/repeat/버튼·행 클릭/방향키/닫기/Esc/채팅 차단·거점 이동 보존/끊김 초기화/읽기 불변 assert 실행 통과. 기타 모달 guard는 코드 확인과 관련 기존 검사로 보완하며 모든 OS 키 조합을 직접 누른 검증은 아니다. |
| AC-04 | 아래 통합 묶음 exit0. 12PNG 두 번 직접 열람, 첫 열람의 미지원 방향키 기호를 문구로 교체 후 재생성. 독립 검토 결과와 원격 종료 근거는 후속 기록/이슈 #68. |

`build/tests/verify-field-queue-final.cmd` 실행 exit0: 전체55개 회귀 + 별도9명령(field-journal/field-content/chat/town-walk-quest/field-map/sprite-frames/travel-client/wardrobe-ui/display) + 일반 클라 x64 Debug/Release2빌드. 로그는 `build/tests/field-queue-*.log`다. 실행기는 양수/음수 실패 종료를 거부한다. 기존 C4244/MSB8028/LNK4098 경고는 해결했다고 주장하지 않는다. 거부/연결실패를 기대하는 fixture 로그는 해당 검사의 최종 PASS와 구분해 읽는다.

생산 렌더러 fixture 출력 `build/tests/journal-{0,1,2,3}-{900x800,1280x720,800x900}.png`12장. 기록은 fixture에서 상태를 주입해 만든 것으로 실제 서버에 연결해 네 상태를 모두 탐험한 장면이 아니다. 전체 게임 수동OS/실제DB·부하/FPS·서버/스트레스 도구 제품 빌드는 이번 큐 최종검사에서 미실행. 새 패킷/서버/DB 변경이 없는 클라 콘텐츠 범위다. 기존 주민/NPC 원본과 idle은 유지한다.

독립 `journal_finish_review`: 12장 각각 직접 열람·명세/구현/테스트 읽기 후 **ship**, 필수 수정 없음. 채팅 외 각 모달·Up 경계·외부 release·새 로그인 직접 호출 assertion은 선택적 후속 검사로 제안됐다. 현재 코드 결함으로 판정되지 않아 이번 UI 변경을 늘리지 않고 후속으로 남긴다. 기존 테스트의 modal/session PASS를 모든 모달/새 로그인 실행으로 확대하지 않는다. NPC21개 원본 SHA256, CP949 roundtrip, 구조14문서/127링크, diff-check도 통과했다. 본 단계 구현/병합 커밋·원격 포함·안전정리·이슈 종료는 [#68](https://github.com/s0o0o/26_Re_GMSEV/issues/68)의 최종 댓글이 근거다.
