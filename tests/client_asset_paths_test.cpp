#include "../TP_2023192003_CLIENT/client_sample/client_asset_paths.hpp"
#include <cassert>
#include <fstream>
#include <chrono>
#include <string>
int main() {
    namespace fs=std::filesystem;
    const auto workspace=fs::current_path();
    const auto root=workspace/"build/tests"/("asset-path-fixture-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    assert(!fs::exists(root)); fs::create_directories(root);
    const auto create=[&](const fs::path& directory) {
        for(const auto* file : {"neodgm.ttf","main.png","assets/characters/hamtaro/character.png","assets/characters/bijou/character.png","assets/maps/clubhouse-sheet.png","assets/maps/sunny-peak-sheet.png"}) {
            fs::create_directories((directory/file).parent_path()); std::ofstream(directory/file)<<"fixture";
        }
    };
    const auto repo=root/"repo",assets=repo/"TP_2023192003_CLIENT/client_sample",exe=repo/"TP_2023192003_CLIENT/x64/Release/client.exe";
    create(assets);
    assert(client_assets::locate(exe,root/"unrelated")==assets);
    const auto package=root/fs::path(L"\uD55C\uAE00")/"release";create(package);
    assert(client_assets::locate(package/"client.exe",assets)==package); // Exe-local assets win.
    const auto independent=root/"isolate/a/b/c/d/e/f/g/standalone";
    assert(client_assets::locate(independent/"client.exe",assets)==assets); // Existing VS working directory.
    fs::create_directories(independent);std::ofstream(independent/"neodgm.ttf")<<"partial";
    assert(!client_assets::complete(independent));
    assert(client_assets::locate(independent/"client.exe",root/"missing").empty());
    // Cleanup is limited to the uniquely created fixture under this workspace's build/tests.
    assert(root.parent_path()==workspace/"build/tests"); fs::remove_all(root);
}
