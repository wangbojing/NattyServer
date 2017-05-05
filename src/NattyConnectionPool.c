/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Author. (C) 2016
 * 
 *
 
****       *****
  ***        *
  ***        *                         *               *
  * **       *                         *               *
  * **       *                         *               *
  *  **      *                        **              **
  *  **      *                       ***             ***
  *   **     *       ******       ***********     ***********    *****    *****
  *   **     *     **     **          **              **           **      **
  *    **    *    **       **         **              **           **      *
  *    **    *    **       **         **              **            *      *
  *     **   *    **       **         **              **            **     *
  *     **   *            ***         **              **             *    *
  *      **  *       ***** **         **              **             **   *
  *      **  *     ***     **         **              **             **   *
  *       ** *    **       **         **              **              *  *
  *       ** *   **        **         **              **              ** *
  *        ***   **        **         **              **               * *
  *        ***   **        **         **     *        **     *         **
  *         **   **        **  *      **     *        **     *         **
  *         **    **     ****  *       **   *          **   *          *
*****        *     ******   ***         ****            ****           *
                                                                       *
                                                                      *
                                                                  *****
                                                                  ****


 *
 */


#include "NattyAbstractClass.h"
#include "NattyConnectionPool.h"
#include "NattyResult.h"

#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <limits.h>

#define MAX_SQL_BUFFER_SIZE			1024

#if 0 //debug_server

const char *hostName = "192.168.1.178";
const char *userName = "juguikeji";
const char *password = "qjk.123456";
const char *dbName = "NATTYDB";
const unsigned int dbPort = 3306;

#else

const char *hostName = "112.93.116.188";
const char *userName = "watch_server";
const char *password = "123456";
const char *dbName = "NATTYDB";
const unsigned int dbPort = 3306;


#endif
#if 0
MYSQL *gConn;
MYSQL_RES *gRes;
MYSQL_ROW gRow;

void print_mysql_error(const char *msg) {
	if (msg) {
		printf("%s:%s\n", msg, mysql_error(gConn));
	} else {
		puts(mysql_error(gConn));
	}
}


int executesql(const char *sql) {
	if (mysql_real_query(gConn, sql, strlen(sql))) {
		return -1;
	}
	return 0;
}

int init_mysql() {
	gConn = mysql_init(NULL);

	if (!mysql_real_connect(gConn, hostName, userName, password, dbName, dbPort, NULL, 0)) {
		return -1;
	}
	if (executesql("set names utf8")) {
		return -1;
	}
	return 0;
}


int main(void) {
	int i = 0;
	puts("!!! Hello World !!!\n");

	if (init_mysql()) {
		print_mysql_error(NULL);
	}

	char sql[MAX_SQL_BUFFER_SIZE] = {0};

	sprintf(sql, "CALL PROC_CHECK_GROUP(11825, 240207489189498949);");
//"CALL PROC_INSERT_BIND_GROUP(303)"
//	sprintf(sql, "CALL PROC_INSERT_BIND_GROUP(245)");
#if 0
	if (executesql(sql)) {
		print_mysql_error(NULL);
	}

	gRes = mysql_store_result(gConn);

	int nRows = mysql_num_rows(gRes);
	int iFields = mysql_num_fields(gRes);

	while (gRow = mysql_fetch_row(gRes)) {
//		long long imei = (long long)gRow[0];
//		const char *name = (const char *)gRow[1];
//long long ll = strtoll(s, &e, 10);
		printf("imei:%s \t name:%s \t", gRow[0], gRow[1]);
		char *e;
		long long ll = strtoll(gRow[0], &e, 10);

		printf(" devId:%lld\n", ll);
	}

	mysql_free_result(gRes);
#else

	MYSQL_STMT *stmt = mysql_stmt_init(gConn);
	if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
		ntylog("mysql_stmt_prepare --> failed\n");
		return -1;
	}

	int ret = 0;
	long long id = 0;
	char name[256] = {0};

	MYSQL_BIND params[3] = {0};
	MYSQL_FIELD *field[3] = {0};


	mysql_stmt_execute(stmt);

	int count = mysql_stmt_field_count(stmt);
	ntylog("count : %d\n", count);
	gRes = mysql_stmt_result_metadata(stmt);

	int row = mysql_stmt_num_rows(stmt);
	ntylog("row : %d\n", row);

	for (i = 0;i < count;i ++) {
		

		if (i == 0) {
			params[i].buffer_type = MYSQL_TYPE_LONG;
			params[i].buffer = &ret;
		} else if (i == 1) {
			params[i].buffer_type = MYSQL_TYPE_LONGLONG;
			params[i].buffer = &id;
		} else if (i == 2) {
			params[i].buffer_type = MYSQL_TYPE_STRING;
			params[i].buffer = name;
			params[i].buffer_length = 256;
		}
		
		field[i] = mysql_fetch_field_direct(gRes, i);
	
	}
	
	mysql_stmt_bind_result(stmt, params);

	mysql_stmt_store_result(stmt);
	int res = 0;
	while ((res = mysql_stmt_fetch(stmt)) == 0) {
		ntylog("ret:%d\tid:%lld\tname:%s\n", ret, id, name);
	}
	ntylog("res : %d\n", res);

	mysql_stmt_close(stmt);

