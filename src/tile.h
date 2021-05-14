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
    char *blob;
} Tile;

struct Tile *createTile(int z, int x, int y, char *blob);
Tile *getTile(sqlite3 *db, char *tileCache, int z, int x, int y);
void printTile(Tile *tile);
void freeTile(Tile *tile);

#endif // TILE_H_