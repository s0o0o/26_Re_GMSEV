#include "game_database.hpp"
#include <Windows.h>
#include <sqlext.h>
#include <iostream>
#include <cstdio>
#include <cwchar>
#include <cstdlib>

#pragma comment(lib, "odbc32.lib")

namespace {
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

} // namespace

bool initializeDB() {
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

	//std::cout << "[DB] 연결 성공!" << std::endl;
	return true;
}

void finalizeDB() {
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

// 로그인할때 정보 확인용
bool get_DB_login_info(int& db_id, int& db_user_x, int& db_user_y, int& db_user_hp,
	int& db_user_level, int& db_user_exp, int& db_user_seed_num, int& db_user_maxHp, int& db_user_damage) {
	SQLHSTMT hstmt = 0;
	SQLRETURN ret;
	SQLINTEGER user_id, user_x, user_y, user_hp, user_level, user_exp, user_seed, user_maxHp, user_damage;
	SQLLEN cb1 = 0, cb2 = 0, cb3 = 0, cb4 = 0, cb5 = 0, cb6 = 0, cb7 = 0, cb8 = 0, cb9 = 0;

	ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	ret = SQLPrepare(hstmt, (SQLWCHAR*)L"EXEC get_user_info ?", SQL_NTS);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		//finalizeDB();
		return false;
	}
	ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &db_id, 0, NULL);
	ret = SQLExecute(hstmt);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		//finalizeDB();
		return false;
	}

	SQLBindCol(hstmt, 1, SQL_C_LONG, &user_id, 0, &cb1);
	SQLBindCol(hstmt, 2, SQL_C_LONG, &user_x, 0, &cb2);
	SQLBindCol(hstmt, 3, SQL_C_LONG, &user_y, 0, &cb3);
	SQLBindCol(hstmt, 4, SQL_C_LONG, &user_hp, 0, &cb4);
	SQLBindCol(hstmt, 5, SQL_C_LONG, &user_level, 0, &cb5);
	SQLBindCol(hstmt, 6, SQL_C_LONG, &user_exp, 0, &cb6);
	SQLBindCol(hstmt, 7, SQL_C_LONG, &user_seed, 0, &cb7);
	SQLBindCol(hstmt, 8, SQL_C_LONG, &user_maxHp, 0, &cb8);
	SQLBindCol(hstmt, 9, SQL_C_LONG, &user_damage, 0, &cb9);


	if (SQLFetch(hstmt) == SQL_SUCCESS) {
		db_id = user_id;			// db에 저장된걸 꺼내서 저장해준다..!
		db_user_x = user_x;
		db_user_y = user_y;
		db_user_hp = user_hp;
		db_user_level = user_level;
		db_user_exp = user_exp;
		db_user_seed_num = user_seed;
		db_user_maxHp = user_maxHp;
		db_user_damage = user_damage;

		/*printf("ID: %d 접속! 위치: (%d, %d) HP : %d, Level : %d, Exp : %d, SeedNum : %d, maxHp : %d, damage : %d\n",
			user_id, user_x, user_y, user_hp, user_level, user_exp, user_seed, user_maxHp, user_damage);*/
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return true;
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return false;
}

// db업뎃용
bool update_user_info(int id, int x, int y, int seed, int hp, int level, int exp, int userMaxHp, int damage) {
	SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK);  // 이전 트랜잭션 실패 가능성 대비

	SQLHSTMT hstmt;
	SQLRETURN ret;

	// 1. Statement 핸들 할당
	ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);


	ret = SQLPrepare(hstmt, (SQLWCHAR*)L"UPDATE SERVER_TABLE1 SET USER_X = ?, USER_Y = ? , USER_SEED_NUM = ?, USER_HP = ?, USER_LEVEL = ?, USER_EXP = ?, USER_MAX_HP = ? WHERE USER_ID = ?", SQL_NTS);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		std::cout << "[DB] SQLPrepare 실패!" << std::endl;
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, ret);  // 에러 로그 추가!
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	// 3. 파라미터 바인딩
	ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &x, 0, NULL);
	ret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &y, 0, NULL);
	ret = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &seed, 0, NULL);
	ret = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &hp, 0, NULL);
	ret = SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &level, 0, NULL);
	ret = SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &exp, 0, NULL);
	ret = SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userMaxHp, 0, NULL);
	ret = SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &id, 0, NULL);


	// 4. 실행
	ret = SQLExecute(hstmt);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		std::cout << "[DB] SQLExecute 실패!" << std::endl;
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, ret);  // 에러 로그 추가!
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

//	std::cout << "유저 정보 업데이트 완료: ID=" << id << " → (" << x << "," << y << ") seed: " << seed << std::endl;

	// 5. 커밋 (안 해도 기본 auto-commit라면 필요 X, 하지만 넣어주자)
	SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);

	// 6. 해제
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return true;
}

bool insert_new_user(int userId, const char* name) {
	SQLHSTMT hstmt;
	SQLRETURN ret;

	int init_x = rand() % 2000;
	int init_y = rand() % 2000;
	int init_hp = rand() % 10 + 20;
	int init_level = 1;
	int init_exp = 0;
	int init_seed = 22;
	int init_maxHp = 30;
	int init_damage = 1;

	ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) return false;

	ret = SQLPrepare(hstmt, (SQLWCHAR*)L"INSERT INTO SERVER_TABLE1 (USER_ID, USER_X, USER_Y, USER_HP, USER_LEVEL, USER_EXP, USER_SEED_NUM, USER_MAX_HP, USER_DAMAGE) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", SQL_NTS);

	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userId, 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_x, 0, NULL);
	SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_y, 0, NULL);
	SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_hp, 0, NULL);
	SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_level, 0, NULL);
	SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_exp, 0, NULL);
	SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_seed, 0, NULL);
	SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_maxHp, 0, NULL);
	SQLBindParameter(hstmt, 9, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &init_damage, 0, NULL);

	ret = SQLExecute(hstmt);
	if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, ret);  // 디버깅용 로그
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	std::cout << "[DB] 새 유저 추가 성공! ID:" << userId << std::endl;
	SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return true;
}
