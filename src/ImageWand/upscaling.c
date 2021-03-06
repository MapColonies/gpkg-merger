#include "upscaling.h"

#define TILE_WIDTH 256
#define TILE_HEIGHT 256
#define ZOOM_LEVEL_COUNT 25
#define X_Y_COORDS_FOR_ALL_ZOOM_LEVELS ZOOM_LEVEL_COUNT << 1 // Multiply by 2
#define SQL_QUERY_SIZE 2000

void lastTileSQLQuery(char *query, char *tableName)
{
    strcat(query, "SELECT zoom_level, tile_column, tile_row, hex(tile_data) FROM");
    strcat(query, " ");
    strcat(query, tableName);
    strcat(query, " WHERE ");

    for (int currentZoomLevel = 0; currentZoomLevel < ZOOM_LEVEL_COUNT; currentZoomLevel++)
    {
        char *sql;
        if (currentZoomLevel == ZOOM_LEVEL_COUNT - 1)
        {
            sql = "(zoom_level = ? AND tile_column = ? AND tile_row = ?) ";
        }
        else
        {
            sql = "(zoom_level = ? AND tile_column = ? AND tile_row = ?) OR ";
        }
        strcat(query, sql);
    }

    strcat(query, "ORDER BY zoom_level DESC LIMIT 1");
}

Tile *bindParametersToQueryAndExecute(sqlite3 *db, char *query, int coords[])
{
    Tile *lastTile = NULL;
    sqlite3_stmt *res;
    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);

    if (rc == SQLITE_OK)
    {
        int currentParameter = 1;
        for (int i = 0; i < X_Y_COORDS_FOR_ALL_ZOOM_LEVELS; i += 2)
        {
            sqlite3_bind_int(res, currentParameter++, i >> 1); // Divide i by 2
            sqlite3_bind_int(res, currentParameter++, coords[i]);
            sqlite3_bind_int(res, currentParameter++, coords[i + 1]);
        }
    }
    else
    {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    int step = sqlite3_step(res);
    if (step == SQLITE_ROW)
    {
        lastTile = (Tile *)malloc(sizeof(Tile));
        lastTile->z = sqlite3_column_int(res, 0);
        lastTile->x = sqlite3_column_int(res, 1);
        lastTile->y = sqlite3_column_int(res, 2);
        lastTile->blob = strdup(sqlite3_column_blob(res, 3));
        lastTile->blobSize = strlen(lastTile->blob);
    }
    else if (step != SQLITE_DONE)
    {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    sqlite3_finalize(res);
    return lastTile;
}

Tile *getLastExistingTile(int x, int y, int zoomLevel, sqlite3 *db, char *tableName)
{
    int coords[X_Y_COORDS_FOR_ALL_ZOOM_LEVELS];
    for (int i = 0; i < X_Y_COORDS_FOR_ALL_ZOOM_LEVELS; i++)
    {
        coords[i] = -1;
    }

    int baseTileX = x, baseTileY = y, arrayIterator;
    for (int i = zoomLevel - 1; i >= 0; i--)
    {
        baseTileX = baseTileX >> 1; // Divide by 2
        baseTileY = baseTileY >> 1; // Divide by 2
        arrayIterator = i << 1;     // Multiply by 2
        coords[arrayIterator] = baseTileX;
        coords[arrayIterator + 1] = baseTileY;
    }

    char query[SQL_QUERY_SIZE] = "";
    lastTileSQLQuery(query, tableName);

    Tile *lastTile = bindParametersToQueryAndExecute(db, query, coords);

    return lastTile;
}

void upscale(MagickWand *baseWand, Tile *baseTile, int newZoomLevel, int newX, int newY)
{
    // Alogirthm for resizing
    PixelInterpolateMethod filterAlg = BilinearInterpolatePixel;
    MagickBooleanType status;

    int zoomLevelDiff = newZoomLevel - baseTile->z,
        tileScale = 2 << (zoomLevelDiff - 1), // 2 to the power of (zoomLevelDiff - 1),
        xOffset = newX % tileScale,
        yOffset = newY % tileScale,
        targetWidth = TILE_WIDTH / tileScale,
        targetHeight = TILE_HEIGHT / tileScale;

    status = MagickCropImage(baseWand, targetWidth, targetHeight, xOffset * targetWidth, yOffset * targetHeight);
    if (status == MagickFalse)
    {
        handleError("Could not crop tile", baseWand);
    }
    status = MagickInterpolativeResizeImage(baseWand, TILE_WIDTH, TILE_HEIGHT, filterAlg);
    if (status == MagickFalse)
    {
        handleError("Could not resize tile", baseWand);
    }
}
