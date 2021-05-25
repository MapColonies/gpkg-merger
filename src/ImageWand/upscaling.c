#include "upscaling.h"
#include "wandUtil.h"

#define TILE_WIDTH 256
#define TILE_HEIGHT 256
#define ZOOM_LEVEL_COUNT 25
#define X_Y_COORDS_FOR_ALL_ZOOM_LEVELS ZOOM_LEVEL_COUNT << 1 // Multiply by 2
#define SQL_QUERY_SIZE 2000
#define SQL_SUBQUERY_SIZE 80

void lastTileSQLQuery(char *query, char *tableName)
{
    strcat(query, "SELECT zoom_level, tile_row, tile_column, hex(tile_data) FROM");
    strcat(query, " ");
    strcat(query, tableName);
    strcat(query, " WHERE ");

    for (int currentZoomLevel = 0; currentZoomLevel < ZOOM_LEVEL_COUNT; currentZoomLevel++)
    {
        char sql[SQL_SUBQUERY_SIZE] = "";
        if (currentZoomLevel == ZOOM_LEVEL_COUNT - 1)
        {
            snprintf(sql, SQL_SUBQUERY_SIZE, "(zoom_level = %d AND tile_row = ? AND tile_column = ?) ", currentZoomLevel);
        }
        else
        {
            snprintf(sql, SQL_SUBQUERY_SIZE, "(zoom_level = %d AND tile_row = ? AND tile_column = ?) OR ", currentZoomLevel);
        }
        strcat(query, sql);
    }

    strcat(query, "ORDER BY zoom_level DESC LIMIT 1");
}

void bindParametersToQueryAndExecute(Tile *lastTile, sqlite3 *db, char *query, int coords[])
{
    sqlite3_stmt *res;
    // int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);
    char *lol = "SELECT zoom_level, tile_row, tile_column, hex(tile_data) FROM O_ihud_w84geo_Nov04_Apr20_gpkg_15_0 WHERE \
(zoom_level = 0 AND tile_column = ? AND tile_row = ?) OR (zoom_level = 1 AND tile_column = ? AND tile_row = ?) OR \
(zoom_level = 2 AND tile_column = ? AND tile_row = ?) OR (zoom_level = 3 AND tile_column = ? AND tile_row = ?) OR \
(zoom_level = 4 AND tile_column = ? AND tile_row = ?) OR (zoom_level = 5 AND tile_column = ? AND tile_row = ?) OR \
(zoom_level = 6 AND tile_column = ? AND tile_row = ?) OR (zoom_level = 7 AND tile_column = ? AND tile_row = ?) OR \
(zoom_level = 8 AND tile_column = ? AND tile_row = ?) OR (zoom_level = 9 AND tile_column = ? AND tile_row = ?) OR \
(zoom_level = 10 AND tile_column = ? AND tile_row = ?) OR (zoom_level = 11 AND tile_column = ? AND tile_row = ?) OR \
(zoom_level = 12 AND tile_column = ? AND tile_row = ?) OR (zoom_level = 13 AND tile_column = ? AND tile_row = ?) OR \
(zoom_level = 14 AND tile_column = ? AND tile_row = ?) OR (zoom_level = 15 AND tile_column = ? AND tile_row = ?) OR \
(zoom_level = 16 AND tile_column = ? AND tile_row = ?) OR (zoom_level = 17 AND tile_column = ? AND tile_row = ?) OR \
(zoom_level = 18 AND tile_column = ? AND tile_row = ?) OR (zoom_level = 19 AND tile_column = ? AND tile_row = ?) OR \
(zoom_level = 20 AND tile_column = ? AND tile_row = ?) OR (zoom_level = 21 AND tile_column = ? AND tile_row = ?) OR \
(zoom_level = 22 AND tile_column = ? AND tile_row = ?) OR (zoom_level = 23 AND tile_column = ? AND tile_row = ?) OR \
(zoom_level = 24 AND tile_column = ? AND tile_row = ?) ORDER BY zoom_level DESC LIMIT 1";

    int rc = sqlite3_prepare_v2(db, lol, -1, &res, 0);
    if (rc == SQLITE_OK)
    {
        int currentParameter = 1;
        for (int i = 0; i < X_Y_COORDS_FOR_ALL_ZOOM_LEVELS; i++, currentParameter++)
        {
            sqlite3_bind_int(res, currentParameter, coords[i]);
        }
    }
    else
    {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }
    // printf("%s\n", sqlite3_expanded_sql(res));
    int step = sqlite3_step(res);
    if (step == SQLITE_ROW)
    {
        // printf("%d\n", sqlite3_column_int(res, 0));
        lastTile->z = sqlite3_column_int(res, 0);
        lastTile->x = sqlite3_column_int(res, 1);
        lastTile->y = sqlite3_column_int(res, 2);
        lastTile->blob = strdup(sqlite3_column_blob(res, 3));
        lastTile->blobSize = strlen(lastTile->blob);
    }
    else
    {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(res);
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

    Tile *lastTile = (Tile *)malloc(sizeof(Tile));
    bindParametersToQueryAndExecute(lastTile, db, query, coords);

    return lastTile;
}

MagickWand *upscale(int zoomLevel, int x, int y, sqlite3 *db, char *tableName, Tile *lastExistingTile)
{
    // Alogirthm for resizing
    PixelInterpolateMethod filterAlg = BilinearInterpolatePixel;
    MagickBooleanType status;
    MagickWand *wand = NewMagickWand();

    int zoomLevelDiff = zoomLevel - lastExistingTile->z,
        tileScale = zoomLevelDiff * 2,
        xOffset = x % tileScale,
        yOffset = y % tileScale,
        resizeCount = 2 << (zoomLevelDiff - 1), // 2 to the power of (zoomLevelDiff - 1),
        targetWidth = TILE_WIDTH / tileScale,
        targetHeight = TILE_HEIGHT / tileScale;

    createWandFromHex(wand, lastExistingTile->blob);

    status = MagickCropImage(wand, targetWidth, targetHeight, xOffset * targetWidth, yOffset * targetHeight);
    if (status == MagickFalse)
    {
        handleError("Could not crop tile", wand);
    }
    status = MagickInterpolativeResizeImage(wand, TILE_WIDTH, TILE_HEIGHT, filterAlg);
    if (status == MagickFalse)
    {
        handleError("Could not resize tile", wand);
    }
    // char fileName[10] = "dst.png";
    // status = MagickWriteImage(wand, fileName);
    // if (status == MagickFalse)
    // {
    //     handleError("Could not write image to file", wand);
    // }

    // TODO: free lastExistingTile;
    return wand;
}

int main()
{
    MagickWandGenesis();
    char *baseGpkgPath = realpath("/home/roees/Documents/gpkgs/artzi_fixed.gpkg", NULL);
    Gpkg *baseGpkg = readGpkgInfo(baseGpkgPath);
    char *newGpkgPath = realpath("/home/roees/Documents/gpkgs/area1_small.gpkg", NULL);
    Gpkg *newGpkg = readGpkgInfo(newGpkgPath);
    mergeGpkgs(baseGpkg, newGpkg, 1000);

    // thisTile->blobSize = strlen(thisTile->blob);

    // printf("%d\n", thisTile->blobSize);

    // printTile(thisTile);

    // char *hex = mergeNewToBase(thisTile, baseGpkg->db, baseGpkg->tileCache);
    // printf("%s\n", hex);

    // MagickWand *wand = NewMagickWand();
    // createWandFromHex(wand, thisTile->blob);
    // printf("%d\n", wandHasAlpha(wand));

    closeGpkg(baseGpkg);
    MagickWandTerminus();
    return 0;
}