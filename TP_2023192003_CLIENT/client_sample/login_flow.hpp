#pragma once

enum class LoginStage { startScreen, CustomizeScreen, ConnectingScreen, GameScreen };
class LoginFlow {
public:
    LoginStage stage() const { return stage_; }
    bool finishCredentials() { return transition(LoginStage::startScreen, LoginStage::CustomizeScreen); }
    bool back() { return transition(LoginStage::CustomizeScreen, LoginStage::startScreen); }
    bool connect() { return transition(LoginStage::CustomizeScreen, LoginStage::ConnectingScreen); }
    bool fail() { return transition(LoginStage::ConnectingScreen, LoginStage::CustomizeScreen); }
    bool accept() { return transition(LoginStage::ConnectingScreen, LoginStage::GameScreen); }
private:
    bool transition(LoginStage from, LoginStage to) {
        if (stage_ != from) return false;
        stage_ = to;
        return true;
    }
    LoginStage stage_ = LoginStage::startScreen;
};
