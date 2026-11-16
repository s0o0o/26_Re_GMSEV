# 필드 구역 장식·랜드마크 #58

2026-11-17 · 구현 전 명세. 자율 큐1/3. 기존256×256/청크16/가시18×16/캐시12를 유지한다.

- AC-01: 공통5개 지점의 고유ID/범위·walkable/거점 연결 도달 확인. 기존 field_terrain의 전체 타일·충돌은 변경하지 않는다.
- AC-02: deterministic 구역 지면 색조와 grass 위 원본 꽃 패치, 지점별 원본 오브젝트/지면표식을 가시 범위 안에만 렌더한다. starter/길/다리/출구를 장식으로 덮지 않는다. 캐시/타일 상한과 별도 장식 수를 검사한다.
- AC-03: 기존 원본 Sunny Peak와 픽셀/nearest 유지, 랜드마크/먼끝/강/절벽·크기별 생산 렌더를 실제 확인한다. 원본 자체를 수정/생성하지 않는다.
- AC-04: field/chunk/픽셀/관련 회귀·native Debug/Release, 독립검토·구조/AI 기록·main 포함/안전정리/이슈 종료. 실제DB/부하 성능 보장 아님.

## Direction contract

THESIS: 넓은 필드를 반복 타일이 아닌 기억할 수 있는 산책 구역으로 만든다.

OWN-WORLD: 기존 Sunny Peak 꽃/그루터기·잔디와 정수 픽셀 원본, 크림/갈색 UI. 새 시각 세계/새 raster 없음.

STORY: 출발지→꽃길→강가→절벽 아래→동쪽 들판의5개 고정 표식을 발견한다. 다음 단계 지도와 방문 기록이 같은ID를 참조한다.

FIRST VIEWPORT: 기존 authored 출발 화면은 보존하고 동쪽 출구 이후18×16 가시 지형 안에만 변화. 길은 비우고 landmark 중심은 걸어 들어갈 수 있는 지면 표식이다.

FORM: 기존 필드의 국소 확장/code-led, 기존 디자인 보존. seed: none (local extension; no roll).

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

기존체계 확장으로 DESIGN 파일/원본 PNG를 재작성하지 않는다. 기존 자산 출처 기록을 유지하고 reviewer에 실제 렌더·상한 검사를 전달한다.

## 실행 이력

최종 독립검토 `landmarks_finish_review`: 필수8장 직접 열람, #58 국소확장 범위 **ship**. 표식/그루터기 배경·길·다중크기 확인. 1/5번은 지형보다 눈금으로 구별되는 한계를 기록하고 다음 지도에서 동일ID/이름을 대응한다. AC-01~04 정의된 범위 검증 완료; main 포함/정리는 #58 종료 댓글에 기록한다.

2026-11-17: `run-field-landmarks.cmd` 전수62640도달/5지점ID·walkable/grass장식조건, `run-field-chunks.cmd` 실제4worker·DB대역/충돌·귀환, `run-field-preview.cmd`5지점/강/절벽/먼끝·256청크 sweep/상한·다중크기, `run-world-character.cmd` 픽셀8프레임/90외형 통과. 일반클라 x64 Debug/Release2빌드 성공. 전체46회귀/실제DB/부하/수동OS 검사는 이번 단계에서 재실행하지 않았다.

처음 실수로 직전 sprite 검증 묶음을 실행했으며 그7검사/클라2빌드도 통과했지만 이를 field 검증으로 대체하지 않았다. 이후 `verify-landmarks.cmd`의4검사/2빌드를 따로 실행하고 시각수정 뒤 같은 묶음 재실행exit0. 로그 build/tests/landmarks-*.log. 독립검토/main 포함은 실제 완료 후 이슈에 연결한다.
