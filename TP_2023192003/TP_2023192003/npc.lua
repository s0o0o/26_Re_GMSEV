-- Per-NPC policy. C++ owns collision, pathfinding, timers and network authority.
local myid = nil
local greeted = {}

function set_uid(id)
    myid = id
    greeted = {}
    math.randomseed(id)
end

-- One greeting for each connection, only while standing on this NPC's tile.
function event_player_move(player, connection)
    local px, py = API_get_x(player), API_get_y(player)
    local nx, ny = API_get_x(myid), API_get_y(myid)
    if px == nil or py == nil or nx == nil or ny == nil then return end
    if px == nx and py == ny and greeted[player] ~= connection then
        greeted[player] = connection
        API_SendMessage(myid, player, "HELLO")
    end
end

-- Existing C++ movement directions: 0 right, 1 left, 2 down, 3 up.
function choose_direction()
    return math.random(0, 3)
end