#endif
	mysql_close(gConn);

	return 0;
}


#else


#if 1 //code from libzdb

#define USEC_PER_SEC 1000000

#define USEC_PER_MSEC 1000

#define TM_GMTOFF tm_wday

#define STR_DEF(s) ((s) && *(s))


#define _i2a(i) (x[0] = ((i) / 10) + '0', x[1] = ((i) % 10) + '0')

static inline int _a2i(const char *a, int l) {
        int n = 0;
        for (; *a && l--; a++)
                n = n * 10 + (*a) - '0';
        return n;
}


struct tm *Time_toDateTime(const char *s, struct tm *t);


time_t Time_toTimestamp(const char *s) {
        if (STR_DEF(s)) {
                struct tm t = {};
                if (Time_toDateTime(s, &t)) {
                        t.tm_year -= 1900;
                        time_t offset = t.TM_GMTOFF;
                        return timegm(&t) - offset;
                }
        }
	return 0;
}


struct tm *Time_toDateTime(const char *s, struct tm *t) {
        assert(t);
        assert(s);
        struct tm tm = {.tm_isdst = -1}; 
        int has_date = false, has_time = false;
        const char *limit = s + strlen(s), *marker, *token, *cursor = s;
	while (true) {
		if (cursor >= limit) {
                        if (has_date || has_time) {
                                *(struct tm*)t = tm;
                                return t;
                        }
                        
                }
                token = cursor;
                
#line 187 "<stdout>"
{
	unsigned char yych;
	unsigned int yyaccept = 0;
	static const unsigned char yybm[] = {
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
	};

	yych = *cursor;
	if (yych <= ',') {
		if (yych == '+') goto yy4;
		goto yy5;
	} else {
		if (yych <= '-') goto yy4;
		if (yych <= '/') goto yy5;
		if (yych >= ':') goto yy5;
	}
	yyaccept = 0;
	yych = *(marker = ++cursor);
	if (yych <= '/') goto yy3;
	if (yych <= '9') goto yy15;
yy3:
#line 243 "src/system/Time.re"
	{
                        continue;
                 }
#line 244 "<stdout>"
yy4:
	yyaccept = 0;
	yych = *(marker = ++cursor);
	if (yych <= '/') goto yy3;
	if (yych <= '9') goto yy6;
	goto yy3;
yy5:
	yych = *++cursor;
	goto yy3;
yy6:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych <= '9') goto yy8;
yy7:
	cursor = marker;
	if (yyaccept <= 1) {
		if (yyaccept == 0) {
			goto yy3;
		} else {
			goto yy9;
		}
	} else {
		if (yyaccept == 2) {
			goto yy23;
		} else {
			goto yy31;
		}
	}
yy8:
	yyaccept = 1;
	yych = *(marker = ++cursor);
	if (yych == '\n') goto yy9;
	if (yych <= '/') goto yy10;
	if (yych <= '9') goto yy11;
	goto yy10;
yy9:
#line 230 "src/system/Time.re"
	{ // Timezone: +-HH:MM, +-HH or +-HHMM is offset from UTC in seconds
                        if (has_time) { // Only set timezone if time has been seen
                                tm.TM_GMTOFF = _a2i(token + 1, 2) * 3600;
                                if (isdigit(token[3]))
                                        tm.TM_GMTOFF += _a2i(token + 3, 2) * 60;
                                else if (isdigit(token[4]))
                                        tm.TM_GMTOFF += _a2i(token + 4, 2) * 60;
                                if (token[0] == '-')
                                        tm.TM_GMTOFF *= -1;
                        }
                        continue;
                 }
#line 294 "<stdout>"
yy10:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych <= '9') goto yy14;
	goto yy7;
yy11:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yych = *++cursor;
	if (yych <= '/') goto yy9;
	if (yych >= ':') goto yy9;
yy13:
	yych = *++cursor;
	goto yy9;
yy14:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych <= '9') goto yy13;
	goto yy7;
yy15:
	yych = *++cursor;
	if (yych <= '/') goto yy17;
	if (yych >= ':') goto yy17;
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych <= '9') goto yy27;
	goto yy7;
yy17:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yych = *++cursor;
	if (yych <= '/') goto yy20;
	if (yych <= '9') goto yy7;
