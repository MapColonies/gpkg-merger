#include "tileBatch.h"

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

Tile *executeTileStatement(sqlite3_stmt *stmt)
{
    // unsigned char *res = NULL;
    int z, x, y;
    char *blob;

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        z = sqlite3_column_int(stmt, 0);
        x = sqlite3_column_int(stmt, 1);
        y = sqlite3_column_int(stmt, 2);
        blob = strdup(sqlite3_column_blob(stmt, 3));
    }
    else
    {
        return NULL;
    }

    Tile *tile = createTile(z, x, y, blob);
    return tile;
}

char *getBatchSelectQuery(char *tileCache, int currentOffset, int batchSize)
{
    char prefix[] = "SELECT zoom_level, tile_column, tile_row, hex(tile_data) FROM ";
    char *sql = (char *)malloc(200 * sizeof(char));
    sprintf(sql, "SELECT zoom_level, tile_column, tile_row, hex(tile_data) FROM %s limit %d offset %d", tileCache, batchSize, currentOffset);
    return sql;
}

Tile **getNextBatch(sqlite3 *db, char *tileCache, int batchSize, int current)
{
    Tile **tiles = (Tile **)malloc(batchSize * sizeof(Tile *));
    int i = -1;

    char *batchSelectQuery = getBatchSelectQuery(tileCache, current, batchSize);
    sqlite3_stmt *stmt = prepareStatement(db, batchSelectQuery);

    do
    {
        i++;
        tiles[i] = executeTileStatement(stmt);
    } while (tiles[i] != NULL);

    // for (int i = 0; tiles[i] != NULL && i < batchSize; i++)
    // {
    //     tiles[i] = executeTileStatement(stmt);
    // }

    free(batchSelectQuery);
    finalizeStatement(stmt);

    // const char *res = executeStatement(gpkg->db, batchSelectQuery);
    // printf("%s\n", res);

    return tiles;
}

TileBatch *getTileBatch(sqlite3 *db, char *tileCache, int batchSize, int current)
{
    TileBatch *tileBatch = (TileBatch *)malloc(sizeof(TileBatch));
    Tile **tiles = getNextBatch(db, tileCache, batchSize, current);
    size_t i = 0;

    tileBatch->tiles = tiles;

    // Determine amount of tiles in batch
    while (i < batchSize && tiles[i] != NULL)
    {
        i++;
    }

    tileBatch->size = i;

    return tileBatch;
}

/**
 * @brief Get the Next Tile object
 * 
 * @param tileBatch Batch object to get the tile from
 * @return struct Tile* 
 */
struct Tile *getNextTile(TileBatch *tileBatch)
{
    Tile *tile = tileBatch->tiles[tileBatch->current];
    tileBatch->current++;
    return tile;
}

void printBatch(TileBatch *batch)
{
    Tile **tiles = batch->tiles;
    for (size_t i = 0; i < batch->size; i++)
    {
        printTile(tiles[i]);
    }
}

void freeBatch(TileBatch *batch)
{
    // Free each tile in the batch
    Tile **tiles = batch->tiles;
    for (size_t i = 0; i < batch->size; i++)
    {
        freeTile(tiles[i]);
    }

    free(batch);
}