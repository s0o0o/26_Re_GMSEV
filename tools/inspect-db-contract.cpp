#include <Windows.h>
#include <sql.h>
#include <sqlext.h>
#include <iostream>
#pragma comment(lib,"odbc32.lib")

// Metadata-only diagnostic: never calls the user procedure or selects account rows.
namespace {
void state(SQLSMALLINT type,SQLHANDLE handle) {
    SQLWCHAR code[6]{},message[1024]{}; SQLINTEGER native=0; SQLSMALLINT length=0;
    if(SQL_SUCCEEDED(SQLGetDiagRecW(type,handle,1,code,&native,message,1024,&length)))
        std::wcout<<L"ODBC state="<<reinterpret_cast<wchar_t*>(code)<<L" native="<<native<<L" (connection details omitted)\n";
}
struct Connection {
    SQLHENV env=SQL_NULL_HENV; SQLHDBC db=SQL_NULL_HDBC; bool connected=false;
    ~Connection() { if(connected) SQLDisconnect(db); if(db) SQLFreeHandle(SQL_HANDLE_DBC,db); if(env) SQLFreeHandle(SQL_HANDLE_ENV,env); }
    bool open(wchar_t* dsn) {
        if(!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&env)) ||
           !SQL_SUCCEEDED(SQLSetEnvAttr(env,SQL_ATTR_ODBC_VERSION,reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3),0)) ||
           !SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC,env,&db))) return false;
        if(!SQL_SUCCEEDED(SQLSetConnectAttr(db,SQL_ATTR_LOGIN_TIMEOUT,reinterpret_cast<SQLPOINTER>(5),0)) ||
           !SQL_SUCCEEDED(SQLSetConnectAttr(db,SQL_ATTR_ACCESS_MODE,reinterpret_cast<SQLPOINTER>(SQL_MODE_READ_ONLY),0))) return false;
        if(!SQL_SUCCEEDED(SQLConnectW(db,reinterpret_cast<SQLWCHAR*>(dsn),SQL_NTS,nullptr,0,nullptr,0))) { state(SQL_HANDLE_DBC,db); return false; }
        connected=true; return true;
    }
};
bool query(SQLHDBC db,const wchar_t* label,const wchar_t* sql,bool allowEmpty=false) {
    SQLHSTMT stmt=SQL_NULL_HSTMT;
    if(!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT,db,&stmt))) return false;
    struct Cleanup { SQLHSTMT s; ~Cleanup(){SQLFreeHandle(SQL_HANDLE_STMT,s);} } cleanup{stmt};
    if(!SQL_SUCCEEDED(SQLSetStmtAttr(stmt,SQL_ATTR_QUERY_TIMEOUT,reinterpret_cast<SQLPOINTER>(5),0)) ||
       !SQL_SUCCEEDED(SQLExecDirectW(stmt,reinterpret_cast<SQLWCHAR*>(const_cast<wchar_t*>(sql)),SQL_NTS))) { state(SQL_HANDLE_STMT,stmt); return false; }
    SQLSMALLINT columns=0; if(!SQL_SUCCEEDED(SQLNumResultCols(stmt,&columns))) return false;
    std::wcout<<L"["<<label<<L"]\n"; int rows=0;
    for(;;) {
        const auto fetched=SQLFetch(stmt); if(fetched==SQL_NO_DATA) break;
        if(!SQL_SUCCEEDED(fetched)) { state(SQL_HANDLE_STMT,stmt); return false; }
        ++rows;
        for(SQLUSMALLINT i=1;i<=columns;++i) {
            SQLWCHAR value[512]{}; SQLLEN length=0;
            if(SQLGetData(stmt,i,SQL_C_WCHAR,value,sizeof(value),&length)!=SQL_SUCCESS) return false;
            std::wcout<<(i==1 ? L"" : L" | ")<<(length==SQL_NULL_DATA ? L"NULL" : reinterpret_cast<wchar_t*>(value));
        }
        std::wcout<<L'\n';
    }
    std::wcout<<L"metadata_rows="<<rows<<L'\n';
    if(rows==0 && allowEmpty) std::wcout<<L"WARNING: no key columns visible; uniqueness is not confirmed.\n";
    return rows>0 || allowEmpty;
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=2) { std::cerr<<"Usage: inspect-db-contract <project DSN>; metadata only\n"; return 2; }
    Connection connection;
    if(!connection.open(argv[1])) { std::cerr<<"Metadata connection failed; no accounts read or written.\n"; return 1; }
    const bool table=query(connection.db,L"SERVER_TABLE1 columns",L"SELECT c.column_id,c.name,TYPE_NAME(c.user_type_id),c.max_length,c.is_nullable FROM sys.columns c WHERE c.object_id=OBJECT_ID(N'SERVER_TABLE1') ORDER BY c.column_id");
    const bool parameters=query(connection.db,L"get_user_info parameters",L"SELECT parameter_id,name,TYPE_NAME(user_type_id),max_length,is_output FROM sys.parameters WHERE object_id=OBJECT_ID(N'get_user_info') ORDER BY parameter_id");
    const bool keys=query(connection.db,L"SERVER_TABLE1 key columns",L"SELECT i.name,i.is_unique,k.key_ordinal,c.name FROM sys.indexes i JOIN sys.index_columns k ON i.object_id=k.object_id AND i.index_id=k.index_id JOIN sys.columns c ON c.object_id=k.object_id AND c.column_id=k.column_id WHERE i.object_id=OBJECT_ID(N'SERVER_TABLE1') AND k.key_ordinal>0 ORDER BY i.index_id,k.key_ordinal",true);
    const bool results=query(connection.db,L"get_user_info result contract",L"SELECT column_ordinal,name,system_type_name,is_nullable FROM sys.dm_exec_describe_first_result_set_for_object(OBJECT_ID(N'get_user_info'),0) WHERE is_hidden=0 ORDER BY column_ordinal");
    std::cout<<"Only metadata SELECTs executed; no procedure invocation, account data, DDL, INSERT or UPDATE.\n";
    return table && parameters && keys && results ? 0 : 1;
}
