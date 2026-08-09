myid = 99999;       -- NPC의 고유 ID 저장하는 변수..
dir = 0;
move_cnt = 0;

-- 서버에서 npc 초기화 할때.. 이 함수 호출시켜서 myid설정함
function set_uid(x)
   myid = x;
end

-- 플레이어가.. npc 근처에 접근했을때 호출되는 이벤트 핸들러?
function event_player_move(player)
   player_x = API_get_x(player);
   player_y = API_get_y(player);
   my_x = API_get_x(myid);
   my_y = API_get_y(myid);
   if (player_x == my_x) then
      if (player_y == my_y) then
         API_SendMessage(myid, player, "HELLO");
           -- 이거 지금 타이머 요청하는거임..
           -- 왜냐면 lua에서 타이머 접근은 복잡하고 위험하기에..   
           -- c++한테 타이머를 '요청'하는거임
         ---- 랜덤 방향 선택 (0:위, 1:아래, 2:왼쪽, 3:오른쪽)
         move_cnt = 0;
         dir = math.random(0, 3)
      end
   end
end

-- 3칸 랜덤으로 도망가는 거
function event_npc_run(player)
    if(move_cnt < 2) then
        API_SendMessage(myid, player, "RUNRUN");
        move_cnt = move_cnt + 1
        return 0
    elseif (move_cnt == 2) then
        API_SendMessage(myid, player, "BYEBYE")
        move_cnt=0;
        return 1
    end
end
