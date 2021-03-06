#ifndef TILE_H_ /* Include guard */
#define TILE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "statement.h"

typedef struct Tile
{
    int z;
    int x;
    int y;
    int blobSize;
    char *blob;
} Tile;

Tile *createTile(int z, int x, int y, char *blob, int blobSize);

/**
 * @brief Get a Tile object with the given z, x, y
 * 
 * @param db database to query
 * @param getTileStmt get tile statement
 * @param getBlobSizeStmt get blob size statement
 * @param tileCache table to query
 * @param z z value
 * @param x z value
 * @param y y value
 * @return Tile* 
 */
Tile *getTile(sqlite3 *db, sqlite3_stmt *getTileStmt, sqlite3_stmt *getBlobSizeStmt, char *tileCache, int z, int x, int y);

void printTile(Tile *tile);

/**
 * @brief Free tile memory
 * 
 * @param tile Tile to free
 */
void freeTile(Tile *tile);

#endif // TILE_H_
