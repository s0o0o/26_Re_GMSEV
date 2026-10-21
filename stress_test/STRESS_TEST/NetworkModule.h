#pragma once
#include <atomic>

void InitializeNetwork();
void GetPointCloud(int* size, float** points);

struct LoadTestStats {
    int attempts, limit, accepted, rejected, closed, errors;
};
LoadTestStats GetLoadTestStats();

extern std::atomic_int global_delay;
extern std::atomic_int active_clients;
