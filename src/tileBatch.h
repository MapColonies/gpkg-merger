#ifndef TILE_BATCH_H_ /* Include guard */
#define TILE_BATCH_H_

#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "tile.h"

typedef struct TileBatch
{
    Tile **tiles;
    int current;
    int size;
} TileBatch;

TileBatch *getTileBatch(sqlite3 *db, char *tileCache, int batchSize, int current);
Tile *getNextTile(TileBatch *TileBatch);
void printBatch(TileBatch *batch);
void freeBatch(TileBatch *batch);

#endif // TILE_BATCH_H_
