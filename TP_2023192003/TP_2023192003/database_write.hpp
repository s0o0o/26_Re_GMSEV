#pragma once
#include <array>
#include <cstdint>

namespace database_write {
// The order must match the UPDATE placeholders, with the account ID last.
inline std::array<std::int32_t, 9> userValues(int id, int x, int y, int seed,
    int hp, int level, int exp, int maxHp, int damage) {
    return {{x, y, seed, hp, level, exp, maxHp, damage, id}};
}

// Stop at the first failed ODBC step. Never execute partially bound statements.
// Exactly one account row must have been affected; unknown/zero counts are not success.
template<class Statement, class Query, class Value, std::size_t N>
bool execute(Statement& statement, Query query, std::array<Value,N>& values) {
    if (!statement.prepare(query)) return false;
    for (std::size_t i=0; i<N; ++i)
        if (!statement.bind(i+1,values[i])) return false;
    return statement.execute() && statement.affectedOneRow();
}
}
