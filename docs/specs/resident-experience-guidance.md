# 체험 의뢰 수첩·안내 #70

2026-11-20 사전명세, main30fe17f 기준. 기존 후속 의뢰를 읽기만 하여 다음 행동을 보여준다. 별도 서버/DB/보상/패킷/영구저장·자동 이동 없음.

- AC-01: 활성 의뢰 동안 꽃길 조사 미완료→ID1/E, 꽃길 조사 후 강가 체험 미완료→ID2/E후F, 둘 완료→Boss 보고 안내. 미수락/보고완료는 기존 일반 탐험 추천으로 복귀. 좌표 거리는 Manhattan이며 경로 아님.
- AC-02: J 수첩을 열면 해당 의뢰 목표행 우선 선택, 다음 행동·진행/접속 한계 표시. 기존5행/방문·조사·체험 집계와 왼쪽 채팅/90%프로필 보존, 읽기만으로 완료 불가.
- AC-03: 필드 HUD의 기존 안내 슬롯에서 후속 의뢰 우선, 기존 모달/주민근처/채팅 차단은 유지. 거점 R/보고 위치·J/M 확인방법을 알린다. 완료 후 기존 방문 의뢰·일반 안내 순서로 복귀.
- AC-04: 순수 우선순위·생산J입력·읽기 불변·5상태×3크기 수첩15/활성3상태×3크기HUD9 렌더·관련검사/클라2빌드·독립검토·구조AI/원격반영. 수동OS/실DB 미검증 구분.

## Direction contract

THESIS: 수첩을 열면 ‘어디’뿐 아니라 ‘무엇을 할지’가 보인다. 새 창 대신 기존 다음행동 슬롯을 쓴다.

OWN-WORLD: cream/brown/neodgm·오른쪽490폭 수첩/하단한줄 HUD·왼쪽 채팅/프로필 유지.

STORY: 의뢰 수락 → J에서 목표·E/F 확인 → 현장 행동 → R귀환/Boss 보고 → 일반탐험 복귀.

FIRST VIEWPORT: 수첩 5행·상태열 위치 고정,556 다음행동·589 진행/재접속 한계. HUD395,632/490×26 기존슬롯 우선순위.

FORM: native SFML 기존 화면 국소확장/code-led. 새이미지/새 정체성/DESIGN 변경 없음.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

이번 범위는 기존체계 보존·현재구조/AI 기록이며 새 DESIGN 작성이 아니다.

## 실행 증거

main30fe17f 기준 feature/체험-의뢰-안내에서 구현했다. `build/tests/verify-experience-guidance.cmd` 최종 exit0: 관련5명령(resident-experience-guidance/resident-experience/field-journal/field-content/chat)·클라x64 Debug/Release2빌드. 순수목표/비활성·미수락/보고완료 복귀, 생산J 목표행과 상태/완료bit 읽기 불변, 거점/필드 보고문구 구분/끊김 초기화 통과(AC-01~04). 거리 표시는 일반 추천의 기존 Manhattan 좌표 거리이며 의뢰 추천은 장소ID와 행동키를 표시한다. 자동 경로/자동 이동은 없다.

수첩 `build/tests/experience-journal-{0,1,2,3,4}-{900x800,1280x720,800x900}.png`15장, HUD `experience-hud-{1,2,3}-{같은3크기}.png`9장을 직접 열었다. 합성 상태의 생산 렌더러 출력이며 HUD는 renderer 직접 호출로 렌더하고 숨김 조건은 owner코드/후속경계검사와 구분한다. 실제게임/수동OS/실DB·부하/FPS는 미실행이다. 전체57회귀는 등록만했고 최종3단계에서 통합 실행한다. 기존 빌드 경고를 해결했다고 주장하지 않는다. 구조14문서132링크·diff-check 통과. 독립 판정과 원격 포함/안전정리·이슈종료는 후속/이슈 #70에 기록한다.

독립 `experience_guidance_finish_review`: 24장 각각 직접 열람·명세/코드/로그 확인 후 **ship**, 필수 수정 없음. 기존 시각 체계를 유지했고 수동 OS/실제 DB 검증으로 확대하지 않는다. CP949 roundtrip도 통과했다. 최종 원격/정리 근거는 #70 종료 댓글에 기록한다.
