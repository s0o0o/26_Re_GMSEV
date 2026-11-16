# 월드·지역 좌표·DB 경계

갱신: 2026-11-16, #55. [구조 입구](README.md).

외형은 사용자 승인 별도 dbo.PLAYER_APPEARANCE에 저장한다. 위치/능력치 기존 테이블·프로시저와 구분하며 로그인 world 잠금 밖 read/저장 실패·복원 계약은 [옷장](wardrobe.md)에 정리했다. 기존900001299 외형1행 저장/새 서버 복원은 실제 DB 검사이며, 새 계정 생성은 없었다.

## 위치의 단일 원본

[GameObjectState](../../TP_2023192003/TP_2023192003/game_object.hpp)의 position은 `{region,x,y}`이며 x/y는 지역 내부 좌표다. location()은 이 값을 반환한다. 옛 전역 _x/_y를 따로 저장하지 않는다. 일반 클라의 OBJECT도 같은 Position 모델이다.

[world_regions.hpp](../../shared/world_regions.hpp)는 Id(Invalid/Clubhouse/SunnyPeak), 지역 descriptor, 좌표 범위, 같은 지역·거리 판정을 정의한다. 동일한 local(10,10)이어도 지역이 다르면 시야·전투·경로 대상이 아니다. fieldOrigin=32는 DB·기존 자료 호환의 오프셋이며, 런타임 위치에 더해 보관하지 않는다.

```text
기존 DB (42,10)
  → restoreLegacy → Position(SunnyPeak,10,10)
  → 서버 이동/시야/AI/섹터 → v5(region=2,x=10,y=10)
  → 클라 Position + local camera/render
종료 저장: Position → encodeLegacy → 기존 DB (42,10)
```

## 이동·맵·포털

- [myserver.cpp](../../TP_2023192003/TP_2023192003/myserver.cpp) processPacket(C2S_P_MOVE): 현재Position복사→방향한칸→playerCanMove(Position)→씨앗수집→portal(Position&)→객체위치/섹터/시야갱신→본인/주변응답. 클라가목적지지역을임의지정하지않는다.
- [world_layout.hpp](../../shared/world_layout.hpp): 지역별walkable/포털/local camera. Clubhouse local(10,14)→SunnyPeak(10,10), SunnyPeak(10,12)→Clubhouse(10,12). 도착칸은즉시반대포털이아니다.
- [world_map.cpp](../../TP_2023192003/TP_2023192003/world_map.cpp): #54 작업 브랜치에서 전체 map_data 배열을 제거했다. Position으로 필드16×16 지형 LRU(최대16청크) 또는 거점 충돌을 조회하며 mapMutex로 보호한다. 씨앗 deadline은 캐시 밖에 보관하여 퇴거가 수집 상태를 복원하지 않게 한다. legacy int API는 decode만 하고 배열 인덱스를 만들지 않는다.
- [field_content.hpp](../../shared/field_content.hpp): 현재작은필드의씨앗/몬스터원본좌표. 자료자체는legacy좌표이며position(Point)이명시어댑터다. monsterCanStand(Position)은local충돌과귀환통로보호를적용한다.
- [session_packets.cpp](../../TP_2023192003/TP_2023192003/session_packets.cpp): 지도는 Position카메라로18×4타일 조각4개를 보낸다. 이 네트워크 조각은 지형 캐시16×16청크와 다른 단위이며 wire형식을 추가 변경하지 않았다.

## 섹터·시야·전투

[region_sector_index.hpp](../../TP_2023192003/TP_2023192003/region_sector_index.hpp)의key는(region,local sector x/y)다. updateSector의옛좌표인수는무시하며현재location에서소속을계산한다. 한객체는한소속만갖는다. getNearBySessions는후보를얻고isCanSee/isMonSee가동일지역과거리를검사한다. 좌표가가깝다는이유로다른지역객체가보이지않는다.

공격은 서버의 위치·피해·씨앗·체력을 사용하고, 클라의 예약 x/y·damage 주장을 권위로 쓰지 않는다. Clubhouse는 비전투 지역이며 town_npcs 주민은 hostile 대상에서 제외된다. myserver.cpp의 공격 분기가 판정하고, world_events.hpp의 publishCombat은 지역 내 관찰자에게 결과를 전달한다.

[pathfinder.cpp](../../TP_2023192003/TP_2023192003/pathfinder.cpp)의 Position A*는 동일 지역만 허용하고 local 경로를 반환한다. MonsterState._nextMonX/Y도 현재 몬스터 지역의 local 다음 칸이다. int A*는 옛 테스트 호환 wrapper로만 유지한다.

## DB 계약과 수명

[game_database.cpp](../../TP_2023192003/TP_2023192003/game_database.cpp)는 ODBC 초기화·조회·생성·저장을 수행하며 databaseMutex로 직렬화한다. DSN·인증정보는 이 문서에 복사하지 않는다. [실측 메타데이터 계약](../database-contract.md)에 get_user_info의 9개 결과 열과 한계를 기록했다.

[database_contract.hpp](../../TP_2023192003/TP_2023192003/database_contract.hpp)는 열 순서·형식·NULL·잘림·요청 ID·다중 행 검사를 담당한다. **NotFound만 생성을 허용**하며 Error를 새 계정 생성으로 바꾸지 않는다. 유일 키의 확인 한계와 동시 생성 경쟁은 validator만으로 해결되지 않는다.

[database_position.hpp](../../TP_2023192003/TP_2023192003/database_position.hpp)는 restoreLegacy와 encodeLegacy를 담당한다. 기존 잘못된 위치 조회는 거점 복구 정책, 잘못된 지역 위치 저장은 거부 정책으로 구분한다. [game_database.hpp](../../TP_2023192003/TP_2023192003/game_database.hpp)의 update_user_position이 Position → 기존 X/Y의 최종 저장 경계다. DB region 열·프로시저 마이그레이션은 하지 않았다.

world 잠금 안에서 상태 스냅샷을 만들고 DB 호출은 world 밖에서 수행한다. 그래도 동기 호출이므로 worker는 그동안 기다린다. DB 풀·비동기 DB 큐가 이미 있다고 가정하지 않는다.

## 아직 남은 경계

#54: SunnyPeak 256×256, 결정적 지형/충돌·서버 LRU·A* 기본1024 확장 예산과 클라 가시 청크 렌더링을 연결했다. [청크 구조](field-chunks.md), [단계별 검증](../specs/field-chunks.md). 지형 기반 완료와 콘텐츠/레벨 아트 완성을 구분한다.

#53의 transitionRegion은 기존 포털과 명시적 귀환 요청을 공통 처리한다. [전환 흐름](travel.md). 새 DB 저장 형식이나 개인 집은 도입하지 않았다.

field_content/DB에는 legacy 호환이 남는다. #52에서 GUIstress도 region/local로 바뀌었고 #54에서 전체 map_data는 제거했다. 새 지역은 자료·충돌·활성범위·DB 표현을 함께 설계해야 한다. tests/legacy_position_fixture.hpp는 과거 시나리오를 보존하는 테스트 어댑터이며 생산 위치 필드가 아니다.
