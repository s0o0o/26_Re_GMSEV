// Headless tests exercise the load tool's actual error handling, not a copy.
#define UNICODE
#define _UNICODE
#include "../stress_test/STRESS_TEST/NetworkModule.cpp"
#include <cassert>
HWND hWnd = nullptr;

int main() {
    WSADATA data{};
    assert(WSAStartup(MAKEWORD(2, 2), &data) == 0);
    active_clients = 0;
    g_clients[0].client_socket = INVALID_SOCKET;
    g_clients[0].socketOpen = true;
    sc_packet_login_fail fail{};
    fail.size = sizeof(fail);
    fail.type = S2C_P_LOGIN_FAIL;
    ProcessPacket(0, reinterpret_cast<unsigned char*>(&fail));
    assert(!g_clients[0].socketOpen && active_clients == 0);
    assert(HandleFailedCompletion(FALSE, 1, 0, nullptr));
    auto send = new OverlappedEx{};
    send->event_type = OP_SEND;
    assert(HandleFailedCompletion(FALSE, 10, 0, send));
    // Worker must continue immediately after the handled/deleted completion.
    OverlappedEx recv{};
    recv.event_type = OP_RECV;
    assert(HandleFailedCompletion(TRUE, 0, 0, &recv));
    assert(!HandleFailedCompletion(TRUE, 10, 0, &recv));
    assert(HandleFailedCompletion(FALSE, 0, -1, &recv));
    g_clients[0].socketOpen = true;
    sc_packet_avatar_info info{};
    info.type = S2C_P_AVATAR_INFO;
    info.id = 2;
    ProcessPacket(0, reinterpret_cast<unsigned char*>(&info));
    ProcessPacket(0, reinterpret_cast<unsigned char*>(&info));
    assert(active_clients == 1);
    DisconnectClient(0);
    DisconnectClient(0);
    assert(active_clients == 0);
    info.id = -1;
    ProcessPacket(0, reinterpret_cast<unsigned char*>(&info));
    assert(active_clients == 0);
    const auto stats = GetLoadTestStats();
    assert(stats.accepted == 1 && stats.rejected == 1);
    assert(stats.closed == 2 && stats.errors >= 1);
    attempt_limit = 30;
    num_connections = 30;
    Adjust_Number_Of_Client();
    assert(num_connections == 30);
    attempt_limit = 100;
    num_connections = 100;
    Adjust_Number_Of_Client();
    assert(num_connections == 100);
    WSACleanup();
}
