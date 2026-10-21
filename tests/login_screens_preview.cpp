// Render the actual client's login/customization screens without connecting to a server.
#define main hamster_game_main
#include "../TP_2023192003_CLIENT/client_sample/client.cpp"
#undef main
#include <cassert>
#include <filesystem>

int main() {
    std::cout << "Preparing login screen assets" << std::endl;
    const auto root = std::filesystem::current_path();
    std::filesystem::current_path(root / "TP_2023192003_CLIENT/client_sample");
    assert(gameFont().loadFromFile("neodgm.ttf"));
    for (const auto& value : {sf::String("Bijou"), sf::String("Connecting..."), sf::String(L"\uD584\uCC0C")}) {
        sf::Text centered(value,gameFont(),22);
        const sf::FloatRect area(190,610,230,52);
        centerText(centered,area);
        const auto bounds = centered.getGlobalBounds();
        assert(std::abs(bounds.left+bounds.width/2-(area.left+area.width/2)) < 0.01f);
        assert(std::abs(bounds.top+bounds.height/2-(area.top+area.height/2)) < 0.01f);
    }
    initStartScreenInit();
    assert(hamsterCustomizer().load("assets/characters"));
    assert(hamsterCustomizer().setBackdrop(*startMainTx));
    std::cout << "Backdrop prepared" << std::endl;
    sf::RenderTexture frame;
    assert(frame.create(900,800));
    id_input = 321;
    inputNickname = sf::String(L"\uD584\uCC0C");
    inputAccountId = "321";
    choosingNickname = true;
    frame.clear();
    drawStartScreen(frame,inputNickname);
    frame.display();
    assert(frame.getTexture().copyToImage().saveToFile((root / "build/tests/login-credentials.png").string()));
    std::cout << "Credentials rendered" << std::endl;
    assert(loginFlow.finishCredentials());
    frame.clear();
    drawCustomizationScreen(frame);
    frame.display();
    assert(frame.getTexture().copyToImage().saveToFile((root / "build/tests/customization-flow.png").string()));
    std::cout << "Customization rendered" << std::endl;
    sf::Event choose{};
    choose.type = sf::Event::KeyPressed;
    choose.key.code = sf::Keyboard::F5;
    sf::RenderWindow inputWindow;
    hamsterCustomizer().handleEvent(choose,inputWindow);
    assert(hamsterCustomizer().selectedCharacter() == 1);
    frame.clear();
    drawCustomizationScreen(frame);
    frame.display();
    assert(frame.getTexture().copyToImage().saveToFile((root / "build/tests/bijou-login-flow.png").string()));
    assert(loginFlow.connect());
    frame.clear();
    drawCustomizationScreen(frame);
    frame.display();
    assert(frame.getTexture().copyToImage().saveToFile((root / "build/tests/login-connecting.png").string()));
    failLogin("Offline test: retry is available.");
    assert(loginFlow.stage() == GAMESTATE::CustomizeScreen);
    assert(hamsterCustomizer().selectedCharacter() == 1);
    assert(inputNickname == sf::String(L"\uD584\uCC0C"));
    assert(s_socket.getRemotePort() == 0);
    assert(!gameResourcesReady);
    std::filesystem::current_path(root);
    std::cout << "Login screen previews passed" << std::endl;
}