yy20:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yyaccept = 2;
	yych = *(marker = ++cursor);
	if (yych == ',') goto yy24;
	if (yych == '.') goto yy24;
yy23:
#line 214 "src/system/Time.re"
	{ // Time: HH:MM:SS
                        tm.tm_hour = _a2i(token, 2);
                        tm.tm_min  = _a2i(token + 3, 2);
                        tm.tm_sec  = _a2i(token + 6, 2);
                        has_time = true;
                        continue;
                 }
#line 353 "<stdout>"
yy24:
	yych = *++cursor;
	if (yybm[0+yych] & 128) {
		goto yy25;
	}
	goto yy7;
yy25:
	++cursor;
	yych = *cursor;
	if (yybm[0+yych] & 128) {
		goto yy25;
	}
	goto yy23;
yy27:
	yych = *++cursor;
	if (yych <= '/') goto yy28;
	if (yych <= '9') goto yy29;
yy28:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych <= '9') goto yy38;
	goto yy7;
yy29:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yyaccept = 3;
	yych = *(marker = ++cursor);
	if (yych <= '-') {
		if (yych == ',') goto yy33;
	} else {
		if (yych <= '.') goto yy33;
		if (yych <= '/') goto yy31;
		if (yych <= '9') goto yy32;
	}
yy31:
#line 222 "src/system/Time.re"
	{ // Compressed Time: HHMMSS
                        tm.tm_hour = _a2i(token, 2);
                        tm.tm_min  = _a2i(token + 2, 2);
                        tm.tm_sec  = _a2i(token + 4, 2);
                        has_time = true;
                        continue;
                 }
#line 398 "<stdout>"
yy32:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych <= '9') goto yy36;
	goto yy7;
yy33:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
yy34:
	++cursor;
	yych = *cursor;
	if (yych <= '/') goto yy31;
	if (yych <= '9') goto yy34;
	goto yy31;
yy36:
	++cursor;
#line 206 "src/system/Time.re"
	{ // Compressed Date: YYYYMMDD
                        tm.tm_year  = _a2i(token, 4);
                        tm.tm_mon   = _a2i(token + 4, 2) - 1;
                        tm.tm_mday  = _a2i(token + 6, 2);
                        has_date = true;
                        continue;
                 }
#line 424 "<stdout>"
yy38:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yych = *++cursor;
	if (yych <= '/') goto yy40;
	if (yych <= '9') goto yy7;
yy40:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	++cursor;
#line 198 "src/system/Time.re"
	{ // Date: YYYY-MM-DD
                        tm.tm_year  = _a2i(token, 4);
                        tm.tm_mon   = _a2i(token + 5, 2) - 1;
                        tm.tm_mday  = _a2i(token + 8, 2);
                        has_date = true;
                        continue;
                 }
#line 448 "<stdout>"
}
#line 246 "src/system/Time.re"

        }
	return NULL;
}


char *Time_toString(time_t time, char result[20]) {
        assert(result);
        char x[2];
        struct tm ts = {.tm_isdst = -1};
        gmtime_r(&time, &ts);
        memcpy(result, "YYYY-MM-DD HH:MM:SS\0", 20);
        /*              0    5  8  11 14 17 */
        _i2a((ts.tm_year+1900)/100);
        result[0] = x[0];
        result[1] = x[1];
        _i2a((ts.tm_year+1900)%100);
        result[2] = x[0];
        result[3] = x[1];
        _i2a(ts.tm_mon + 1); // Months in 01-12
        result[5] = x[0];
        result[6] = x[1];
        _i2a(ts.tm_mday);
        result[8] = x[0];
        result[9] = x[1];
        _i2a(ts.tm_hour);
        result[11] = x[0];
        result[12] = x[1];
        _i2a(ts.tm_min);
        result[14] = x[0];
        result[15] = x[1];
        _i2a(ts.tm_sec);
        result[17] = x[0];
        result[18] = x[1];
	return result;
}


time_t Time_now(void) {
	struct timeval t;
	if (gettimeofday(&t, NULL) != 0) return 0;
	
	return t.tv_sec;
}


long long Time_milli(void) {
	struct timeval t;
	if (gettimeofday(&t, NULL) != 0) return 0;
	
	return (long long)t.tv_sec * 1000  +  (long long)t.tv_usec / 1000;
}


int Time_usleep(long u) {
        struct timeval t;
        t.tv_sec = u / USEC_PER_SEC;
        t.tv_usec = (suseconds_t)(u % USEC_PER_SEC);
        select(0, 0, 0, 0, &t);
        return true;
}



#endif


static inline int ntyCheckAndSetColumnIndex(int columnIndex, int columnCount) {
        int i = columnIndex - 1;
        if (columnCount <= 0 || i < 0 || i >= columnCount) {
			return NTY_RESULT_FAILED;
		}
        return i;
}



