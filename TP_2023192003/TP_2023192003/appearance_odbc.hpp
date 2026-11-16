#pragma once
#include "appearance_storage.hpp"
#include "appearance_sql.hpp"
#include "appearance_row.hpp"
#include <Windows.h>
#include <sqlext.h>
#include <array>
#include <mutex>
#include <cwchar>

namespace appearance_storage {
class OdbcStore final : public Store {
    SQLHDBC& connection_;
    std::mutex& mutex_;
    struct Query {
        SQLHSTMT handle=SQL_NULL_HSTMT;
        explicit Query(SQLHDBC db) {
            if(!db || !SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT,db,&handle))) return;
            if(!SQL_SUCCEEDED(SQLSetStmtAttr(handle,SQL_ATTR_QUERY_TIMEOUT,reinterpret_cast<SQLPOINTER>(5),0))) {
                SQLFreeHandle(SQL_HANDLE_STMT,handle); handle=SQL_NULL_HSTMT;
            }
        }
        ~Query() { if(handle) SQLFreeHandle(SQL_HANDLE_STMT,handle); }
        bool prepare(const wchar_t* sql) {
            return handle && SQLPrepareW(handle,reinterpret_cast<SQLWCHAR*>(const_cast<wchar_t*>(sql)),SQL_NTS)==SQL_SUCCESS;
        }
        bool bind(int index,SQLINTEGER& value) {
            return SQLBindParameter(handle,static_cast<SQLUSMALLINT>(index),SQL_PARAM_INPUT,SQL_C_LONG,SQL_INTEGER,0,0,&value,0,nullptr)==SQL_SUCCESS;
        }
    };
    static bool integer(SQLHSTMT s,int column,SQLINTEGER& value) {
        SQLLEN length=0;
        return SQLGetData(s,static_cast<SQLUSMALLINT>(column),SQL_C_LONG,&value,sizeof(value),&length)==SQL_SUCCESS
            && length==sizeof(value);
    }
public:
    OdbcStore(SQLHDBC& db,std::mutex& mutex) : connection_(db),mutex_(mutex) {}
    // Startup already holds the shared DB mutex. This does not run any DDL.
    bool schemaReadyUnlocked() {
        Query q(connection_);
        if(!q.prepare(appearance_sql::probe) || SQLExecute(q.handle)!=SQL_SUCCESS) return false;
        SQLSMALLINT count=0;
        if(SQLNumResultCols(q.handle,&count)!=SQL_SUCCESS || count!=5) return false;
        const wchar_t* names[]={L"USER_ID",L"CHARACTER_KIND",L"FUR_COLOR",L"EYE_STYLE",L"SCARF_STYLE"};
        for(int i=0;i<5;++i) {
            SQLWCHAR name[64]{}; SQLSMALLINT length=0,type=0,scale=0,nullable=0; SQLULEN width=0;
            if(SQLDescribeColW(q.handle,static_cast<SQLUSMALLINT>(i+1),name,64,&length,&type,&width,&scale,&nullable)!=SQL_SUCCESS
                || length<1 || length>=64 || std::wcscmp(reinterpret_cast<wchar_t*>(name),names[i])
                || type!=(i==0 ? SQL_INTEGER : SQL_TINYINT) || nullable!=SQL_NO_NULLS) return false;
        }
        SQLCloseCursor(q.handle);
        if(!q.prepare(L"SELECT COUNT(*),SUM(CASE WHEN c.name=N'USER_ID' AND k.key_ordinal=1 THEN 1 ELSE 0 END) FROM sys.indexes i JOIN sys.index_columns k ON i.object_id=k.object_id AND i.index_id=k.index_id JOIN sys.columns c ON c.object_id=k.object_id AND c.column_id=k.column_id WHERE i.object_id=OBJECT_ID(N'dbo.PLAYER_APPEARANCE') AND i.is_primary_key=1 AND k.key_ordinal>0")
            || SQLExecute(q.handle)!=SQL_SUCCESS || SQLFetch(q.handle)!=SQL_SUCCESS) return false;
        SQLINTEGER keys=0,idKeys=0;
        return integer(q.handle,1,keys) && integer(q.handle,2,idKeys) && keys==1 && idKeys==1;
    }
    Read read(int account) override {
        if(account<0) return {Status::Error,{}};
        std::lock_guard<std::mutex> lock(mutex_);
        Query q(connection_); SQLINTEGER id=account;
        if(!q.prepare(appearance_sql::read) || !q.bind(1,id) || SQLExecute(q.handle)!=SQL_SUCCESS) return {Status::Error,{}};
        const auto fetched=SQLFetch(q.handle);
        if(fetched==SQL_NO_DATA) return {Status::Missing,{}};
        if(fetched!=SQL_SUCCESS) return {Status::Error,{}};
        std::array<SQLINTEGER,5> values{};
        for(int i=0;i<5;++i) if(!integer(q.handle,i+1,values[i])) return {Status::Error,{}};
        std::array<std::int32_t,5> row{};
        for(int i=0;i<5;++i) row[i]=static_cast<std::int32_t>(values[i]);
        avatar_appearance::Value value;
        if(!decodeRow(account,row,value) || SQLFetch(q.handle)!=SQL_NO_DATA) return {Status::Error,{}};
        return {Status::Found,value};
    }
    Status save(int account,avatar_appearance::Value value) override {
        if(account<0 || !avatar_appearance::valid(value)) return Status::Error;
        std::lock_guard<std::mutex> lock(mutex_);
        Query q(connection_);
        std::array<SQLINTEGER,5> values{{account,value.kind,value.fur,value.eyes,value.scarf}};
        if(!q.prepare(appearance_sql::save)) return Status::Error;
        for(int i=0;i<5;++i) if(!q.bind(i+1,values[i])) return Status::Error;
        if(SQLExecute(q.handle)!=SQL_SUCCESS || SQLFetch(q.handle)!=SQL_SUCCESS) return Status::Error;
        SQLINTEGER accepted=0;
        if(!integer(q.handle,1,accepted) || accepted!=1 || SQLFetch(q.handle)!=SQL_NO_DATA) return Status::Error;
        return Status::Saved;
    }
};
}
