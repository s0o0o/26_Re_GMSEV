#include "../shared/login_input.hpp"
#include <cassert>
#include <string>

int main() {
    std::int32_t id = -1;
    assert(login_input::accountId("321", id) && id == 321);
    assert(login_input::accountId("2147483647", id));
    assert(!login_input::accountId("2147483648", id));
    assert(!login_input::accountId("-1", id));
    assert(!login_input::accountId("Hamster", id));
    assert(!login_input::accountId("", id));
    assert(login_input::nickname("Hamster_321", 12));
    const char korean[] = "\xED\x96\x84\xEC\xB0\x8C";
    assert(login_input::nickname(korean, sizeof(korean)));
    assert(!login_input::nickname(korean, sizeof(korean) - 1));
    assert(!login_input::nickname("", 1));
    assert(!login_input::nickname("bad name", 9));
    assert(!login_input::nickname("\xC0\xAF", 3));
    assert(!login_input::nickname("\xED\xA0\x80", 4));
    assert(!login_input::nickname("\xE3\x84", 3));
    assert(!login_input::nickname("\xF4\x90\x80\x80", 5));
    std::string maximum(12, 'a');
    assert(login_input::nickname(maximum.c_str(), maximum.size() + 1));
    maximum += 'a';
    assert(!login_input::nickname(maximum.c_str(), maximum.size() + 1));
}
