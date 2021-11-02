#include "upscaling.h"

#define TILE_WIDTH 256
#define TILE_HEIGHT 256
#define ZOOM_LEVEL_COUNT 25
#define X_Y_COORDS_FOR_ALL_ZOOM_LEVELS ZOOM_LEVEL_COUNT << 1 // Multiply by 2
#define SQL_QUERY_SIZE 2000
#define MAX(a, b) ((a) > (b) ? a : b)

void lastTileSQLQuery(char *query, char *tableName, int zoomLevel)
{
    strcat(query, "SELECT zoom_level, tile_column, tile_row, hex(tile_data) FROM");
    strcat(query, " ");
    strcat(query, tableName);
    strcat(query, " WHERE ");

    const int maxZoomLevel = zoomLevel - 1;
    for (int currentZoomLevel = 0; currentZoomLevel < zoomLevel; currentZoomLevel++)
    {
        char *sql;
        if (currentZoomLevel == maxZoomLevel)
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

Tile *bindParametersToQueryAndExecute(sqlite3 *db, char *query, int coords[], int zoomLevel)
{
    Tile *lastTile = NULL;
    sqlite3_stmt *res;
    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);

    if (rc == SQLITE_OK)
    {
        int currentParameter = 1, arrayIdx;
        //for (int i = 0; i < X_Y_COORDS_FOR_ALL_ZOOM_LEVELS; i += 2)
        for (int i = zoomLevel - 1; i >= 0; i--)
        {
            arrayIdx = i << 1; // Multiply by 2
            sqlite3_bind_int(res, currentParameter++, i);
            sqlite3_bind_int(res, currentParameter++, coords[arrayIdx]);
            sqlite3_bind_int(res, currentParameter++, coords[arrayIdx + 1]);
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
    lastTileSQLQuery(query, tableName, zoomLevel);

    Tile *lastTile = bindParametersToQueryAndExecute(db, query, coords, zoomLevel);

    return lastTile;
}

void upscale(MagickWand *baseWand, Tile *baseTile, int newZoomLevel, int newX, int newY)
{
    // Alogirthm for resizing
    PixelInterpolateMethod filterAlg = BilinearInterpolatePixel;
    MagickBooleanType status;

    int zoomLevelDiff = newZoomLevel - baseTile->z;
    int scale = 1 << zoomLevelDiff;
    double scaleAsDouble = (double)scale;

    double tilePartX = (newX % scale) / scaleAsDouble;
    double tilePartY = (newY % scale) / scaleAsDouble;
    double tileSize = TILE_HEIGHT / scaleAsDouble;

    int pixelX = (int)(tilePartX * TILE_WIDTH),
        pixelY = (int)(tilePartY * TILE_HEIGHT),
        imageWidth = MAX((int)tileSize, 1),
        imageHeight = MAX((int)tileSize, 1);

    status = MagickCropImage(baseWand, imageWidth, imageHeight, pixelX, pixelY);
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
