# NPC 원본 자료와 첫 주민 적용

사용자가 2026-11-11 제공한 Hamtaro: Ham-Ham Heartbreak PNG 21개를 `source/`에 원본 그대로 보관했다. 원래 크레딧 영역을 자르거나 수정하지 않았다. 다운로드 폴더의 파일은 변경하지 않았다.

사용자가 알려준 출처: https://www.spriters-resource.com/game_boy_advance/hamtarohamhamheartbreak/
원작/캐릭터/이미지 권리는 원 권리자에게 있다. 원본 시트의 추출자 크레딧을 보존한다. 주로 Jayman/Jayman357, 일부 Lemon, Snoozer는 Koopah 표기가 있으며 정확한 표기는 각 원본을 따른다. 여기 보관했다는 사실이 배포 이용 허락을 의미하지는 않는다.

## 목록·확인 결과·활용 후보

아래 용도는 앞으로의 후보이며 '적용' 표시 외에는 게임에 생성하지 않았다. 이름은 원본 파일의 이름을 따른다.

| 보관 파일 | 원본 명칭 | 상태/활용 후보 |
| --- | --- | --- |
| arnies-brother.png | Arnie's Brother | 보관, 주민·방문객 후보, 다양한 방향/표정 |
| blushie-ham.png | Blushie Ham | 보관, 짧은 인사/부끄러움 반응 주민 |
| harmony.png | Harmony | 보관, 천사 외형의 안내/회복 연출 후보 |
| shopkeeper.png | Shopkeeper | 보관, 상인 포즈와 상점 화면/의상 UI가 섞인 자료 |
| shy-ham.png | Shy Ham | 보관, 조용한 주민·반응 포즈 |
| snoozer.png | Snoozer | **적용**, 소파 근처 잠자는 주민 정지 포즈 |
| mistress.png | Mistress | 보관, 퀘스트/표정 연출 후보 |
| penelope.png | Penelope | 보관, 어린 주민·작은 체형 |
| pashmina.png | Pashmina | **적용**, 거실 주민 정면 포즈 |
| panda.png | Panda | 보관, 제작/건설 주민 후보 |
| oxnard.png | Oxnard | 보관, 수집/먹거리 주민 후보 |
| cappy.png | Cappy | 보관, 모자 착용 주민/외형 변형 참고 |
| bye-q-child.png | Bye-Q Child | 보관, 어린 주민/가족 구성 후보 |
| busy-mother.png | Busy Mother | 보관, 가족/생활 주민 후보 |
| bulls-eye-kid.png | Bull's-Eye Kid | 보관, 놀이/도전 연출 후보 |
| boss.png | Boss | **적용**, 거실 입구 쪽 주민 정면 포즈 |
| ham-chat-j-l.png | Ham-Chat J-L | 보관, 캐릭터 전체 시트가 아닌 동작/표정/UI 참고 |
| ham-chat-a-c.png | Ham-Chat A-C | 보관, 캐릭터 전체 시트가 아닌 동작/표정/UI 참고 |
| hungry-ham-2.png | Hungry Ham 2 | 보관, 필드 몬스터 외형/먹거리 이벤트 후보(미교체) |
| inventor.png | Inventor | 보관, 제작/연구 주민 후보 |
| postie.png | Postie | 보관, 우편/안내 주민 후보 |

21개를 SFML로 로드하고 contact sheet로 전체 확인했다. 정사각 그리드 시트가 아니며 UI/크레딧/포즈가 혼합돼 있으므로 모든 이미지를 같은 칸 크기로 자동 분할하지 않는다. 검정으로 보이는 영역은 실제 투명 배경인 경우도 있고, Pashmina/Snoozer처럼 단색 불투명 배경인 경우도 있다.

## 재현과 무결성

- `tools/import-npc-sources.ps1`: 명시된 Downloads 원본 21개만 복사. 다른 내용의 기존 대상은 덮어쓰지 않고 실패한다. 원본명→보관명 매핑은 이 스크립트에 있다.
- `tools/verify-npc-sources.ps1`: 다운로드 원본 없이 `source/SHA256SUMS`의 21개 SHA256과 저장 파일 개수를 검사한다.
- `tests/run-town-preview.cmd`: 21개 로드/전체 자료 이미지와 실제 주민 렌더 코드의 클럽하우스 미리보기 생성.

기본 source rectangle (x,y,w,h): Boss (3,79,30,39), Pashmina (3,5,35,33), Snoozer (57,56,43,37). `town_npc_renderer.hpp`가 정면 대기 2포즈/방향 포즈/수면 2프레임을 시간으로 선택한다. 단색 배경 마스크는 메모리에서만 적용하고 원본 파일은 그대로 둔다. 최근접 2배 확대, 칸 하단 기준점 정렬이다. 옆/뒤는 단일 방향 포즈로 완전한 걷기 발동작 사이클은 아니다. Boss/Pashmina는 서버 산책, Snoozer는 고정 수면을 한다.

서버 배치/이름/타입/위치는 `shared/town_npcs.hpp`. 주민 타입 100~102, ID MAX_USER+200000부터로 기존 필드 몬스터 범위와 분리했다. 대화/상점/퀘스트 시스템은 미구현이며 공격/배회/추격 대상도 아니다.
