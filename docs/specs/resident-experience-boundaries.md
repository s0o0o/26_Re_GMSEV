# 콘텐츠 입력·수명 통합 검증 #71

2026-11-20 사전명세, 기준 maind851842. 자율3큐 마지막 단계. UI 표현/새 기능을 늘리지 않고 실제 입력·수명 경계를 검증한다. 테스트가 구체적으로 재현한 결함만 같은 범위에서 수정한다. DB/패킷/보상/새 계정/새 NPC 없음.

- AC-01: J 수첩의 채팅·주민·조사·지도·옷장·귀환 차단 각각, 키 반복/Up·Down 경계, 버튼·행의 내부 press/외부 release 취소, 열린 수첩의 다른 게임 입력 소비를 생산 경로로 검증한다.
- AC-02: 체험 중 포커스/이탈 취소·완료bit 불변, 유효 Boss 이탈/LEAVE 후 잘못된 보고 차단, 거점 왕복 보존/끊김·새 로그인 전체 콘텐츠 초기화를 검사한다.
- AC-03: 전체57회귀·관련 생산검사·일반클라x64 Debug/Release2빌드, CP949/원본NPC21해시·구조문서/링크·diff-check. 새 실행기의 실패 종료가 바깥 셸에도 전파되는지 확인한다.
- AC-04: 생산 렌더러의 관련 테스트 장면 제공, 필요 시 UI 수정이 발생하면 변경 화면 재렌더/독립 검토. 시각 변경이 없으면 기존 #69/#70 검토 범위를 유지하고 이번 작업은 검증 전용으로 명시한다. 3단계 이슈/원격 포함·브랜치 안전정리/큐·AI 문서 마무리.

전체 테스트는 DB대역/loopback/합성 상태·주입 입력 증거이며 실제DB 부하/수동OS 플레이/장시간 성능 검증과 다르다. 신규 권한이 필요하면 자동 확대하지 않는다.

생산 검증 연결 계획: main의 UI 입력 순서를 `handleGameUiInput`으로 추출하여 테스트가 동일 소비 경로를 호출한다. 기존 우선순위는 유지한다. 모달 전환 중 release/focus가 하위 패널에 전달되지 않아 키가 붙는 경우도 회귀 시나리오로 검사한다.

## 구현·재현 결과

main의 기존 단락 입력 순서를 helper로 추출했다. 최초 경계검사에서 `Q 누른 상태 → 대화 Esc → 수첩 J → Q 해제 → 수첩 Esc → Boss E/Q` 후 후속 Active가 되지 않는 assertion 실패/바깥 VERIFY_EXIT=1을 관측했다. KeyReleased/LostFocus만 전 helper로 전달하도록 수정한 뒤 같은 시나리오와 J→지도 간 latch,6모달 차단·행 상하 경계·외부release, 포커스/이탈 취소, Boss LEAVE/다른NPC 보고 거부, 접속 종료·새로그인 초기화가 PASS/exit0였다. 이 검사는 실제 OS 키보드 조작이 아니라 생산 router 이벤트 주입이다.

UI 색/위치/본문은 변경하지 않았다. 관련 미리보기 `build/tests/content-boundaries-900x800.png`는 경계검사 끝의 Boss 보고 완료 상태를 생산 렌더러로 만든 출력이다. 월드 actor 전체/실제DB 접속 캡처가 아니며 대화 상태 확인 범위다. 기존36장(#69의12/#70의24)도 최종 묶음에서 재생성할 예정이다. 전체57/관련12/클라2빌드 결과·독립 검토와 원격 종료 증거는 최종 확인 뒤 추가한다.

독립 `content_boundaries_finish_review`: 생산 router·패널 release/focus 분기·main 호출·테스트/실행기·요구된 preview를 읽기 전용 검토 후 **ship(최종 전체검증 확인 조건)**, 필수 수정 없음. release/focus만으로 수락/보고/이동을 만들지 않음을 코드 확인했다. M/W/R/Tab 등 모든 latch 조합 전수 검사는 비차단 후속 제안으로 보류하며 Q/J 재현 범위를 모든 조합 검증으로 확대하지 않는다. 이전 #69/#70 전체 화면 검토와 이번 입력 경계 검토는 다른 범위다.

## 최종 통합 실행

`tests/run-content-queue-checks.cmd`를 VS x64 환경에서 실제 실행, 내부/바깥 `VERIFY_EXIT=0` 확인. 전체57회귀, 별도12명령(content-boundaries/resident-experience/resident-experience-guidance/field-journal/field-content/chat/town-walk-quest/field-map/sprite-frames/wardrobe-ui/display/travel-client), 일반클라x64 Debug/Release2빌드 모두 PASS. 로그 `build/tests/resident-queue-*.log`. 독립 ship의 최종 검사 조건을 충족했다. 빌드 경고는 남아 있으며 서버/스트레스 도구 제품 빌드·실제DB·수동OS·장시간 성능은 미실행이다.

NPC21 SHA256·CP949 roundtrip·구조14문서136링크·diffcheck 통과. 관련36PNG와 경계preview가 최종 테스트에서 재생성됐다. 새 게임용 래스터 자산은 만들지 않았다. 본 큐의 원격 main 포함/구현·병합 커밋/안전정리·종료 증거는 #69/#70/#71 최종 댓글에서 확인한다. 세 작업 이후 후보는 Pashmina 의뢰이며 이번 승인으로 네 번째 작업을 자동 시작하지 않는다.
