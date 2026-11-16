# 접속 중 탐험 기록 #60

2026-11-17 · 구현·검증 완료(사전 명세에서 갱신) · 큐3/3. 지도/표식과 같은5개ID를 사용한다. 서버보상/DB저장/새패킷/계정생성없음. main포함/안전정리·최종종료증거는 #60 종료 댓글에 연결한다.

- AC-01: 서버가 확인한 본인 위치가 SunnyPeak 유효walkable타일이고 지점좌표의Chebyshev거리2이내면 방문1회 기록. 잘못된지역/경계/막힌타일/범위밖/다른플레이어 이동은 기록하지 않는다. 클라예측입력만으로기록하지 않는다.
- AC-02: 거점/필드 전환은기록유지,연결종료·새로그인은초기화. 로그인시기존저장위치가도달조건이면정상기록. 5비트/5지점 상한,세션이아니면기록거부.
- AC-03: 기존지도에서미방문/방문을명확히구분하고진행0~5와접속중기록·재접속초기화를설명한다. 빈기록/일부/전체·다중크기·현재위치겹침/긴범례검사. 별도새패널/보상오인없음.
- AC-04: 순수규칙·실제생산패킷처리/연결정리·지도입력회귀,전체회귀와큐추가검사·클라Debug/Release빌드,실제생성미리보기/독립검토·원본문서/AI기록·main원격포함/안전정리/이슈종료후3큐완료감사.

## Direction contract

THESIS: 산책의목표를다섯표식의방문으로작게완결한다.

OWN-WORLD: 기존지도cream/brown·neodgm,미방문gold/방문green과글자상태동시표현. 새이미지없음.

STORY: 표식근처도달→지도범례의방문표시/진행수확인→나머지찾기. 접속중기록이며저장/보상은없다.

FIRST VIEWPORT: 기존지도좌측실지형은유지. 우측5범례각이름아래미방문/방문표시,마지막범례아래진행수,하단지도아래세션설명. 기존닫기/귀환안내위치는보존.

FORM: 지도내국소확장/code-led,no conceptroll/newDESIGN. 서버확정위치기반의비경제로컬체크리스트.

FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, DESIGN.md, and every shipping raster carrying its provenance

## 실행 증거 (2026-11-17)

최종 독립 `discovery_finish_review`:9PNG·계약/생산패킷/연결경계/규칙·로그확인 후 **ship**. 수정요구없음. 방문색이지형녹색과가까우나동일ID범례의상태글자/진행수로색만의존하지않음을확인했다. 0/5사진의현재위치가2번위인것은sample Record상태합성이므로실제시간순플레이장면으로부르지않는다는제안채택. 기존MSB8028경고는남아있다. AC-01~04의정의된범위검증완료.

- AC-01/02: 전체회귀의 `field_discovery_test`는inactive/타지역/경계/거리2포함·3제외/막힌절벽/중복/5개상한/거점유지/reset을검사한다. 생산 `field_map_test`는본인AVATAR_INFO·중복INFO·본인MOVE/ENTER·타인MOVE·거점왕복·local배치무기록·disconnect/newavatar와실제beginLoginConnection reset을검사했다.
- 새로그인초기화 검사는 localhost port0 접속실패를의도한것이다. `[LOGIN] Failed ... sent=0 received=0`뒤기록0/비활성을assert하며DB/실서버를실행하지않았다. 이예상실패를기능회귀로오인하지않는다.
- AC-03:0/2/5방문 상태 각각900×800/1280×720/800×900 총9PNG를생성했다. base3+resized6을주에이전트가직접확인했다. 예제Record로화면상태를만든합성검사이며실제전지점을OS로걸어간캡처가아니다.
- AC-04: `verify-exploration-final.cmd` exit0, 전체48회귀+추가11명령(field-map/landmarks/field-preview/sprite-frames/world-character/town-preview/wardrobe-client/UI/travel/display/login-preview)와native x64 Debug/Release2빌드통과. 로그 build/tests/exploration-*.log. native소스CP949roundtrip확인/최소diff,원본raster미변경. 실제DB/실제부하/수동OS검사는이번큐에서미실행이다.
- 독립검토: `discovery_finish_review`에9PNG·생산코드/규칙/경계테스트·명세/기존참조/로그를전달했다. 최종판정/main원격/정리는실제완료후연결한다.
