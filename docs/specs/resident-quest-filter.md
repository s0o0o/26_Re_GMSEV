# 의뢰 목록 필터 #76

2026-11-21 사전명세/main9a97d7f/5큐4/5.

- AC-01: 목록 F/버튼으로 전체/진행중(Active+Ready) 전환. 잠금/미수락/보고완료는진행중에서제외. 빈상태는전체보기/주민수락안내,추적불가.
- AC-02: ID기반선택/클릭·상하경계,필터변경/선택소멸시누름취소. 보이는행과실제추적ID일치, 필터로의뢰진행변경없음.
- AC-03: F반복/외부release·모달/거점왕복·접속중필터유지,끊김/새로그인전체보기초기화.
- AC-04: 순수포함/ID·생산F/클릭/빈/선택소멸/취소/초기화·4상태×3크기12렌더/관련4검사·클라2빌드/독립검토·구조AI·원격반영. 전체최종5단계/실DB·OS미실행.

## Direction contract

THESIS: 끝낸부탁은잠시접고지금할것만본다. 빈목록도막힌상태가아니라전체보기로돌아가는안내다.

OWN-WORLD: 기존cream/brown/neodgm·3행수첩/추적버튼·Tab/닫기위치보존.

STORY: J/Tab→F진행중→행추적→빈경우F전체→주민수락.

FIRST VIEWPORT:225행필터버튼/개수,265이후압축행,빈경우선택상세자리안내·추적버튼비활성. 레이아웃교체없음.

FORM: native SFML 국소확장/code-led·새래스터/DESIGN없음.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

기존체계보존/구조AI갱신으로적용한다.

## 실행 결과

AC-01/02: 순수 필터·원래 ID 대응, 생산 F 반복/행 클릭/빈 목록/읽기 불변 검사 PASS. 선택 소멸·누른 중 F 취소는 코드 확인이며 직접 실행 assertion은 마지막 통합 단계에서 보강한다.
AC-03: 초기화 검사 PASS, 거점 왕복 보존은 코드 확인/통합 단계 보강 대상이다.
AC-04: 관련 4명령·클라이언트 Debug/Release exit0, 12장 각각 열람. 독립 quest_filter_finish_review SHIP/필수 수정 없음. 실제 DB·OS 입력 미실행, 전체62는 마지막 단계에서 실행한다.
