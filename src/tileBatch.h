#ifndef TILE_BATCH_H_ /* Include guard */
#define TILE_BATCH_H_

#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <stdatomic.h>
#include "tile.h"
#include "statement.h"
#include "ImageWand/upscaling.h"

typedef struct TileBatch
{
    int current;
    int size;
    Tile **tiles;
} TileBatch;

TileBatch *getTileBatch(sqlite3_stmt *stmt, int batchSize, int current);

TileBatch *getCorrespondingBatch(TileBatch *tileBatch, sqlite3 *db, sqlite3_stmt *getTileStmt, sqlite3_stmt *getBlobSizeStmt, char *tileCache);

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
