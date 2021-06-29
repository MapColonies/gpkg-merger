#include "statement.h"

#define QUERY_SIZE 500

sqlite3_stmt *prepareStatement(sqlite3 *db, char *query, int flags)
{
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v3(db, query, -1, flags, &stmt, 0);
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

char *executeQuerySingleColResult(sqlite3 *db, char *query)
{
    sqlite3_stmt *stmt = prepareStatement(db, query, 0);
    return executeStatementSingleColResult(db, stmt, 1);
}

char *executeStatementSingleColResult(sqlite3 *db, sqlite3_stmt *stmt, int finalize)
{
    unsigned char *res = NULL;

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        res = strdup(sqlite3_column_text(stmt, 0));
    }
    else if (rc != SQLITE_DONE && rc != SQLITE_OK)
    {
        printf("Error in sqlite, code: %d\n", rc);
        printf("Error in sqlite: %s\n", sqlite3_errmsg(db));
    }

    if (finalize)
    {
        sqlite3_finalize(stmt);
    }

    return res;
}

char *getBlobSizeQuery(char *tileCache, int z, int x, int y)
{
    char *sql = (char *)malloc(QUERY_SIZE * sizeof(char));
    sprintf(sql, "SELECT length(hex(tile_data)) FROM %s where zoom_level=%d and tile_column=%d and tile_row=%d", tileCache, z, x, y);
    return sql;
}

int getBlobSize(sqlite3 *db, char *tileCache, int z, int x, int y)
{
    char *query = getBlobSizeQuery(tileCache, z, x, y);
    char *res = executeQuerySingleColResult(db, query);
    int size = atoi(res);

    free(res);
    free(query);
    return size;
}
