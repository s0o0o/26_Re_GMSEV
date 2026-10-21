#include "game_database.hpp"
#include "database_write.hpp"
#include <Windows.h>
#include <sqlext.h>
#include <iostream>
#include <cstdio>
#include <cwchar>
#include <cstdlib>
#include <mutex>

#pragma comment(lib, "odbc32.lib")

namespace {
std::mutex databaseMutex;
SQLHENV henv = nullptr;
SQLHDBC hdbc = nullptr;
void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];

	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

class Statement {
public:
    SQLHSTMT handle = SQL_NULL_HSTMT;
    Statement() {
        const auto result = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&handle);
        if (!SQL_SUCCEEDED(result)) {
            HandleDiagnosticRecord(hdbc,SQL_HANDLE_DBC,result);
            handle = SQL_NULL_HSTMT;
            return;
        }
        const auto timeoutResult = SQLSetStmtAttr(handle,SQL_ATTR_QUERY_TIMEOUT,
            reinterpret_cast<SQLPOINTER>(5),0);
        if (!SQL_SUCCEEDED(timeoutResult)) {
            std::cerr << "[DB] Cannot configure query timeout.\n";
            HandleDiagnosticRecord(handle,SQL_HANDLE_STMT,timeoutResult);
            SQLFreeHandle(SQL_HANDLE_STMT,handle);
            handle = SQL_NULL_HSTMT;
        }
    }
    ~Statement() { if (handle != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT,handle); }
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    bool check(SQLRETURN result, const char* step) {
        if (SQL_SUCCEEDED(result)) return true;
        std::cerr << "[DB] " << step << " failed.\n";
        HandleDiagnosticRecord(handle,SQL_HANDLE_STMT,result);
        return false;
    }
    bool prepare(const wchar_t* query) {
        return handle != SQL_NULL_HSTMT && check(SQLPrepareW(handle,
            reinterpret_cast<SQLWCHAR*>(const_cast<wchar_t*>(query)),SQL_NTS),"prepare");
    }
    template<class Value>
    bool bind(std::size_t index, Value& value) {
        static_assert(sizeof(Value) == sizeof(SQLINTEGER),"ODBC integer size mismatch");
        return check(SQLBindParameter(handle,static_cast<SQLUSMALLINT>(index),SQL_PARAM_INPUT,
            SQL_C_LONG,SQL_INTEGER,0,0,&value,0,nullptr),"bind parameter");
    }
    bool execute() { return check(SQLExecute(handle),"execute"); }
    bool affectedOneRow() {
        SQLLEN rows = -1;
        if (!check(SQLRowCount(handle,&rows),"row count")) return false;
        if (rows == 1) return true;
        std::cerr << "[DB] Expected one account row; affected rows=" << rows << '\n';
        return false;
    }
};

} // namespace

bool initializeDB() {
    std::lock_guard<std::mutex> guard(databaseMutex);
	SQLRETURN ret;

	ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) return false;

	ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) return false;

	ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) return false;

	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

	ret = SQLConnect(hdbc, (SQLWCHAR*)L"2025_gameserver_2023192003", SQL_NTS, NULL, 0, NULL, 0);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) return false;

    ret = SQLSetConnectAttr(hdbc,SQL_ATTR_AUTOCOMMIT,
        reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_ON),0);
    if (!SQL_SUCCEEDED(ret)) {
        HandleDiagnosticRecord(hdbc,SQL_HANDLE_DBC,ret);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV,henv);
        hdbc = nullptr;
        henv = nullptr;
        return false;
    }
	//std::cout << "[DB] 연결 성공!" << std::endl;
	return true;
}

void finalizeDB() {
    std::lock_guard<std::mutex> guard(databaseMutex);
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

// 로그인할때 정보 확인용
LoginLookupResult get_DB_login_info(int& db_id, int& db_user_x, int& db_user_y, int& db_user_hp,
    int& db_user_level, int& db_user_exp, int& db_user_seed_num, int& db_user_maxHp, int& db_user_damage) {
    std::lock_guard<std::mutex> guard(databaseMutex);
    Statement statement;
    if (statement.handle == SQL_NULL_HSTMT) return LoginLookupResult::Error;
    SQLRETURN ret;
    const auto fail = [&]() {
        HandleDiagnosticRecord(statement.handle, SQL_HANDLE_STMT, ret);
        return LoginLookupResult::Error;
    };
    ret = SQLPrepare(statement.handle, (SQLWCHAR*)L"EXEC get_user_info ?", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) return fail();
    ret = SQLBindParameter(statement.handle, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
        0, 0, &db_id, 0, nullptr);
    if (!SQL_SUCCEEDED(ret)) return fail();
    ret = SQLExecute(statement.handle);
    if (!SQL_SUCCEEDED(ret)) return fail();

    SQLINTEGER values[9] = {};
    SQLLEN indicators[9] = {};
    for (SQLUSMALLINT i = 0; i < 9; ++i) {
        ret = SQLBindCol(statement.handle, i + 1, SQL_C_LONG, &values[i], sizeof(values[i]), &indicators[i]);
        if (!SQL_SUCCEEDED(ret)) return fail();
    }
    ret = SQLFetch(statement.handle);
    if (ret == SQL_NO_DATA) return LoginLookupResult::NotFound;
    if (!SQL_SUCCEEDED(ret)) return fail();
    for (auto indicator : indicators) {
        if (indicator == SQL_NULL_DATA) {
            std::cerr << "[DB] Login row has a NULL required column.\n";
            return LoginLookupResult::Error;
        }
    }
    // Publish outputs only after the complete row has been validated.
    db_id = values[0];
    db_user_x = values[1];
    db_user_y = values[2];
    db_user_hp = values[3];
    db_user_level = values[4];
    db_user_exp = values[5];
    db_user_seed_num = values[6];
    db_user_maxHp = values[7];
    db_user_damage = values[8];
    return LoginLookupResult::Found;
}

bool update_user_info(int id, int x, int y, int seed, int hp, int level, int exp, int userMaxHp, int damage) {
    std::lock_guard<std::mutex> guard(databaseMutex);
    Statement statement;
    auto values = database_write::userValues(id,x,y,seed,hp,level,exp,userMaxHp,damage);
    return database_write::execute(statement,
        L"UPDATE SERVER_TABLE1 SET USER_X = ?, USER_Y = ?, USER_SEED_NUM = ?, USER_HP = ?, USER_LEVEL = ?, USER_EXP = ?, USER_MAX_HP = ?, USER_DAMAGE = ? WHERE USER_ID = ?",
        values);
}

bool insert_new_user(int userId, const char* /*name*/) {
    // Nickname persistence is not part of the existing account schema.
    std::lock_guard<std::mutex> guard(databaseMutex);
    Statement statement;
    std::array<SQLINTEGER,9> values = {{userId,rand()%2000,rand()%2000,rand()%10+20,1,0,22,30,1}};
    const bool saved = database_write::execute(statement,
        L"INSERT INTO SERVER_TABLE1 (USER_ID, USER_X, USER_Y, USER_HP, USER_LEVEL, USER_EXP, USER_SEED_NUM, USER_MAX_HP, USER_DAMAGE) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
        values);
    if (saved) std::cout << "[DB] Account created: " << userId << '\n';
    return saved;
}
