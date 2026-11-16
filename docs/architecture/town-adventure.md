# 주민 대화·안내·산책 의뢰

2026-11-17 · #61/#62/#63 · [구조 입구](README.md), [실행 큐](../specs/resident-adventure-queue.md).

## 위치와 입력

client.cpp의townDialogueCandidates는live players에서정식주민3ID만조회한다. shared/town_dialogue.hpp는ID/type/지역·walkable/Manhattan거리2·중간통로를확인하고가장가까운대상(동률은낮은ID)을고른다. 서버home상수위치가아니라현재관측위치로판정한다.

handleTownDialogueInput → 지도 → 옷장 → 귀환 → 채팅/이동 순서다. 열린대화는게임입력을소비한다. E/Enter/다음/닫기와E·Enter반복방지,대화중G안내/Boss전용Q의뢰를TownDialoguePanel이처리한다. Q도release전에다시실행하지않아기존방문이있는수락이보고까지자동연속되지않는다.

범위이탈/LEAVE는다음입력또는drawTownDialogueUi의reconcile에서닫고,지역전환/끊김/새로그인에서도정리한다. 대화는월드일시정지가아니므로기존서버NPCidle/산책/수면은계속된다. portrait역시같은원본·idle phase를쓴다. 대사는로컬고정안내이며Lua/LLM생성이나서버대화RPC가아니다.

## 안내와 의뢰

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
