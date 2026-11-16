#pragma once
#include "../../shared/appearance.hpp"
#include <atomic>

// Gameplay depends on this boundary, not ODBC handles. Before initializeDB()
// installs the validated SQL adapter, the default refuses Save and writes nothing.
namespace appearance_storage {
enum class Status { Saved, Missing, Found, Unavailable, Error };
struct Read { Status status=Status::Unavailable; avatar_appearance::Value value; };
class Store {
public:
    virtual ~Store()=default;
    virtual Read read(int account)=0;
    virtual Status save(int account,avatar_appearance::Value value)=0;
};
class Unconfigured final : public Store {
public:
    Read read(int) override { return {}; }
    Status save(int,avatar_appearance::Value) override { return Status::Unavailable; }
};
inline Unconfigured unconfigured;
// Install before serving requests; test adapters outlive every worker.
inline std::atomic<Store*> active{&unconfigured};
}
