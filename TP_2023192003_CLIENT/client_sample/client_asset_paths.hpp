#pragma once
#include <filesystem>
#include <initializer_list>
namespace client_assets {
namespace fs=std::filesystem;
inline bool complete(const fs::path& root) {
    std::error_code error;
    for(const auto* relative : {"neodgm.ttf","main.png","assets/characters/hamtaro/character.png","assets/characters/bijou/character.png","assets/maps/clubhouse-sheet.png","assets/maps/sunny-peak-sheet.png"}) {
        if(!fs::is_regular_file(root/relative,error) || error) return false;
    }
    return true;
}
inline fs::path locate(const fs::path& executable,const fs::path& originalCwd) {
    auto directory=executable.parent_path();
    for(int level=0;level<6 && !directory.empty();++level) {
        for(const auto& candidate : {directory,directory/"client_sample",directory/"TP_2023192003_CLIENT"/"client_sample"})
            if(complete(candidate)) return candidate;
        const auto parent=directory.parent_path(); if(parent==directory) break; directory=parent;
    }
    return complete(originalCwd) ? originalCwd : fs::path{};
}
}
