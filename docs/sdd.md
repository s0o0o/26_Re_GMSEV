# 가벼운 스펙 기반 개발 (SDD)

목적은 채팅을 다시 읽지 않고도 **왜 바꾸는지, 무엇을 바꾸지 않는지, 어떻게 완료를 판단하는지** 저장소에서 찾게 하는 것이다. 외부 프레임워크나 문서 생성 도구는 설치하지 않는다. 기존 [Git·이슈 절차](development-workflow.md) 안에서 사용한다.

## 언제 쓰나

- 새 동작, 동시성·오류 처리, 여러 모듈 리팩토링, 서버/클라이언트/프로토콜 경계 변경: 명세를 먼저 작성하거나 관련 명세를 갱신한다.
- 오탈자, 정렬, 동작 불변의 작은 수정: 이슈/커밋에 목적·범위·검증만 기록한다. 새 명세·별도 계획 파일은 필요 없다.
- 이미 끝난 작업을 정리할 때는 반드시 **사후 명세**로 표시한다. 과거에 명세부터 작성한 것처럼 꾸미지 않는다.

## 작업 순서

1. Git 상태·원격 main·관련 코드·기존 명세를 읽고 사용자 변경을 보존한다. 이슈를 발행하고 최신 main에서 `feature/설명`을 만든다.
2. [템플릿](specs/_template.md)을 `docs/specs/짧은-주제.md`로 복사한다. 목적, 범위/제외, 현재/목표 동작, 영향, 오류·경계, 완료 기준을 적는다. 모르는 것은 가정/열린 질문으로 표시한다.
3. 명세 상태를 `초안 → 구현 중 → 검증 완료`로 관리한다. 미해결 선택이 결과를 크게 바꾸면 사용자 확인 후 진행한다. 단순히 문서가 있다는 이유로 승인된 요구사항이라고 간주하지 않는다.
4. 완료 기준을 `AC-01`처럼 식별하고 같은 파일의 작업 표에 구현 위치·테스트를 연결한다. 구현 중 범위 변경은 이슈와 명세에 반영한다. 새 기능을 몰래 포함하지 않는다.
5. 검증 결과에 날짜·기준 커밋·명령·환경·결과·한계를 기록한다. `코드 확인`, `실행 통과`, `미실행`, `실패`를 구별한다. 실패/미실행 기준을 통과로 바꾸지 않는다. 결과는 덮어쓰지 말고 날짜별로 남긴다.
   구조/함수흐름/스레드·잠금/좌표·패킷/DB/AI/검증방식이 바뀌면 [현재 구조 문서](architecture/README.md)도 같은 작업에서 갱신한다. 과거 refactoring 기록만 추가하는 것으로 대체하지 않는다.
6. `feat : 한국어 설명` 커밋 → main 병합/push → 포함 확인 → `git branch -d` → 이슈 결과 기록/종료. 명세 링크와 작업/병합 커밋을 이슈에 남긴다. 검증 미완료가 이번 범위의 필수 기준이면 작업을 완료 처리하지 않는다.

## 최소한 남길 연결

`이슈 ↔ 명세의 AC ID ↔ 구현 작업·파일 ↔ 테스트·실행 결과 ↔ 커밋`

계획과 결과는 명세 하나에 함께 적어도 된다. `refactoring-step-*.md`는 사용자가 이해하기 쉬운 변경 설명으로 유지하고 명세를 링크한다. 같은 설명을 여러 파일에 중복해서 유지할 필요는 없다.

## 검증 원칙

- 관찰 가능한 기준을 쓴다: “안정적이다” 대신 “같은 숫자 ID의 새 접속은 이전 connectionId를 거부한다”.
- 테스트 대역과 실제 경로를 구분한다. 패킷 캡처, TCP loopback, 실제 게임 화면, 실제 DB/부하 실행은 서로 다른 증거다.
- DB 없는 테스트 통과를 실제 DB 동시 접속 안정성으로 확대하지 않는다. 실제 DB 테스트의 계정 생성·데이터 변경 가능성은 별도 안내한다.
- 문서만 바꿨다면 UTF-8, 상대 링크, 기준/작업/검증 연결과 `git diff --check`를 확인한다. 실행하지 않은 빌드는 새로 통과했다고 쓰지 않는다.

## 명세 목록

- [Pashmina 조사·체험 의뢰](specs/pashmina-experience-quest.md): 독립3/4조건·주민별 문맥·Boss 우선 수첩/HUD 안내.

- [콘텐츠 입력·수명 경계](specs/resident-experience-boundaries.md): main 공유 router·모달 간 키 해제/포커스 정리·거부/초기화 회귀와3큐 통합.

- [체험 의뢰 수첩·안내](specs/resident-experience-guidance.md): 의뢰 목표행·다음 행동 우선순위와 기존 HUD 슬롯 공유.

- [주민 체험 3큐](specs/resident-experience-queue.md), [Boss 후속 의뢰](specs/resident-experience-quest.md): 기존 산책 보고 이후 조사·체험과 주민 이야기 연결.

- [탐험 수첩](specs/field-journal.md): 방문/조사/체험 읽기 전용 요약·다음 장소/좌표 거리·J 입력 경계.

- [장소 상호작용](specs/field-interaction.md):0.9초연출·취소/쿨다운/접속중완료bit.

- [필드 콘텐츠3큐](specs/field-content-queue.md), [탐험지 조사](specs/field-investigation.md): E조사→장소연출→수첩 순차실행.