int ntyStrParseInt(const char *s) {
	if(s == NULL) return 0;
	
    errno = 0;
    char *e;
	
	int i = (int)strtol(s, &e, 10);
	if (errno || (e == s))
		return 0;
	
	return i;
}


long long ntyStrParseLLong(const char *s) {
	if(s == NULL) return 0;

	errno = 0;
    char *e;
	
	long long ll = strtoll(s, &e, 10);
	if (errno || (e == s))
		return 0;
	
	return ll;
}


double ntyStrParseDouble(const char *s) {
	if(s == NULL) return 0;
	
    errno = 0;
    char *e;
	
	double d = strtod(s, &e);
	if (errno || (e == s))
		return 0;
	
	return d;
}



nShrimpConn *ntyShrimpInit(nShrimpConn *c) {
	return mysql_init(c);
}

int ntyShrimpBeginTransaction(nShrimpConn *c) {
	ASSERT(c);
	return (NTY_RESULT_SUCCESS == mysql_query(c, "START TRANSACTION;"));
}


int ntyShrimpPing(nShrimpConn *c) {
	ASSERT(c);
	return (NTY_RESULT_SUCCESS == mysql_ping(c));
}

int ntyShrimpCommit(nShrimpConn *c) {
	ASSERT(c);
    return (NTY_RESULT_SUCCESS == mysql_query(c, "COMMIT;"));
}


int ntyShrimpRollback(nShrimpConn *c) {
	ASSERT(c);
	return (NTY_RESULT_SUCCESS == mysql_query(c, "ROLLBACK;"));
}


long long ntyShrimpLastRowId(nShrimpConn *c) {
	ASSERT(c);
	return (long long)mysql_insert_id(c);
}


long long ntyShrimpRowsChanged(nShrimpConn *c) {
	return (long long)mysql_affected_rows(c);
}


nShrimpStmt *ntyShrimpStmtInit(nShrimpConn *c) {
	return mysql_stmt_init(c);
}

int ntyShrimpStmtPrepare(nShrimpStmt *s, const char *sql, int length) {
	return mysql_stmt_prepare(s, sql, length);
}

int ntyShrimpStmtExecute(nShrimpStmt *s) {
	return mysql_stmt_execute(s);
}

const char *ntyShrimpStmtError(nShrimpStmt *s) {
	return mysql_stmt_error(s);
}

int ntyShrimpStmtFreeResult(nShrimpStmt *s) {
	return mysql_stmt_free_result(s);
}

int ntyShrimpStmtClose(nShrimpStmt *s) {
	return mysql_stmt_close(s);
}

int ntyShrimpStmtFieldCount(nShrimpStmt *s) {
	return mysql_stmt_field_count(s);
}

int ntyShrimpStmtFetch(nShrimpStmt *s) {
	return mysql_stmt_fetch(s);
} 

int ntyShrimpStmtFetchColumn(nShrimpStmt *s, nShrimpBind *b, unsigned int index,  unsigned long offset) {
	return mysql_stmt_fetch_column(s, b, index, offset);
}

nShrimpRes *ntyShrimpStmtResultMetaData(nShrimpStmt *s) {
	return mysql_stmt_result_metadata(s);
} 

nShrimpField *ntyShrimpFetchFieldDirect(nShrimpRes *res, int columnIndex) {
	return mysql_fetch_field_direct(res, columnIndex);
}

int ntyShrimpStmtBindResult(nShrimpStmt *res, nShrimpBind *bind) {
	return mysql_stmt_bind_result(res, bind);
}

void ntyShrimpDeInit(nShrimpConn *c) {
	mysql_close(c);
}


void ntyShrimpResRelease(nShrimpRes *res) {
	mysql_free_result(res);
}

nShrimpRow ntyShrimpFetchRow(nShrimpRes *res) {
	return mysql_fetch_row(res);
}

static inline void ensureCapacity(nSnailsSet *snail, int i) {
	int ret = 0;

	if ((snail->columns[i].real_length > snail->Bind[i].buffer_length)) {
		
		snail->columns[i].buffer = realloc(snail->columns[i].buffer, snail->columns[i].real_length + 1);
		
	    snail->Bind[i].buffer = snail->columns[i].buffer;
	    snail->Bind[i].buffer_length = snail->columns[i].real_length;
		
	    if ((ret = ntyShrimpStmtFetchColumn(snail->Stmt, &snail->Bind[i], i, 0)))
	            ntylog( "mysql_stmt_fetch_column -- %s", ntyShrimpStmtError(snail->Stmt));
		
	    snail->needRebind = 1;
	}
}


