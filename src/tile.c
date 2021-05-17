#include "tile.h"

char *getTileSelectQuery(char *tileCache, int z, int x, int y)
{
    // char prefix[] = "SELECT zoom_level, tile_column, tile_row, hex(tile_data) FROM ";
    char *sql = (char *)malloc(500 * sizeof(char));
    sprintf(sql, "SELECT hex(tile_data) FROM %s where zoom_level=%d and tile_column=%d and tile_row=%d", tileCache, z, x, y);
    return sql;
}

Tile *createTile(int z, int x, int y, char *blob)
{
    Tile *tile = (Tile *)malloc(sizeof(Tile));
    tile->z = z;
    tile->x = x;
    tile->y = y;
    tile->blob = blob;
    return tile;
}

Tile *getTile(sqlite3 *db, char *tileCache, int z, int x, int y)
{
    char *query = getTileSelectQuery(tileCache, z, x, y);
    char *blob = executeStatementSingleColResult(db, query);
    free(query);
    Tile *tile = createTile(z, x, y, blob);
    return tile;
}

void printTile(Tile *tile)
{
    printf("z: %d\n", tile->z);
    printf("x: %d\n", tile->x);
    printf("y: %d\n", tile->y);
    printf("blob: %s\n", tile->blob);
}

void freeTile(Tile *tile)
{
    free(tile->blob);
    free(tile);
}
