#ifndef STATEMENT_H_ /* Include guard */
#define STATEMENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

sqlite3_stmt *prepareStatement(sqlite3 *db, char *query, int flags);

/**
 * @brief Execute sql query with single row and column result
 * 
 * @param db The database to run the query on
 * @param query Sql query to run
 * @return char* The query result
 */
char *executeQuerySingleColResult(sqlite3 *db, char *query);

/**
 * @brief Execute sql statement with single row and column result
 * 
 * @param db The database to run the query on
 * @param stmt Sql statement to run
 * @param finalize Should finalize statement
 * @return char* The query result
 */
char *executeStatementSingleColResult(sqlite3 *db, sqlite3_stmt *stmt, int finalize);

void finalizeStatement(sqlite3_stmt *stmt);

int getBlobSize(sqlite3 *db, sqlite3_stmt *stmt, char *tileCache, int z, int x, int y);

sqlite3_stmt *getBatchSelectStmt(sqlite3 *db, char *tileCache);

sqlite3_stmt *getBlobSizeStmt(sqlite3 *db, char *tileCache);

void bindBatchSelect(sqlite3_stmt *stmt, int limit, int offset);

#endif // STATEMENT_H_
