#include "tileBatch.h"

Tile *executeTileStatement(sqlite3_stmt *stmt)
{
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
    char *sql = (char *)malloc(500 * sizeof(char));
    sprintf(sql, "SELECT zoom_level, tile_column, tile_row, hex(tile_data) FROM %s limit %d offset %d", tileCache, batchSize, currentOffset);
    return sql;
}

Tile **getNextBatch(sqlite3 *db, char *tileCache, int batchSize, int current)
{
    Tile **tiles = (Tile **)malloc(batchSize * sizeof(Tile *));
    int i = 0;

    char *batchSelectQuery = getBatchSelectQuery(tileCache, current, batchSize);
    sqlite3_stmt *stmt = prepareStatement(db, batchSelectQuery);

    do
    {
        tiles[i] = executeTileStatement(stmt);
        i++;
    } while (i < batchSize && tiles[i - 1] != NULL);

    free(batchSelectQuery);
    finalizeStatement(stmt);

    return tiles;
}

TileBatch *getTileBatch(sqlite3 *db, char *tileCache, int batchSize, int current)
{
    TileBatch *tileBatch = (TileBatch *)malloc(sizeof(TileBatch));
    Tile **tiles = getNextBatch(db, tileCache, batchSize, current);
    int i = 0;

    tileBatch->tiles = tiles;

    // Determine amount of tiles in batch
    while (i < batchSize && tiles[i] != NULL)
    {
        i++;
    }

    tileBatch->size = i;
    tileBatch->current = 0;

    return tileBatch;
}

Tile *getNextTile(TileBatch *tileBatch)
{
    // If we are at the end of the batch
    if (tileBatch->current == tileBatch->size)
    {
        return NULL;
    }

    Tile *tile = tileBatch->tiles[tileBatch->current];
    tileBatch->current++;
    return tile;
}

void printBatch(TileBatch *batch)
{
    Tile **tiles = batch->tiles;
    for (int i = 0; i < batch->size; i++)
    {
        printTile(tiles[i]);
    }
}

void freeBatch(TileBatch *batch)
{
    // Free each tile in the batch
    Tile **tiles = batch->tiles;
    for (int i = 0; i < batch->size; i++)
    {
        freeTile(tiles[i]);
    }

    free(batch);
}
