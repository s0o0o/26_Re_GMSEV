#pragma once

// Database API: callers do not depend on ODBC headers or handle types.
bool initializeDB();
void finalizeDB();
enum class LoginLookupResult { Found, NotFound, Error };

// Only an explicitly missing row permits account creation.
template<class Lookup, class Create>
LoginLookupResult load_or_create_user(Lookup lookup, Create create) {
    const auto result = lookup();
    if (result != LoginLookupResult::NotFound) return result;
    if (!create()) return LoginLookupResult::Error;
    return lookup();
}

LoginLookupResult get_DB_login_info(int& db_id, int& db_user_x, int& db_user_y,
    int& db_user_hp, int& db_user_level, int& db_user_exp,
    int& db_user_seed_num, int& db_user_maxHp, int& db_user_damage);
bool update_user_info(int id, int x, int y, int seed, int hp,
    int level, int exp, int userMaxHp, int damage);
bool insert_new_user(int userId, const char* name);
