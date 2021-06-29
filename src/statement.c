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

sqlite3_stmt *getBatchSelectStmt(sqlite3 *db, char *tileCache)
{
    char *sql = (char *)malloc(QUERY_SIZE * sizeof(char));
    sprintf(sql, "SELECT zoom_level, tile_column, tile_row, hex(tile_data), length(hex(tile_data)) as blob_size FROM %s limit ? offset ?", tileCache);
    sqlite3_stmt *stmt = prepareStatement(db, sql, SQLITE_PREPARE_PERSISTENT);
    free(sql);
    return stmt;
}

void bindBatchSelect(sqlite3_stmt *stmt, int limit, int offset)
{
    sqlite3_reset(stmt);
    sqlite3_bind_int(stmt, 1, limit);
    sqlite3_bind_int(stmt, 2, offset);
}

sqlite3_stmt *getTileSelectStmt(sqlite3 *db, char *tileCache)
{
    char *sql = (char *)malloc(QUERY_SIZE * sizeof(char));
    sprintf(sql, "SELECT hex(tile_data) FROM %s where zoom_level=? and tile_column=? and tile_row=?", tileCache);
    sqlite3_stmt *stmt = prepareStatement(db, sql, SQLITE_PREPARE_PERSISTENT);
    free(sql);
    return stmt;
}

void bindTileSelect(sqlite3_stmt *stmt, int x, int y, int z)
{
    sqlite3_reset(stmt);
    sqlite3_bind_int(stmt, 1, z);
    sqlite3_bind_int(stmt, 2, x);
    sqlite3_bind_int(stmt, 3, y);
}

sqlite3_stmt *getBlobSizeStmt(sqlite3 *db, char *tileCache)
{
    char *sql = (char *)malloc(QUERY_SIZE * sizeof(char));
    sprintf(sql, "SELECT length(hex(tile_data)) FROM %s where zoom_level=? and tile_column=? and tile_row=?", tileCache);
    sqlite3_stmt *stmt = prepareStatement(db, sql, SQLITE_PREPARE_PERSISTENT);
    free(sql);
    return stmt;
}

int getBlobSize(sqlite3 *db, sqlite3_stmt *stmt, char *tileCache, int z, int x, int y)
{
    bindTileSelect(stmt, x, y, z);
    char *res = executeStatementSingleColResult(db, stmt, 0);
    int size = atoi(res);

    free(res);
    return size;
}