static nShrimpStmt *doPrepare(nShrimp *shrimp, const char *sql, int length) {
	nShrimpStmt *stmt = ntyShrimpStmtInit(shrimp->Conn);
	
	shrimp->error = ntyShrimpStmtPrepare(stmt, sql, length);
	if (shrimp->error) {
		ntylog("doPrepare --> ntyShrimpStmtPrepare error %s\n",  ntyShrimpStmtError(stmt));
		ntyShrimpStmtClose(stmt);
        stmt = NULL;
	}
	return stmt;
}

static int doRetrieve(nConnPool *Pool) {
	int i = 0;
	time_t timeout = Time_now();

	for (i = 0;i < Pool->cur_num;i ++) {
		nShrimp *shrimp = (nShrimp*)(Pool->ConnList+i);
		if (shrimp->Conn == NULL) continue;

		if (shrimp->enable) { //used
			if (shrimp->accessTime < (timeout - Pool->handleTimeout)) { //executeQuery not ret
				ntyConnPoolRetConnection(shrimp);
			}
		} else { //not used
			if (shrimp->accessTime < (timeout - Pool->connectTimeout)) { //
				
				ntyConnPoolCheckConnection(shrimp);
			}
		}
	}
}



nSnailsSet *ntyShrimpStmtResultSetNew(nSnailsSet *snail) {
	
	if (snail == NULL) return NULL;
	
	snail->columnCount = ntyShrimpStmtFieldCount(snail->Stmt);
	snail->needRebind = 1;
	if ((snail->columnCount <= 0) || ! (snail->Res = ntyShrimpStmtResultMetaData(snail->Stmt)))  {
		ntylog("ntyShrimpStmtResultSet --> is null\n");
		return NULL;
	} else {
		snail->Bind = malloc(snail->columnCount*sizeof(nShrimpBind));
		if (snail->Bind == NULL) return NULL;
		memset(snail->Bind, 0, snail->columnCount*sizeof(nShrimpBind));

		snail->columns = malloc(snail->columnCount * sizeof(nColumn));
		if (snail->columns == NULL) return NULL;
		memset(snail->Bind, 0, snail->columnCount*sizeof(nColumn));
		
		int i = 0;
		for (i = 0;i < snail->columnCount;i ++) {
			snail->columns[i].buffer = malloc(MAX_SQL_BUFFER_SIZE+1);
			
			snail->Bind[i].buffer_type = MYSQL_TYPE_STRING;
			snail->Bind[i].buffer = snail->columns[i].buffer;
			snail->Bind[i].buffer_length = MAX_SQL_BUFFER_SIZE;
			snail->Bind[i].is_null = &snail->columns[i].is_null;
			snail->Bind[i].length = &snail->columns[i].real_length;

			snail->columns[i].field = ntyShrimpFetchFieldDirect(snail->Res, i);
		}

		int error = ntyShrimpStmtBindResult(snail->Stmt, snail->Bind);
		if (error) {
			ntylog("Warning: bind error - %s\n", ntyShrimpStmtError(snail->Stmt));
		}
	}

	return snail;
}

int ntyShrimpStmtResultSetDelete(nSnailsSet *snail) {

	if (snail == NULL) return NTY_RESULT_ERROR;

	int i = 0;

	for (i = 0;i < snail->columnCount;i ++) {
		if (snail->columns[i].buffer != NULL) {
			free(snail->columns[i].buffer);
		}
	}
	snail->columnCount = 0;
	
	if (snail->columns != NULL) {
		free(snail->columns);
		snail->columns = NULL;
	}
	if (snail->Bind != NULL) {
		free(snail->Bind);
		snail->Bind = NULL;
	}

	if (snail->Stmt != NULL) {
		
		ntyShrimpStmtFreeResult(snail->Stmt);
		ntyShrimpStmtClose(snail->Stmt);

		snail->Stmt = NULL;
	}

	if (snail->Res != NULL) {
		ntyShrimpResRelease(snail->Res);
		snail->Res = NULL;
	}
	memset(snail, 0, sizeof(nSnailsSet));
	//free(snail);
	
	return NTY_RESULT_SUCCESS;
}

int ntyShrimpStmtResultSetNext(nSnailsSet *snail) {
	if (snail == NULL) return NTY_RESULT_ERROR;
	int ret = 0;

	if (snail->needRebind) {
		ret = ntyShrimpStmtBindResult(snail->Stmt, snail->Bind);
		if (ret) {
			ntylog("ntyShrimpStmtBindResult --> failed\n");
			return NTY_RESULT_FAILED;
		}
		snail->needRebind = 0;
    }

	ret = ntyShrimpStmtFetch(snail->Stmt);
    if (ret == 1) {
		ntylog("ntyShrimpStmtField --> %s", ntyShrimpStmtError(snail->Stmt));
    }

	
    return ((ret == NTY_RESULT_SUCCESS) || (ret == MYSQL_DATA_TRUNCATED));
}

