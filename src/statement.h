#ifndef STATEMENT_H_ /* Include guard */
#define STATEMENT_H_

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

sqlite3_stmt *prepareStatement(sqlite3 *db, char *query);

/**
 * @brief Execute sql query with single row and column result
 * 
 * @param db THe database to run the query on
 * @param query Sql query to run
 * @return char* The query result
 */
char *executeStatementSingleColResult(sqlite3 *db, char *query);

void finalizeStatement(sqlite3_stmt *stmt);

#endif // STATEMENT_H_
