#include "../TP_2023192003/TP_2023192003/appearance_odbc.hpp"
#include <iostream>
#pragma comment(lib,"odbc32.lib")

namespace {
struct Connection {
    SQLHENV env=SQL_NULL_HENV; SQLHDBC db=SQL_NULL_HDBC; bool connected=false;
    ~Connection() { if(connected) SQLDisconnect(db); if(db) SQLFreeHandle(SQL_HANDLE_DBC,db); if(env) SQLFreeHandle(SQL_HANDLE_ENV,env); }
    bool open(wchar_t* dsn,bool apply) {
        if(!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&env))
           || !SQL_SUCCEEDED(SQLSetEnvAttr(env,SQL_ATTR_ODBC_VERSION,reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3),0))
           || !SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC,env,&db))) return false;
        if(!SQL_SUCCEEDED(SQLSetConnectAttr(db,SQL_ATTR_LOGIN_TIMEOUT,reinterpret_cast<SQLPOINTER>(5),0))
           || !SQL_SUCCEEDED(SQLSetConnectAttr(db,SQL_ATTR_ACCESS_MODE,reinterpret_cast<SQLPOINTER>(static_cast<std::uintptr_t>(apply ? SQL_MODE_READ_WRITE : SQL_MODE_READ_ONLY)),0))) return false;
        connected=SQL_SUCCEEDED(SQLConnectW(db,reinterpret_cast<SQLWCHAR*>(dsn),SQL_NTS,nullptr,0,nullptr,0));
        return connected;
    }
};
bool scalar(SQLHDBC db,const wchar_t* sql,int& value) {
    SQLHSTMT s=SQL_NULL_HSTMT;
    if(SQLAllocHandle(SQL_HANDLE_STMT,db,&s)!=SQL_SUCCESS) return false;
    struct Cleanup { SQLHSTMT s; ~Cleanup(){SQLFreeHandle(SQL_HANDLE_STMT,s);} } cleanup{s};
    if(SQLSetStmtAttr(s,SQL_ATTR_QUERY_TIMEOUT,reinterpret_cast<SQLPOINTER>(5),0)!=SQL_SUCCESS
       || SQLExecDirectW(s,reinterpret_cast<SQLWCHAR*>(const_cast<wchar_t*>(sql)),SQL_NTS)!=SQL_SUCCESS
       || SQLFetch(s)!=SQL_SUCCESS) return false;
    SQLINTEGER result=0; SQLLEN length=0;
    if(SQLGetData(s,1,SQL_C_LONG,&result,sizeof(result),&length)!=SQL_SUCCESS || length!=sizeof(result)) return false;
    value=result; return true;
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3 || (std::wcscmp(argv[2],L"Inspect") && std::wcscmp(argv[2],L"Apply"))) return 2;
    const bool apply=std::wcscmp(argv[2],L"Apply")==0;
    Connection connection;
    if(!connection.open(argv[1],apply)) { std::cerr<<"Appearance DB connection unavailable (details omitted).\n"; return 1; }
    int exists=0;
    if(!scalar(connection.db,L"SELECT CASE WHEN OBJECT_ID(N'dbo.PLAYER_APPEARANCE',N'U') IS NULL THEN 0 ELSE 1 END",exists)) return 1;
    std::mutex mutex; appearance_storage::OdbcStore store(connection.db,mutex);
    if(exists) {
        const bool ready=store.schemaReadyUnlocked();
        std::cout<<"Appearance table present; schema="<<(ready ? "valid" : "MISMATCH")<<". No schema/data writes executed.\n";
        return ready ? 0 : 1;
    }
    if(!apply) { std::cout<<"Appearance table absent. Metadata/empty schema probes only; no writes executed.\n"; return 0; }
    int created=0;
    if(!scalar(connection.db,appearance_sql::create,created) || created!=1 || !store.schemaReadyUnlocked()) {
        std::cerr<<"Appearance CREATE/schema verification failed. No ALTER/DROP/account writes requested.\n"; return 1;
    }
    std::cout<<"Created dbo.PLAYER_APPEARANCE with 5 columns, USER_ID primary key and range constraints. Schema verified; no account rows created/changed.\n";
    return 0;
}
