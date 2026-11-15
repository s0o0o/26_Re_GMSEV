# 2B-2A: 지역 내부 좌표 패킷 v2

2026-11-15 기준1515f2c / #49. 상태: 2B-2A 검증 완료. 2B-2B 내부상태 전환은 후속.

목적: 지역을 기존 전역x 오프셋에서 분리하는 첫 통신 단위. 송신 avatar/move/enter/map은 region ID와 local좌표로 전달한다. 로그인 요청/초기정보에 명시적인 v2를 두고 크기·버전·지역/좌표 범위를 검증한다. 잘못된 위치는 송신하지 않고 수신자는 상태 변경 전에 거부한다.

현재 서버 GameObjectState와 클라 렌더러는 legacy좌표다. 서버 송신 직전 encode, 클라/GUIstress 수신 직후 검증된 복사본만 legacy로 변환한다. 콘솔stress는 v2 위치검증을 공유한다. 이 어댑터는 2B-2B canonical 상태 전환 때 제거할 임시 경계이며 이번이 전체 좌표 전환 완료는 아니다. DB X/Y codec/스키마/프로시저 변경 없음. 기존 캐릭터 위치는 그대로 복원/저장하므로 데이터 migration·rollback 불필요.

- AC-01: 두 지역 같은local좌표 구분/전칸왕복, invalid ID/음수/범위/구버전·구길이 거부. 크기 검증 후에만 body 접근. legacy adapter는 원래 수신 버퍼를 바꾸지 않는다.
- AC-02: 실제 생산 packet writer의 필드위치42,10은 SunnyPeak/10,10으로 전송. 지도chunk도local. DB대역 실제worker/TCP 포털 왕복·이전지역leave/시야·저장경계 회귀. 잘못된 로그인버전은 DB호출/생성/저장 없이 실패.
- AC-03: 일반클라/GUIstress/콘솔stress 함께빌드. 생산클라loopback에서 두지역 local좌표→기존배경/카메라/주민/본인이동 정상. 잘못된좌표/버전은 게임상태 변경 전 거부. 전체회귀와 관련렌더러출력 검사.
- AC-04: 서버·클라·기존stress x64 Debug/Release, 콘솔 빌드, 명세/결과/미리보기/버전배포·rollback 절차. 실제DB실행/300부하/일반exe실제OS창은 이번에 실행한 것만 별도 보고한다.

배포: 서버와 모든 클라/테스트 도구를 함께 교체하며 구/신 혼용은 지원하지 않는다. v1 요청크기는 framing에서 차단, v2크기에잘못된버전은로그인실패. old server 응답은 새 클라의 길이검사에서 차단. rollback은 전체 바이너리를 같은 이전커밋으로 빌드/교체, DB는그대로다.

제외: 지역크기확대/귀환버튼/새지역·개인집/DB region열/게임상태전면전환/과거stress크래시원인확정. C2S 공격의 기존 x,y는 서버가사용하지않는 비권위 호환필드이며 이번 좌표정보로해석하지않는다. 클라가목적지를정하는전송은추가하지않는다.

## 완료 기준과 실행 연결

| 기준 | 구현/실행 증거 |
| --- | --- |
| AC-01 | shared/protocol.hpp, legacy_packet_adapter.hpp / region_wire_test.cpp 두 지역 전칸 roundtrip·수신버퍼불변·잘못된지역/좌표/구버전·구길이·지도chunk경계 통과. |
| AC-02 | session_packets.cpp / session_packets_test.cpp 실제 writer의 SunnyPeak/10,10·지도local·invalid무송신 확인. 실제worker TCP 포털/시야/이전지역leave·DB대역 회귀 통과. 잘못된로그인버전에서 load/save0·같은소켓정상재시도 통과. |
| AC-03 | 생산client/GUIstress는 수신복사본만legacy변환, 콘솔은공유wire검증. 전체38회귀·생산클라loopback·잘못된버전→커마복귀/좌표·지도불변·주민회귀·화면/월드캐릭터 검사 통과. |
| AC-04 | 서버/일반클라/GUIstress x64 Debug/Release 6빌드+콘솔/W4 빌드 통과. 생산렌더러 v2/DBcodec 이미지 생성/육안 확인. 실제DB300명15초 혼합traceON 1회 통과(저장성공반환300/EOF300/송신오류0). |

실제 DB는 승인된900001000~900001299 재사용/저장. 전후두범위 COUNT300+300, 신규계정/삭제/스키마변경 없음. 120초 v2부하/일반exe 실제OS입력/모든이동응답지연·durable readback은 이번 미검증. 기존v1의120초결과를 v2실행결과로 재사용하지 않는다. [45단계](../refactoring-step-45.md), [실측 JSON](../test-results/region-wire-v2-2026-11-15.json).
