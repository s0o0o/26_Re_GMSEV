# 정상 송신 완료 경합 완화

2026-11-15, 기준a2a195a, 작업[#46](https://github.com/s0o0o/26_Re_GMSEV/issues/46)/부모#41. 상태: 구현·제한 범위 검증 완료.

OP_SEND 정상 완료는 operation 해제 외 작업이 없지만 worker가 completionMutex를 획득한 뒤 분기한다. DB/월드 처리로 그 mutex를 잡은 RECV 뒤에 많은 worker가 대기할 수 있다. 소유권은 operation.shared_ptr가 유지하므로 정상 전체 바이트 송신은 게임 상태를 건드리지 않고 해제할 수 있다.

- AC-01: OP_SEND/owner 존재/key 일치를 먼저 확인한다. 살아있는연결의정상·양수·전체길이/transportFailed=false 송신, 또는 이미disconnectStarted=true로정리담당이정해진SEND만수명해제로끝낸다(중간범위보완참조). registry로 owner를 다시 찾지 않는다. 그외실패/부분/0바이트/불일치는기존직렬화경로를유지한다. 살아있는연결의부분송신은묵시적성공대신disconnect한다(부분재전송은별도).
- AC-02: 실제 worker에서 다른 thread가 completionMutex를 잡은 채 성공SEND를 게시해도 worker가 다음 제어작업을 처리함을 확인한다. owner 수명/슬롯 재사용/실패 경로/부분 송신의 분류를 단위·통합 회귀로 검증한다. 동기화 보호를 제거하는 일반 fast path로 확장하지 않는다.
- AC-03: 전체 회귀와 서버Debug/Release·기존stress 빌드. 같은 승인300계정을 재사용한 실제 DB 혼합 부하를 동일45초/15초/5초 조건, trace 활성/비활성으로 비교한다. 컴파일/회귀와 실측은 동시에 돌리지 않는다. baseline·실패도 보존하고 trace sample은 낮은 빈도로 기록한다.
- AC-04: 실제 결과/미검증/미리보기·설명·다음 작업을 이슈/문서에 연결한다. 개선이 확인돼도 전체 장시간 안정성이나 과거 크래시 원인 확정으로 확대하지 않는다. 부모#41의 종료 기준은 별도로 판단한다.

world 잠금/DB/패킷/클라/리소스/timeout 변경은 제외. 기존CP949 보존. 실제 로그인/종료는 승인범위900001000~900001299의 데이터를 저장하며 추가계정/삭제/스키마 변경은 하지 않는다.

중간 범위 보완: 첫수정의실제혼합은300로그인/15초유지까지개선됐으나종료timeout300은남았다. 이미disconnectStarted=true인owner의SEND도기존classifier가Ignore할항목인데같은mutex를기다린다. 해당owner/key가확인된SEND는종료플래그가이미설정됐으면성공/실패/부분여부와무관하게객체해제만허용한다. transportFailed=true이지만disconnectStarted=false면기존cleanup경로를유지한다. RECV/LOGIN에는확장하지않는다. 종료담당은기존exchange(true)가한명으로유지하며다시저장/종료하지않는다.

## 실행 결과 (2026-11-15)

- AC-01 코드/실행 확인: `releaseSendCompletionWithoutLock`와 worker의 RAII operation 해제 경로. object ID가할당후변경되지않음을검색확인. flag/key/owner/type/full/partial/error/closed 경계단위검사통과. 닫히기시작한SEND만예외로해제하며수신조각·상태·DB를건드리지않는다.
- AC-02 실행 통과: 실제worker1개에서owner잠금을잡은채full SEND와이미종료중0바이트SEND를post하고,잠금해제전두번째probe가완료됨을검사. 원래owner의weak참조소멸·부분SEND의disconnect도검사했다. OS TCP부분전송을강제로만든것은아니다.
- AC-03 실행 통과: 전체회귀36개,별도worker검사,서버Debug/Release와기존stress Debug/Release 빌드. 기존C4244/LNK4098경고잔존. baseline실제DB traceON은294로그인/로그인timeout1/종료timeout299. 첫수정은300유지성공이나종료timeout300. 최종수정은traceON/OFF/ON 3회모두300로그인/15초유지/EOF300/클라이언트오류0/서버송신오류0. 제한45/15/5초그대로,컴파일/회귀와실측동시실행없음.
- AC-04 [42단계 설명](../refactoring-step-42.md)·[실측 JSON](../test-results/send-fast-2026-11-15.json) 연결. 5실행비교PNG 생성·육안확인(측정도표,게임캡처아님). #46은완료하되부모#41은수정후신규계정생성·추가장시간조건을남겨P2로유지하고다음#19 실제클라확인으로진행한다.

최종traceON 두실행의로그인완료큐최대119/71ms(같은날baseline10797ms),수신→ready최대962/944ms. 유지종료시미응답이동74/69/4개는클라버퍼·네트워크·서버처리와측정시점을포함하며서버큐길이가아니다. 종료drain은545/637/376ms. traceON에서종료저장성공반환각300/추가송신완료빠른경로표본하한1818624/1716224. 전체I/O drain/readback/장시간/새계정생성재검증은아니다.

초기테스트패치는기존공백불일치로적용전실패해정확한행으로수정했다. 첫정상SEND만최적화한실험의실패도보존했다. 원본CP949를보존했고문서/새헤더는UTF8. 기존600계정미삭제,일반계정·스키마변경없음. bounded 반복 통과를최대수용량이나과거#10/#19의동일원인확정으로확대하지않는다.
