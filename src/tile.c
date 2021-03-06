#include "tile.h"

#define QUERY_SIZE 500

Tile *createTile(int z, int x, int y, char *blob, int blobSize)
{
    Tile *tile = (Tile *)malloc(sizeof(Tile));
    tile->z = z;
    tile->x = x;
    tile->y = y;
    tile->blob = blob;
    tile->blobSize = blobSize;
    return tile;
}

Tile *getTile(sqlite3 *db, sqlite3_stmt *getTileStmt, sqlite3_stmt *getBlobSizeStmt, char *tileCache, int z, int x, int y)
{
    bindTileSelect(getTileStmt, x, y, z);
    char *blob = executeStatementSingleColResult(db, getTileStmt, 0);

    if (blob == NULL)
    {
        return NULL;
    }

    int blobSize = getBlobSize(db, getBlobSizeStmt, tileCache, z, x, y);
    Tile *tile = createTile(z, x, y, blob, blobSize);
    return tile;
}

void printTile(Tile *tile)
{
    printf("z: %d\n", tile->z);
    printf("x: %d\n", tile->x);
    printf("y: %d\n", tile->y);
    // printf("blob: %s\n", tile->blob);
    printf("blob size: %d\n", tile->blobSize);
}

void freeTile(Tile *tile)
{
    free(tile->blob);
    free(tile);
}
