#include "tileBatch.h"

#define QUERY_SIZE 500

Tile *executeTileStatement(sqlite3_stmt *stmt)
{
    int z, x, y, blobSize;
    char *blob;

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        z = sqlite3_column_int(stmt, 0);
        x = sqlite3_column_int(stmt, 1);
        y = sqlite3_column_int(stmt, 2);
        blob = strdup(sqlite3_column_blob(stmt, 3));
        blobSize = sqlite3_column_int(stmt, 4);
    }
    else
    {
        return NULL;
    }

    Tile *tile = createTile(z, x, y, blob, blobSize);
    return tile;
}

char *getBatchSelectQuery(char *tileCache, int currentOffset, int batchSize)
{
    char *sql = (char *)malloc(QUERY_SIZE * sizeof(char));
    sprintf(sql, "SELECT zoom_level, tile_column, tile_row, hex(tile_data), length(hex(tile_data)) as blob_size FROM %s limit %d offset %d", tileCache, batchSize, currentOffset);
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

TileBatch *getCorrespondingBatch(TileBatch *tileBatch, sqlite3 *db, char *tileCache)
{
    TileBatch *newTileBatch = (TileBatch *)malloc(sizeof(TileBatch));
    Tile **tiles = (Tile **)malloc(tileBatch->size * sizeof(Tile *));
    for (int i = 0; i < tileBatch->size; i++)
    {
        Tile *newTile = getNextTile(tileBatch);
        Tile *baseTile = getTile(db, tileCache, newTile->z, newTile->x, newTile->y);

        if (baseTile == NULL)
        {
            baseTile = getLastExistingTile(newTile->x, newTile->y, newTile->z, db, tileCache);
        }
        tiles[i] = baseTile;
    }

    newTileBatch->tiles = tiles;
    newTileBatch->size = tileBatch->size;
    newTileBatch->current = 0;
    tileBatch->current = 0;
    return newTileBatch;
}

Tile *getNextTile(TileBatch *tileBatch)
{
    unsigned int currentTile = __atomic_fetch_add(&(tileBatch->current), 1, __ATOMIC_SEQ_CST);

    // If we are at the end of the batch
    if (currentTile == tileBatch->size)
    {
        return NULL;
    }

    Tile *tile = tileBatch->tiles[currentTile];
    return tile;
}

void printBatch(TileBatch *batch)
{
    Tile **tiles = batch->tiles;
    for (int i = 0; i < batch->size; i++)
    {
        if (tiles[i] != NULL)
        {
            printTile(tiles[i]);
        }
    }
}

void freeBatch(TileBatch *batch)
{
    // Free each tile in the batch
    Tile **tiles = batch->tiles;
    for (int i = 0; i < batch->size; i++)
    {
        if (tiles[i] != NULL)
        {
            freeTile(tiles[i]);
        }
    }
    free(tiles);
    free(batch);
}
