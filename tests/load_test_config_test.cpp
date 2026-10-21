#include "../stress_test/STRESS_TEST/load_test_config.hpp"
#include <cassert>

int main() {
    assert(parseLoadTestAttempts(nullptr) == 30);
    assert(parseLoadTestAttempts("") == 30);
    assert(parseLoadTestAttempts("30") == 30);
    assert(parseLoadTestAttempts("100") == 100);
    assert(parseLoadTestAttempts("1") == 1);
    assert(parseLoadTestAttempts("10000") == 10000);
    assert(parseLoadTestAttempts("10001") == 30);
    assert(parseLoadTestAttempts("0") == 30);
    assert(parseLoadTestAttempts("-1") == 30);
    assert(parseLoadTestAttempts("abc") == 30);
    assert(parseLoadTestAttempts("100x") == 30);
    assert(parseLoadTestAttempts("99999999999999999999") == 30);
}
