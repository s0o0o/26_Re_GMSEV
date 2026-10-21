#pragma once

constexpr int DEFAULT_LOAD_TEST_ATTEMPTS = 30;
constexpr int MAX_LOAD_TEST_ATTEMPTS = 10000;

// Invalid configuration falls back to a small, bounded test.
inline int parseLoadTestAttempts(const char* text) {
    if (!text || !*text) return DEFAULT_LOAD_TEST_ATTEMPTS;
    int value = 0;
    for (; *text; ++text) {
        if (*text < '0' || *text > '9') return DEFAULT_LOAD_TEST_ATTEMPTS;
        const int digit = *text - '0';
        if (value > (MAX_LOAD_TEST_ATTEMPTS - digit) / 10) return DEFAULT_LOAD_TEST_ATTEMPTS;
        value = value * 10 + digit;
    }
    return value > 0 ? value : DEFAULT_LOAD_TEST_ATTEMPTS;
}
