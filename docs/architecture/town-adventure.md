# 주민 대화·안내·산책 의뢰

2026-11-17 · #61/#62/#63 · [구조 입구](README.md), [실행 큐](../specs/resident-adventure-queue.md).

## 조사·체험 후속 의뢰 #69 (2026-11-20)

[계약](../specs/resident-experience-quest.md), [이번 3큐](../specs/resident-experience-queue.md). 기존 방문 산책 의뢰는 그대로 유지한다. Reported 뒤 새 Q 요청은 `residentExperienceQuest`의 accept/report로 전환된다. 수락 가능한 조건은 활성 Record·기존 방문 보고·패널이 현재 근접성을 검사한 Boss ID/type이다. 후속 상태는 Available → Active → Ready → Reported이며 `field_investigation::State.inspected(1)`과 `completed(2)` 두 조건만 읽는다. 방문만으로 Ready가 되지 않는다. 같은 접속의 수락 전 조사/체험도 인정하고 수락과 보고는 서로 다른 Q press-release가 필요하다.

TownDialoguePanel::setQuestContext 뒤, 기존 의뢰 Reported인 경우에만 setExperienceContext가 후속 본문/Q 상태를 제공한다. owner의 입력/대화 draw에서 update하며 gameConnectionLost/beginLoginConnection은 후속도 reset한다. 거점 왕복은 보존한다. 서버 NPC idle/산책·패킷·Lua/DB/보상은 변경하지 않는다. 생산 렌더 fixture와 실제 게임/DB 플레이는 다른 검증이다.

## Pashmina 독립 의뢰 #72 (2026-11-21)

[계약](../specs/pashmina-experience-quest.md). 체험 State는 inspectId/experienceId/residentType 정의와 단계 상태를 나눈다. 기본값(1,2,100)은 기존 Boss이며 새 pashminaExperienceQuest는 (3,4,101)이다. reset은 단계만 초기화하고 정의를 유지한다. 활성접속/live Pashmina 근접 ID/type 대화에서 Q 수락/보고하며 선행 Boss 의뢰는 요구하지 않는다. 이전 같은접속 기록 인정·보고별도·중복거부/보상없음은 같다.

`syncResidentQuestContext`가 양쪽 update 후 현재 대화 주민에 맞게 본문/Q 문맥을 설정한다. Pashmina 처리와 기존 Boss 방문/체험 처리 분기를 분리해 다른 주민 보고가 의뢰를 완료하지 않는다. 끊김/새로그인은 둘 다 reset, 거점왕복은 보존한다. TownDialoguePanel은 같은 원본 idle 초상화·기존버튼/위치에서 Pashmina의 두 행동과 진행을 표시한다. 서버 NPC idle/산책/Lua/패킷/DB는 변경하지 않았다.

`resident_experience_guidance::preferred`는 동시 활성 때 Boss → Pashmina → 일반 탐험 순으로 선택한다. Goal에 residentType을 포함해 Ready 보고 대상 이름도 바뀐다. owner의 guidedExperienceQuest()를 J 목표행/수첩/필드HUD가 함께 읽으므로 우선순위가 어긋나지 않는다. 수첩 읽기로 조사/체험 완료를 만들지는 않는다.

## 위치와 입력

client.cpp의townDialogueCandidates는live players에서정식주민3ID만조회한다. shared/town_dialogue.hpp는ID/type/지역·walkable/Manhattan거리2·중간통로를확인하고가장가까운대상(동률은낮은ID)을고른다. 서버home상수위치가아니라현재관측위치로판정한다.

handleTownDialogueInput → 지도 → 옷장 → 귀환 → 채팅/이동 순서다. 열린대화는게임입력을소비한다. E/Enter/다음/닫기와E·Enter반복방지,대화중G안내/Boss·Pashmina의Q의뢰를TownDialoguePanel이처리한다. Q도release전에다시실행하지않아기존방문이있는수락이보고까지자동연속되지않는다.

현재 생산 입구(#71)는 `handleGameUiInput`이다. 일반 입력은 수첩 → 주민 → 조사 → 지도 → 옷장 → 귀환 → 채팅의 단락 소비 순서를 유지한다. KeyReleased/LostFocus만은 같은 순서로 모든 helper를 호출해 뒤쪽 패널에도 키 latch/포커스 정리가 도달하도록 한다. 이 분기는 새 수락·보고·이동을 실행하는 press가 아니다. Q를 누른 채 대화를 닫고 수첩에서 놓았을 때 다음 Q가 무시되던 경계가 생산 router 테스트에서 실패 재현 후 수정됐다. 게임 이동/공격은 router가 입력을 소비하지 않았을 때만 main의 기존 경로로 간다.

범위이탈/LEAVE는다음입력또는drawTownDialogueUi의reconcile에서닫고,지역전환/끊김/새로그인에서도정리한다. 대화는월드일시정지가아니므로기존서버NPCidle/산책/수면은계속된다. portrait역시같은원본·idle phase를쓴다. 대사는로컬고정안내이며Lua/LLM생성이나서버대화RPC가아니다.

## 안내와 의뢰

#70 후속 의뢰 진행 중에는 shared/resident_experience_guidance.hpp가 목적 행동을 선택하고 client_sample/resident_experience_hud.hpp가 E/F 또는 R/보고 문구를 만든다. J 수첩도 같은 Goal을 읽는다. 별도 위치 권위·자동 이동·보상을 만들지 않으며 완료 뒤 일반 안내로 복귀한다. [필드 콘텐츠 구조](field-content.md)에 렌더/입력 우선순위를 연결했다.

exploration_guidance::State는같은접속의field_discovery::Record에서첫미방문ID를선택한다. 본인확정AVATAR_INFO/ENTER/MOVE로Record.observe뒤안내update가실행된다. 방향은N/S/E/W조합이고거리는Manhattan좌표거리다. 실제장애물우회/최단경로가아니다.

town_walk_quest::State는별도접속중상태다.

| 상태 | 조건/행동 |
| --- | --- |
| Available | 유효Boss대화+활성Record에서Q수락 |
| Active | Record의1 Flower Walk와2 Riverside Rest 방문진행 |
| Ready | 두지점모두방문;같은접속의수락전방문도인정 |
| Reported | 유효Boss대화에서Q보고1회,중복보고거부 |

현재패널이검증한Boss target/type와근접성을owner가accept/report에전달한다. 타주민·거리밖·비활성접속·미완료보고는거부한다. 의뢰는보상/서버권위퀘스트/DB저장이아니며능력치·씨앗을변경하지않는다. disconnect/beginLoginConnection은의뢰·안내·방문Record를함께reset한다. 거점왕복은유지한다.

## 렌더와 검증

대화는기존cream/brown/neodgm패널의원본portrait와짧은대사다. Boss만Q상태버튼과접속/재접속초기화를표시한다. 일반안내/의뢰HUD는동일한하단슬롯을우선순위로공유하여겹치지않고NPCprompt/열린모달/채팅중숨긴다. Reported후에는일반탐험안내로돌아간다.

run-town-dialogue.cmd,run-exploration-guidance.cmd,run-town-walk-quest.cmd는순수규칙과생산client.cpp의주입입력/패킷·렌더를검사한다. 전체회귀에는순수규칙3개를추가해51개다. 상세실행/실패·통과/한계는 [대화](../specs/town-dialogue.md), [안내](../specs/exploration-guidance.md), [의뢰](../specs/town-walk-quest.md) 명세에서확인한다. PNG는fixture생산렌더러출력이며실접속수동OS/실제DB·부하검증이아니다.
