# 34단계: NPC 자료 보관과 클럽하우스 주민

이슈 [#36](https://github.com/s0o0o/26_Re_GMSEV/issues/36), [사전 명세/AC별 결과](specs/town-residents.md).

## 이번 결과

첨부 21개 PNG 전부를 assets/npcs/source에 원본 그대로 저장하고 SHA256으로 복사 무결성을 확인했다. 원본 크레딧도 보존했다. 전체 목록과 용도는 [NPC 자료 목록](../TP_2023192003_CLIENT/client_sample/assets/npcs/README.md)에 정리했다.

첫 적용은 Boss/Pashmina/Snoozer다. Boss는 거실 위쪽, Pashmina는 아래쪽, Snoozer는 소파 근처에 배치했다. 주민 3명은 단순히 클라에서 그림만 그리는 장식이 아니라 서버 객체로 생성되어 ENTER/LEAVE 패킷과 지역 시야를 따른다.

## 기존 코드의 문제와 변경

기존 `id>=MAX_USER` 캐릭터는 전부 몬스터처럼 취급했다. 이 분기를 그대로 재사용하면 주민에게도 배회·추격·공격 UI가 적용될 수 있다. 기존 타입 1/2를 덮어쓰지 않고 주민 타입 100~102를 별도로 두었다. ID는 MAX_USER+200000부터 예약해 최대 기존 필드 몬스터 범위와 충돌하지 않도록 static_assert로 확인한다.

- `shared/town_npcs.hpp`: 주민 ID/타입/이름/위치/원본 키, hostile/resident 판정의 공통 정의.
- `myserver.cpp`: 초기화 때 주민 3명 등록과 섹터 배치. NPC 깨우기/배회/타이머/추격에서 주민 제외, 공격 목록은 hostile 타입만 허용. 기존 몬스터의 Lua/전투/부활은 유지한다.
- `town_npc_renderer.hpp`: 원본 텍스처의 정지 포즈 영역을 직접 렌더링한다. Pashmina/Snoozer의 단색 배경은 메모리에서만 투명 처리한다. 원본 PNG는 바꾸지 않는다. 최근접 확대와 기준점/중앙 이름표로 기존 픽셀 스타일을 유지했다.
- `client.cpp`: 주민 ENTER의 외형과 이름을 받고 주민 렌더 경로로 표시한다. 주민은 HP바와 공격 타깃 HUD에서 제외한다. 기존 NPC 타입을 객체 생성 전에 기록해 생성으로 유실하던 부분을 생성 후 설정으로 정리했고 id/type 초기값과 원본 이름 저장도 명시했다. 모르는 NPC 타입은 초기화 안 된 객체를 만들지 않고 무시한다.

주민은 정지 포즈다. 통행 충돌도 추가하지 않았다. 대화/상점/퀘스트/걷기 일정은 아직 구현하지 않았으며 주민 HP=1은 공통 객체 필드의 기본값이지 공격 가능하다는 뜻이 아니다. 필드로 주민을 옮긴 테스트에서도 역할 검사로 공격이 차단된다.

## 자료 활용 방향

21종은 주민 후보, 상점 UI 혼합 자료, Ham-Chat 동작/표정 자료로 나눴다. Shopkeeper/Inventor/Postie 등은 향후 역할을 붙이기 좋지만 아직 배치하지 않았다. Hungry Ham 2는 필드 외형 후보로 보관했을 뿐 기존 몬스터를 교체하지 않았다. Ham-Chat을 일반 캐릭터 4방향 시트로 오인하지 않도록 목록에 표시했다.

## 검증/사용 방법

서버·클라이언트를 새 빌드로 함께 실행하면 클럽하우스의 주민이 보인다. 패킷 크기는 유지하지만 이전 클라이언트는 주민 타입을 모르므로 새 버전이 필요하다. stress 도구는 ENTER를 무시하므로 주민 표시 대상이 아니며 빌드 호환만 확인했다.

전체 회귀 29개, 주민 4-worker TCP 반복 10회, 추가 GPU 2개, 서버/클라/스트레스 Debug/Release와 DB 없는 startup-check를 실행했다. 실제 client.cpp packet fixture는 ENTER→OBJECT.draw→LEAVE, 주민 HP HUD 미등록, 기존 hostile HP HUD 보존을 확인한다. 실제 DB 로그인 게임 화면을 촬영한 것은 아니다. 세부 실행/미검증/초기 실패 수정은 명세에 구분했다.

`tests/run-town-preview.cmd`는 `build/tests/npc-source-catalog.png`(21종 전체)와 `build/tests/town-residents-preview.png`(실제 주민/맵 렌더 코드 출력)를 생성한다. 두 이미지 모두 확인했다. 새 그림 생성이나 원본 편집 대신 기존 리소스와 중앙 정렬을 유지하는 데 UI 스킬 지침을 참고했다.

## 다음 작업

지역 ID/지역 내부 좌표/지역별 섹터와 패킷·DB 전환(2B)을 이어간다. 주민은 이번 공통 정의/역할 구분을 그대로 새 지역 상태로 옮긴다. 이후 귀환 버튼과 넓은 필드, 그리고 대화/옷장/상점 순으로 확장할 수 있다.