const char *ntyShrimpStmtResultSetgetString(nSnailsSet *snail, int columnIndex) {
    if (snail == NULL) return NULL;
	
    int i = ntyCheckAndSetColumnIndex(columnIndex, snail->columnCount);
    if (snail->columns[i].is_null)
            return NULL;
	
    ensureCapacity(snail, i);
    snail->columns[i].buffer[snail->columns[i].real_length] = 0;
	
    return snail->columns[i].buffer;
}





nSnailsSet *ntyShrimpExecuteQuery(nShrimp *shrimp, const char *sql, int length) {
	if (shrimp == NULL || sql == NULL) return NULL;
#if 0	
	int ret = mysql_real_query(shrimp->Conn, sql, length);
	if (ret) return NULL;
	shrimp->Res = mysql_store_result(shrimp->Conn);
#else
	if (shrimp->Snails == NULL) return NULL;
	
	shrimp->Snails->Stmt = doPrepare(shrimp, sql, length);
	if (shrimp->Snails->Stmt == NULL) {
		return NULL;
	}
	
#if MYSQL_VERSION_ID >= 50002
	unsigned long cursor = CURSOR_TYPE_READ_ONLY;
	mysql_stmt_attr_set(shrimp->Snails->Stmt, STMT_ATTR_CURSOR_TYPE, &cursor);
#endif

	shrimp->error = ntyShrimpStmtExecute(shrimp->Snails->Stmt);
	if (shrimp->error) {
		ntylog("ntyShrimpExecuteQuery --> ntyShrimpStmtExecute error : %s\n", ntyShrimpStmtError(shrimp->Snails->Stmt));
		ntyShrimpStmtClose(shrimp->Snails->Stmt);
	}

#endif

	return ntyShrimpStmtResultSetNew(shrimp->Snails);
}

int ntyShrimpExecute(nShrimp *shrimp, const char *sql, int length) {
	if (shrimp == NULL || sql == NULL) return NTY_RESULT_ERROR;
	
	int ret = mysql_real_query(shrimp->Conn, sql, length);
	if (ret) return NTY_RESULT_FAILED;

	return NTY_RESULT_SUCCESS;
}



void *ntyMySqlConnInitialize(nShrimpConn *Conn) {

	Conn = ntyShrimpInit(NULL);

	if (!mysql_real_connect(Conn, hostName, userName, password, dbName, dbPort, NULL, 0)) {
		ntylog("ntyMySqlConnInitialize --> mysql_real_connect failed\n");
		
		free(Conn);
		return NULL;
	}
	return Conn;
}

void *ntyMySqlConnRelease(nShrimpConn *Conn) {
	if (Conn == NULL) return NULL;

	mysql_close(Conn);
	Conn = NULL;

	return Conn;
}



long long ntyResultSetgetLLong(nSnailsSet *snail, int columnIndex) {
	const char *s = ntyShrimpStmtResultSetgetString(snail, columnIndex);
	
	return s ? ntyStrParseLLong(s) : 0;
}

const char* ntyResultSetgetString(nSnailsSet *snail, int columnIndex) {
	return ntyShrimpStmtResultSetgetString(snail, columnIndex);
}

int ntyResultSetgetInt(nSnailsSet *snail, int columnIndex) {
	const char *s = ntyShrimpStmtResultSetgetString(snail, columnIndex);
	
	return s ? ntyStrParseInt(s) : 0;
}

time_t ntyResultSetgetTimeStamp(nSnailsSet *snail, int columnIndex) {
	const char *s = ntyShrimpStmtResultSetgetString(snail, columnIndex);

	time_t t = 0;
	if (STR_DEF(s))
		t = Time_toTimestamp(s);

	return t;
}


void *ntyRetrieveThreadCb(void *arg) {
	nConnPool *Pool = arg;
	if (Pool == NULL) return NULL;

	ntylog("ntyRetrieveThreadCb enter\n");

	struct timespec wait = {0, 0};

	ntyMutexLock(Pool->mutex);
	while (!Pool->stopped) {
		wait.tv_sec = Time_now() + Pool->reapInterval;

		ntySemTimeWait(Pool->alarm, Pool->mutex, wait);
		if (Pool->stopped) break;

		doRetrieve(Pool);
	}
	ntyMutexUnlock(Pool->mutex);
	
}

void *ntyShrimpNew(nShrimp *shrimp) {

	if (shrimp == NULL) return NULL;
	
	shrimp->Conn = ntyMySqlConnInitialize(shrimp->Conn);
	shrimp->Snails = (nSnailsSet*)malloc(sizeof(nSnailsSet));
	ASSERT(shrimp->Snails);
	memset(shrimp->Snails, 0, sizeof(nSnailsSet));
	
	shrimp->accessTime = Time_now();
	
	shrimp->enable = 0;
	shrimp->status = 1;
	
	shrimp->con_lock = 0;
	shrimp->arg = NULL;

	return shrimp;
}

