#ifndef TILE_BATCH_H_ /* Include guard */
#define TILE_BATCH_H_

#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <stdatomic.h>
#include "tile.h"
#include "statement.h"

typedef struct TileBatch
{
    Tile **tiles;
    atomic_int current;
    int size;
} TileBatch;

TileBatch *getTileBatch(sqlite3 *db, char *tileCache, int batchSize, int current);

TileBatch *getCorrespondingBatch(TileBatch *tileBatch, sqlite3 *db, char *tileCache);

/**
 * @brief Get the Next Tile object
 * 
 * @param tileBatch Batch object to get the tile from
 * @return Tile* 
 */
Tile *getNextTile(TileBatch *tileBatch);

void printBatch(TileBatch *batch);
void freeBatch(TileBatch *batch);

#endif // TILE_BATCH_H_
