#include "../TP_2023192003_CLIENT/client_sample/login_flow.hpp"
#include <cassert>
int main() {
    LoginFlow flow;
    assert(flow.stage() == LoginStage::startScreen);
    assert(!flow.connect() && !flow.accept());
    assert(flow.finishCredentials());
    assert(flow.stage() == LoginStage::CustomizeScreen);
    assert(flow.back());
    assert(flow.finishCredentials());
    assert(flow.connect());
    assert(!flow.connect() && !flow.back());
    assert(flow.fail());
    assert(flow.stage() == LoginStage::CustomizeScreen);
    assert(flow.connect());
    assert(flow.accept());
    assert(flow.stage() == LoginStage::GameScreen);
    assert(!flow.fail() && !flow.finishCredentials());
}