void *ntyShrimpFree(nShrimp *shrimp) {

	if (shrimp == NULL) return NULL;

	ntyMySqlConnRelease(shrimp->Conn);

	if (shrimp->Snails) {
		free(shrimp->Snails);
	}

	return shrimp;
}

void *ntyShrimpClear(nShrimp *shrimp) {
	if (shrimp->arg) {
		free(shrimp->arg);
	}
	if (shrimp->Snails) {
		ntyShrimpStmtResultSetDelete(shrimp->Snails);

	}
	
	ntyShrimpFree(shrimp);

	return shrimp;
}

void *ntyConnPoolInitialize(nConnPool *Pool) {
	int i = 0;
	
	if (Pool == NULL) return NULL;

	if (Pool->max_num == 0) {
		Pool->max_num = NTY_DEFAULT_CONNPOOL_MAX;
	}
	if (Pool->min_num == 0) {
		Pool->min_num = NTY_DEFAULT_CONNPOOL_MIN;
	}
	if (Pool->cur_num == 0) {
		Pool->cur_num = NTY_DEFAULT_CONNPOOL_MIN;
	}
	Pool->active = 0;
	Pool->stopped = 0;

	Pool->connectTimeout = NTY_DEFAULT_CONNECTION_TIMEOUT;
	Pool->handleTimeout = NTY_DEFAULT_HANDLE_TIMEOUT;
	Pool->reapInterval = NTY_DEFAULT_REAP_TIMEOUT;

	Pool->ConnList = (nMysqlConn*)malloc(Pool->max_num * sizeof(nShrimp));
	if (Pool->ConnList == NULL) return NULL;
	
	memset(Pool->ConnList, 0, Pool->max_num * sizeof(nShrimp));

	for (i = 0;i < Pool->min_num;i ++) {
		nShrimp *ConnItem = (nMysqlConn*)(Pool->ConnList+i);
		
		ConnItem->Pool = Pool;
		ConnItem->index = i;

		ntyShrimpNew(ConnItem);
		
	}
	
#if 1 //startup 
	pthread_mutex_init(&Pool->mutex, NULL);
	pthread_cond_init(&Pool->alarm, NULL);

	int rc = pthread_create(&Pool->reaper, NULL, ntyRetrieveThreadCb, Pool);
	if (rc) {
		ntylog("ntyConnPoolInitialize --> create RetrieveThreadCb Failed\n");
	}
#endif

	return Pool;
}


void *ntyConnPoolRelease(nConnPool *Pool) {
	int i = 0;
	
	for (i = 0;i < Pool->max_num;i ++) {
		nShrimp *ConnItem = (nShrimp*)(Pool->ConnList+i);
		if (ConnItem == NULL) break;
		
		if (ConnItem->status == 1) {
			if (0 == cmpxchg(&ConnItem->con_lock, 0, 1, WORD_WIDTH)) {

				if (ConnItem->arg) {
					free(ConnItem->arg);
				}
				if (ConnItem->Snails) {
					ntyShrimpStmtResultSetDelete(ConnItem->Snails);

				}
				
				ntyShrimpFree(ConnItem);
				
				ConnItem->enable = 0;
				ConnItem->index = i;
				ConnItem->status = 0;
				
				ConnItem->con_lock = 0;
			}
		}
	}

	free(Pool->ConnList);
	
	return Pool;
}


void ntyConnPoolSetMin(nConnPool *Pool, int min) {
	Pool->min_num = min;
}

void ntyConnPoolSetMax(nConnPool *Pool, int max) {
	Pool->max_num = max;
}

int ntyConnPoolGetSize(nConnPool *Pool) {
	return Pool->cur_num;
}

int ntyConnPoolGetMax(nConnPool *Pool) {
	return Pool->max_num;
}

int ntyConnPoolGetActive(nConnPool *Pool) {
	return Pool->active;
}

void ntyConnPoolResize(nConnPool *Pool) {

	int active = Pool->active;
	int min = Pool->min_num;
	int cur = Pool->cur_num;
	int max = Pool->max_num;
	int i = 0;

	if (active <= cur*NTY_DEFAULT_CONNPOOL_RATIO) return ;
	
	for (i = 0;i < NTY_DEFAULT_CONNPOOL_INC;i ++) {
		nShrimp *ConnItem = (nShrimp*)(Pool->ConnList+i+cur);

		
	}
	
}

