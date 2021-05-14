#include "statement.h"

sqlite3_stmt *prepareStatement(sqlite3 *db, char *query)
{
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to execute query: %s, error message: %s\n", query, sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    return stmt;
}

void finalizeStatement(sqlite3_stmt *stmt)
{
    sqlite3_finalize(stmt);
}

char *executeStatementSingleColResult(sqlite3 *db, char *query)
{
    unsigned char *res = NULL;
    sqlite3_stmt *stmt = prepareStatement(db, query);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        res = strdup(sqlite3_column_text(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return res;
}
