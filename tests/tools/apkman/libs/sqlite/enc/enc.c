// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <openenclave/internal/tests.h>
#include <openenclave/internal/trace.h>
#include "test_t.h"

#include <stdio.h>
#include <string.h>

#include <sqlite3.h>

static int callback(void* unused, int argc, char** argv, char** column_name)
{
    OE_UNUSED(unused);
    int i;
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", column_name[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

#define SQL_TEST(expr)                                                  \
    do {                                                                \
        if ((expr) != SQLITE_OK) {                                      \
            fprintf(stderr, "sqlite error: %s\n", sqlite3_errmsg(db));  \
            OE_TRACE_ERROR("%s failed\n", #expr);			\
	    oe_abort();							\
        }								\
        else {                                                          \
            fprintf(stdout, "%s succeeded\n", #expr);			\
        }                                                               \
    } while(0)

int enc_main(int argc, char** argv)
{
    OE_UNUSED(argc);
    OE_UNUSED(argv);

    sqlite3* db = NULL;
    const char* sql = NULL;
    sqlite3_stmt* stmt = NULL;
    
    // Print version
    printf("sqlite version: %s\n", sqlite3_libversion());
    
    // Open database
    SQL_TEST(sqlite3_open(":memory:", &db));

    /* Create table */
    sql = "CREATE TABLE PATIENT("
	"ID INT PRIMARY KEY     NOT NULL,"
	"NAME           TEXT    NOT NULL,"
	"AGE            INT     NOT NULL,"
	"ADDRESS        TEXT    NOT NULL);";

    /* Execute SQL statement */
    SQL_TEST(sqlite3_exec(db, sql, callback, 0, NULL));

    /* Insert items */
    sql = "INSERT INTO PATIENT (ID,NAME,AGE,ADDRESS) "
	"VALUES (1, 'Dennis', 72, 'California'); "
	"INSERT INTO PATIENT (ID,NAME,AGE,ADDRESS) "
	"VALUES (2, 'Bjarne', 65, 'Texas'); ";
    SQL_TEST(sqlite3_exec(db, sql, callback, 0, NULL));

    /* Fetch items */
    sql = "SELECT * from PATIENT";
    SQL_TEST(sqlite3_exec(db, sql, callback, NULL, NULL));
    
    SQL_TEST(sqlite3_finalize(stmt));
    SQL_TEST(sqlite3_close(db));

    return 0;
}

OE_SET_ENCLAVE_SGX(
    1,    /* ProductID */
    1,    /* SecurityVersion */
    true, /* Debug */
    1024, /* NumHeapPages */
    1024, /* NumStackPages */
    2);   /* NumTCS */
