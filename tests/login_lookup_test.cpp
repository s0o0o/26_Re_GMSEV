#include "../TP_2023192003/TP_2023192003/game_database.hpp"
#include <cassert>

int main() {
    using R = LoginLookupResult;
    int reads = 0, creates = 0;
    auto create = [&]() { ++creates; return true; };
    assert(load_or_create_user([&]() { ++reads; return R::Found; }, create) == R::Found);
    assert(reads == 1 && creates == 0);
    reads = creates = 0;
    assert(load_or_create_user([&]() { ++reads; return R::Error; }, create) == R::Error);
    assert(reads == 1 && creates == 0);
    reads = creates = 0;
    assert(load_or_create_user([&]() { return ++reads == 1 ? R::NotFound : R::Found; }, create) == R::Found);
    assert(reads == 2 && creates == 1);
    reads = creates = 0;
    assert(load_or_create_user([&]() { ++reads; return R::NotFound; },
        [&]() { ++creates; return false; }) == R::Error);
    assert(reads == 1 && creates == 1);
    reads = creates = 0;
    assert(load_or_create_user([&]() { return ++reads == 1 ? R::NotFound : R::Error; }, create) == R::Error);
    assert(reads == 2 && creates == 1);
    reads = creates = 0;
    assert(load_or_create_user([&]() { ++reads; return R::NotFound; }, create) == R::NotFound);
    assert(reads == 2 && creates == 1);
}
