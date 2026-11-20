# 의뢰 사용성 최종 경계 #77

5큐 마지막. UI/제품 동작 변경 없이 기존 생산 입력 fixture를 보강한다.

- AC-01: 추적 버튼을 누른 중 F 전환 또는 선택 의뢰 소멸 후 release가 다른 의뢰를 고정하지 않는다. 상태 변경은 fixture 준비이며 제품 필터 입력은 의뢰 진행을 변경하지 않는다.
- AC-02: 끊김·새 로그인 각각 직전에 진행 중 필터와 수동 추적이 실제 활성인지 assertion 후 초기화를 확인한다.
- AC-02b: 기존 안내 fixture도 현재 Snapshot의 Boss 체험 선행 조건(산책 보고)을 준비하여 preferred row assertion을 유지한다. 제품 잠금을 우회하는 변경은 하지 않는다.
- AC-03: 전체62 회귀(순수 규칙 및 DB 대역 네트워크 포함)/관련17 명령/클라 Debug·Release를 실행하고 실패·미검증을 구분한다. 기존12 필터 렌더를 재생성하고 대표 이미지를 제공한다.
- AC-04: 구조/AI/큐/이슈에 증거를 남기고 원격 main 포함과 안전 브랜치 삭제를 확인한다.

DB·OS 수동입력·장기부하·새 UI/자산/패킷/보상/영구저장은 제외한다. 추가 렌더를 전체 게임 캡처로 주장하지 않는다.

## 실제 실행 결과

AC-01/02/02b: 생산 router held-F/선택 소멸 취소/각 초기화 활성 전제, 기존 안내 preferred row PASS. 테스트 준비 연결 상태와 산책 보고 선행 조건을 수정했으며 제품 코드는 변경하지 않았다.

AC-03: 전체62 회귀 마지막 재실행 PASS (`resident-queue-regressions.log`). 관련17/클라2는 fixture 수정 후 별도 wrapper 실행 `Related17 and client2 PASS`, 외부 VERIFY_EXIT=0. 모든17 로그의 최신 시각/실패 assertion 부재·빌드 산출물 확인. 실행은 단일 무실패 묶음이 아니었다: 최초 localhost connect 제한, 다음 기존 server_worker:928 실패, 이후 전체62 통과, UI fixture 준비 실패2건 및 수정 후 관련17 통과. 동시 공격 실패는 [#78](https://github.com/s0o0o/26_Re_GMSEV/issues/78) 후속으로 보존하며 원인 해결을 주장하지 않는다.

기존 필터12장 재생성/대표 quest-filter-1-900x800 및 안내 experience-journal-2-900x800 직접 열람. 생산 렌더러 fixture이며 실제 게임 OS 캡처가 아니다. UI 스타일 변경이 없어 새 독립 시각 검토는 하지 않았다(필터12장 #76 독립 SHIP 유지). CP949 roundtrip/NPC21 SHA256/구조14문서147링크/diffcheck 통과. 기존 C4244/MSB8028/LNK4098 경고는 별도다. 실제DB/수동IME/장기성능은 미실행.

AC-04: 구조/AI/큐 갱신. 커밋·원격 main 포함/안전 삭제·종료 근거는 #77 최종 댓글에 연결한다.