- [긴 채팅·기록 탐색](specs/chat-history.md): v6/119byte·줄바꿈·스크롤앵커/최신복귀·Windows입력검증한계.

- [지역 채팅·프로필 배치](specs/region-chat.md): UTF-8/지역 송수신·기록/프로필 공간 자동 이동.

- [Boss 산책 의뢰](specs/town-walk-quest.md): 접속중수락/방문/근접보고·Q반복/세션경계·보상없음.

- [주민 탐험 안내](specs/exploration-guidance.md): 대화요청/미방문목표·지도링/좌표방향거리·세션경계.

- [주민·탐험 연결 큐](specs/resident-adventure-queue.md), [근접 주민 대화](specs/town-dialogue.md):순차3작업과현재관측위치·입력/대화수명계약.

- [접속 중 탐험 기록](specs/field-discovery.md): 서버확정위치/5비트방문·중복/세션경계·지도체크리스트.

- [필드 지도](specs/field-map.md): 공통 실제지형/위치/표식 지도와 입력·상태 경계.

- [필드 탐험 자율 큐](specs/field-exploration-queue.md), [장식·랜드마크](specs/field-landmarks.md):3단계 연속 실행과 비충돌 렌더/공통ID·위치 계약.

- [스프라이트 프레임·HUD 경계](specs/sprite-frame-integrity.md): 사용 중 주민 crop 완전성·훈스터 원본 경계·이름 슬롯과 렌더 검사.

- [클럽하우스 옷장](specs/clubhouse-wardrobe.md): draft/서버 검증·동기화·승인된 DB 저장/복원과 실제 부하 비교 한계.

- [넓은 필드 청크·충돌](specs/field-chunks.md): 256×256 지형/충돌, 제한 LRU·가시 표시·경로 예산과 v4 범위 계약.
- [거점↔필드 귀환](specs/hub-field-travel.md): 서버 권위 목적지·중복 방지·전환 정리·native 버튼/R키와 실패 처리.

- [기존 GUI 부하 수명·단계 재현](specs/gui-load-lifecycle.md): 승인계정/local상태·IOCP중지/수거와미재현한계.
- [서버 지역 상태·구조 문서](specs/server-region-state.md): 서버Position단일원본/이동·AI/DB경계와상시architecture갱신.
- [일반클라 지역 내부 상태](specs/client-region-state.md): 2B-2B1 native위치/카메라local, 서버상태는후속.
- [지역 좌표 패킷 v2](specs/region-wire-v2.md): 2B-2A wire/local과 임시legacy렌더·DB경계, canonical상태는후속.
- [기존300계정 유지 시간 확장](specs/extended-mixed-load.md): 제한형120초 혼합/실제 DB 재사용과 미검증 조건 구분.
- [일반 exe 로그인·재시도](specs/native-login-validation.md): 실행 경로 독립 리소스 탐색과 실제 OS 창/DB 증거.
- [템플릿](specs/_template.md)
- [송신 완료 경합 완화](specs/send-completion-fast-path.md): 정상/이미종료중SEND의수명해제와부분송신차단.
- [월드 캐릭터 크기·픽셀](specs/world-character-scale.md): 커마 표시 보존, native 월드 아틀라스와 정수2배.
- [부하 처리량·실제 동접 장면](specs/load-throughput-scene.md): 실행기 ramp/이동 응답과 실제DB 생산 클라 코드 scene.
- [혼합 부하·종료 오류 계측](specs/mixed-load-diagnostics.md): opt-in 시각/연결 trace와 수신 drain 비교.
- [300명 제한형 부하 검증](specs/bounded-load-test.md): DB 대역과 승인된 실제 DB 실행의 구분.
- [DB 위치·로그인 계약](specs/database-position-contract.md): 읽기 전용 메타데이터 확인과 안전한 지역 위치 저장 경계.
- [지역별 섹터 인덱스](specs/region-sector-index.md): 2B-1 독립 지역 bucket과 단일 소속 관리.
- [주민 산책·대기 동작](specs/town-resident-behaviour.md): 서버 자율 행동과 클라 보간/프레임.
- [NPC 원본과 주민 배치](specs/town-residents.md): 첨부 21개 보관, 클럽하우스 비전투 주민 3명.
- [지역 ID·좌표 호환 경계](specs/world-region-boundaries.md): 지역 모델과 시야/전투/AI 경계, 패킷/DB 전환 전 단계.
- [월드 확장·화면 대응 로드맵](specs/world-display-roadmap.md): 가변 해상도부터 지역/귀환/필드/커마까지 단계 경계.
- [Lua AI 흐름 개선](specs/lua-ai-flow.md): 실제 역할 조사와 안전한 script 정책/타이머 경계.
- [월드 상태 보호](specs/world-state-concurrency.md): 이동·전투·패킷 읽기의 공통 잠금과 DB 경계.
- [접속 목록 동기화](specs/session-registry-concurrency.md): registry와 입장·퇴장 경계를 구분하는 다중 worker 작업.
- [실제 worker 통합 검증](specs/server-worker-harness.md): DB 대역과 실제 네트워크 경로를 구분한 검증 명세.
- [비동기 접속 수명](specs/async-session-lifetime.md): 첫 사전 명세 적용 작업.
- [세션 종료·재접속과 월드 이벤트](specs/session-world-lifecycle.md): 첫 사후 적용 사례. 구현된 동작과 미검증 네트워크 수명 경계를 분리한다.
