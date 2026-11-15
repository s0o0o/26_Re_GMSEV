# GUI IOCP 부하 도구 안전 실행·단계 재현

2026-11-16 기준91adbf2 / #52, 부모#10. 현재범위 검증 완료, 과거원인은미확정.

- AC-01: GUI클라 Position(region/local) 단일원본, packet검증/같은연결위치갱신. 화면/ID매핑/소켓submit-close를공유잠금으로보호한다.
- AC-02: 실제DB쓰기는명시확인환경변수와승인된시작ID(900000000/900001000) 없으면접속하지않는다. GUI시도는1~300으로제한하며기존일반계정0시작을제거한다.
- AC-03: producer중지→소켓종료→pending I/O drain→worker정지/해제. 최대10초drain초과는실패로보고하고미완료buffer를일찍해제하지않는다. 자동검사모드전체60초제한/전원접속후15초유지(성공하려면45초안에전원접속),소켓취소오류와유지중오류구분. TCP수신EOF까지half-close drain하는검사는아니다.
- AC-04: 모의오류/위치·설정/IOCP종료회귀와Debug/Release빌드. 실제기존GUIexe 30→100→300명 각15초/1회순차검사. 승인된기존계정만재사용하며전후COUNT확인. 렌더링창존재/결과로그/종료코드기록,크래시면예외코드/가능한진단로그보존. 이번에미재현이면과거원인확정없이한계를기록한다.
- AC-05: architecture와작업큐·이슈·실측JSON/미리보기갱신. 일반콘솔도구결과와GUI실행을구분한다.

서버/DBschema변경·신규계정·과거원인확정/장시간보증은제외. GUI의자가종료검사모드는테스트용정상앱로직이며실제마우스/키보드검증은아니다.

## 완료 기준 연결

- AC-01: CLIENT.position/ownMOVE·stateMutex/매핑해제,stress_error_test의local위치·원본packet·invalid거부회귀 통과.
- AC-02: gui_load_options.hpp의확인/전용ID/1~300검사와GUI기동전COUNT300+300 통과. 미승인/잘못된범위거부단위검사.
- AC-03: mock IOCP의두worker정지검사와실제각단계pending0/drained1·종료0. 강제pending timeout주입은미검증. 종료취소는유지중오류와분리.
- AC-04: 최종39회귀/GUI Debug·Release 통과. 실제GUI30/100/300 각15초최종1회 모두로그인·이동응답전원/오류0/종료0/scene·swap확인. 서버저장30/100/300성공반환. 종료후서버오류는별도기록. 과거크래시미재현이지최초원인확정아님.
- AC-05: [48단계](../refactoring-step-48.md),[결과JSON](../test-results/gui-load-2026-11-16.json),architecture/gui-stress.md와작업큐갱신. 미리보기는실측도표이며OS/게임캡처아님.