nShrimp *ntyConnPoolGetConnection(nConnPool *Pool) {
	int i = 0;
	nShrimp * shrimp = NULL;

	for (i = 0;i < Pool->cur_num;i ++) {
		nShrimp *ConnItem = (nShrimp*)(Pool->ConnList+i);
		if (ConnItem == NULL) return NULL;

		if (ConnItem->status == 1 && ConnItem->enable == 0) {
			if (0 == cmpxchg(&ConnItem->con_lock, 0, 1, WORD_WIDTH)) {
				
				ConnItem->enable = 1;
				Pool->active ++;
				shrimp = ConnItem;

				ConnItem->accessTime = Time_now();
				
				ConnItem->con_lock = 0;
				break;
			}
		}
	}
	return shrimp;
}

void ntyConnPoolRetConnection(nShrimp *shrimp) {
	if (shrimp == NULL) return ;
	
#if 1 //debug : Commands out of sync; you can't run this command now
	MYSQL_RES *result;
	do { 
		result = mysql_store_result(shrimp->Conn); 
    	mysql_free_result(result); 
	} while( !mysql_next_result(shrimp->Conn));
#endif

	if (0 == cmpxchg(&shrimp->con_lock, 0, 1, WORD_WIDTH)) {
		nConnPool *Pool = shrimp->Pool;
		
		Pool->active --;
		shrimp->enable = 0;


		if (shrimp->Snails) {
			ntyShrimpStmtResultSetDelete(shrimp->Snails);
		}
		
		shrimp->con_lock = 0;
	}
}

void ntyConnPoolCheckConnection(nShrimp *shrimp) {
	if (shrimp == NULL) return ;

	if (0 == cmpxchg(&shrimp->con_lock, 0, 1, WORD_WIDTH)) {

		if (!ntyShrimpPing(shrimp->Conn)) { //connect failed

			shrimp = ntyShrimpClear(shrimp);

			shrimp = ntyShrimpNew(shrimp);
		}

		shrimp->accessTime = Time_now();

		shrimp->con_lock = 0;
	}
}


nSnailsSet* ntyConnPoolExecuteQuery(nShrimp *shrimp, const char *format, ...) {
	U8 sql[MAX_SQL_BUFFER_SIZE] = {0};
	if (shrimp == NULL) return NULL;
	
	va_list ap;
	va_start(ap, format);
	va_end(ap);

	vsprintf(sql, format, ap);

	return ntyShrimpExecuteQuery(shrimp, sql, strlen(sql));

}

void ntyConnPoolExecute(nShrimp *shrimp, const char *format, ...) {
	U8 sql[MAX_SQL_BUFFER_SIZE] = {0};
	if (shrimp == NULL) return ;

	va_list ap;
	va_start(ap, format);
	va_end(ap);

	vsprintf(sql, format, ap);

	int ret = ntyShrimpExecute(shrimp, sql, strlen(sql));
	if (ret == NTY_RESULT_SUCCESS) {
		ntylog("ntyConnPoolExecute --> ntyShrimpExecute\n");
	}
	
}




#if 0
#define NTY_DB_INSERT_BIND_GROUP			"CALL PROC_INSERT_BIND_GROUP(%d)"

#define NTY_DB_WATCHIDLIST_SELECT_FORMAT	"CALL PROC_WATCHIDLIST_APPID_SELECT(%lld)"


static int ntyQueryWatchIDListSelect(void *self, C_DEVID aid) {
	nConnPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Shrimp con = ntyConnPoolGetConnection(pool);
	int ret = -1;

	
	SnailsSet r = ntyConnPoolExecuteQuery(con, "CALL PROC_SELECT_WATCHLIST_USER(%lld)", aid);
	if (r != NULL) {
		while (ntyShrimpStmtResultSetNext(r)) {
			C_DEVID id = ntyResultSetgetLLong(r, 1);
			const char *name = ntyResultSetgetString(r, 2);
			const char *pnum = ntyResultSetgetString(r, 3);
			const char *lnglat = ntyResultSetgetString(r, 7);

			ntylog(" id : %lld, name:%s, lnglat:%s, num:%s\n", id, name, lnglat, pnum);
			ret = 0;
		}
	}

	ntyConnPoolRetConnection(con);

	return ret;
}




int main() {

	nConnPool *pool = malloc(sizeof(nConnPool));

	pool = ntyConnPoolInitialize(pool);
	if (pool == NULL) {
		ntylog("ntyConnPoolInitialize --> malloc \n");
	}

	ntylog("ntyConnPoolInitialize --> Success\n");
	int msgId = 303;
	C_DEVID pId = 0x0;
	U8 ph[32] = {0};
	
	
	ntyQueryWatchIDListSelect(pool, 11931);
	ntylog("ntyQueryWatchIDListSelect --> 11931\n");
	

	ntyQueryWatchIDListSelect(pool, 11299);

	getchar();
}
#endif

#endif



