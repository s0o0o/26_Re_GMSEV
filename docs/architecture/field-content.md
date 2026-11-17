# 필드 조사·상호작용 콘텐츠

2026-11-18 #66 · [구조 입구](README.md) · [3큐](../specs/field-content-queue.md) · [조사 계약](../specs/field-investigation.md)

`field_landmarks::all`의5ID/좌표가단일원본이다. `field_investigation::eligible`은SunnyPeak/valid/walkable/Manhattan2와중간통로를검사하고 `nearest`가동률시앞ID를선택한다. State.open은활성접속에서만target을열고별도5bit inspected에기록한다. 방문Record와구분되며영구저장·서버보상아니다.

생산client의handleFieldInvestigationInput은주민대화다음/지도·옷장·귀환·채팅/이동앞이다. 현재avatar.position은수신자기INFO/ENTER/MOVE의확정위치이며fieldDiscovery.active도필요하다. panel은E반복을거부하고클릭/닫기/Esc·입력소비/모달차단을맡는다. moveAvatar/draw마다reconcile로이탈·지역변화를정리하고disconnect/새로그인은State.reset한다. 거점왕복은조사bit보존.

오른쪽395,390의조사패널은장소별2줄설명/좌표/접속중한계·완료를보여준다. 왼쪽채팅/프로필은그대로보인다. 새이미지/패킷/DB/경제보상은없고NPC idle/월드갱신은계속된다.

#66 추가사용자요청의내프로필90%는 `profile_layout::selfStates`에서좌15/하785앵커의렌더변환으로프레임·초상화·동적글자를함께줄인다. 원본텍스처/명목NAME 슬롯/이름데이터는보존한다. `selfTop()`은676.1로바뀌며몬스터stack/채팅목표y는같은실제경계를쓴다. 채팅12px간격은유지한다.

검사: 순수field_investigation은전체53회귀에포함,run-field-content는생산client.cpp E/서버패킷/모달/세크기5장소렌더·프로필변환을검사한다. PNG는fixture생산렌더러출력이며전체게임수동OS/실제DB/성능증거아님. 실행결과는명세/이슈에기록한다.
