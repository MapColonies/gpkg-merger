#include "gpkg.h"

#define MIN_ZOOM_QUERY "SELECT MIN(zoom_level) FROM gpkg_tile_matrix"
#define MAX_ZOOM_QUERY "SELECT MAX(zoom_level) FROM gpkg_tile_matrix"
#define CACHE_NAME_QUERY "SELECT table_name FROM gpkg_contents"
#define TILE_MATRIX_QUERY "SELECT * FROM gpkg_tile_matrix"

typedef struct TileMatrix
{
    char *tableName;
    int zoomLevel;
    int matrixWidth;
    int matrixHeight;
    int tileWidth;
    int tileHeight;
    double pixleXSize;
    double pixleYSize;
} TileMatrix;

void insertTile(sqlite3 *db, char *tileCache, Tile *tile)
{
    char *tileInsertQuery = (char *)malloc(50000 * sizeof(char));
    sprintf(tileInsertQuery, "REPLACE INTO %s (zoom_level, tile_column, tile_row, tile_data) VALUES (?, ?, ?, ?)", tileCache);
    // char *sql = "REPLACE INTO %s (zoom_level, tile_column, tile_row, tile_data) VALUES (?, ?, ?, ?)";
    // return sql;

    // char *tileInsertQuery = getTileInsertQuery(tileCache, tile);
    sqlite3_stmt *stmt = prepareStatement(db, tileInsertQuery);
    sqlite3_bind_int(stmt, 1, tile->z);
    sqlite3_bind_int(stmt, 2, tile->x);
    sqlite3_bind_int(stmt, 3, tile->y);
    sqlite3_bind_blob(stmt, 4, tile->blob, strlen(tile->blob), SQLITE_TRANSIENT);
    executeStatementSingleColResult(db, tileInsertQuery);
    free(tileInsertQuery);
    finalizeStatement(stmt);
}

char *getTileMatrixInsertQuery(char *tileCache, TileMatrix *tileMatrix)
{
    char *sql = (char *)malloc(500 * sizeof(char));
    sprintf(sql, "REPLACE INTO gpkg_tile_matrix (table_name, zoom_level, matrix_width, matrix_height, tile_width, tile_height, pixel_x_size, pixel_y_size) VALUES ('%s', %d, %d, %d, %d, %d, %0.20f, %0.20f)",
            tileCache, tileMatrix->zoomLevel, tileMatrix->matrixWidth, tileMatrix->matrixHeight, tileMatrix->tileWidth, tileMatrix->tileHeight, tileMatrix->pixleXSize, tileMatrix->pixleYSize);
    return sql;
}

// char *getBatchInsertQuery(char *tileCache, int currentOffset, int batchSize)
// {
//     char *sql = (char *)malloc(200 * sizeof(char));
//     sprintf(sql, "SELECT zoom_level, tile_column, tile_row, hex(tile_data) FROM %s limit %d offset %d", tileCache, batchSize, currentOffset);
//     return sql;
// }

void readTileCacheLevel(Gpkg *gpkg)
{
    const char *tileCache = executeStatementSingleColResult(gpkg->db, CACHE_NAME_QUERY);
    // TODO: check if failed
    gpkg->tileCache = strdup(tileCache);
}

void readMinZoomLevel(Gpkg *gpkg)
{
    const char *minZoom = executeStatementSingleColResult(gpkg->db, MIN_ZOOM_QUERY);
    // TODO: check if failed
    gpkg->minZoom = atoi(minZoom);
}

void readMaxZoomLevel(Gpkg *gpkg)
{
    const char *maxZoom = executeStatementSingleColResult(gpkg->db, MAX_ZOOM_QUERY);
    // TODO: check if failed
    gpkg->maxZoom = atoi(maxZoom);
}

void openGpkg(Gpkg *gpkg)
{
    int rc = sqlite3_open(gpkg->path, &gpkg->db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(gpkg->db));
        closeGpkg(gpkg);
    }
}

Gpkg *readGpkgInfo(char *path)
{
    Gpkg *gpkg = (Gpkg *)malloc(sizeof(Gpkg));
    gpkg->path = path;

    openGpkg(gpkg);
    readTileCacheLevel(gpkg);
    readMinZoomLevel(gpkg);
    readMaxZoomLevel(gpkg);

    return gpkg;
}

void copyTileMatrix(Gpkg *baseGpkg, Gpkg *newGpkg)
{
    sqlite3_stmt *stmt = prepareStatement(newGpkg->db, TILE_MATRIX_QUERY);
    int rc = sqlite3_step(stmt);
    do
    {
        TileMatrix *tileMatrix = (TileMatrix *)malloc(sizeof(TileMatrix));
        tileMatrix->zoomLevel = sqlite3_column_int(stmt, 1);
        tileMatrix->matrixWidth = sqlite3_column_int(stmt, 2);
        tileMatrix->matrixHeight = sqlite3_column_int(stmt, 3);
        tileMatrix->tileWidth = sqlite3_column_int(stmt, 4);
        tileMatrix->tileHeight = sqlite3_column_int(stmt, 5);
        tileMatrix->pixleXSize = sqlite3_column_double(stmt, 6);
        tileMatrix->pixleYSize = sqlite3_column_double(stmt, 7);

        char *query = getTileMatrixInsertQuery(baseGpkg->tileCache, tileMatrix);
        executeStatementSingleColResult(baseGpkg->db, query);

        free(tileMatrix);
        rc = sqlite3_step(stmt);
    } while (rc == SQLITE_ROW);
}

void insertTileBatch(TileBatch *tileBatch, sqlite3 *db, char *tileCache)
{
    for (int i = 0; i < tileBatch->size; i++)
    {
        Tile *tile = getNextTile(tileBatch);

        Tile *baseTile = getTile(db, tileCache, tile->z, tile->x, tile->y);
        // printTile(baseTile);

        // Merge tiles if tile exists in base gpkg
        if (baseTile->blob != NULL)
        {
            char *pythonMerge = (char *)malloc(100000 * sizeof(char));
            sprintf(pythonMerge, "python3 /home/shimon/Documents/gpkg-merger/src/merge.py %s %s", baseTile->blob, tile->blob);
            FILE *fp = popen(pythonMerge, "r");

            if (fp == NULL)
            {
                printf("Failed to run merge command");
                exit(1);
            }

            char combinedBlob[100000];
            // fgets(combinedBlob, sizeof(combinedBlob), fp);

            while (fgets(combinedBlob, sizeof(combinedBlob), fp) != NULL)
                ;

            // printf("%s\n", combinedBlob);
            free(tile->blob);
            tile->blob = combinedBlob;
            pclose(fp);
        }

        printf("yessss %s\n", tile->blob);

        insertTile(db, tileCache, tile);

        // char *tileInsertQuery = getTileInsertQuery(tileCache, tile);
        // sqlite3_stmt *stmt = prepareStatement(db, tileInsertQuery);
        // sqlite3_bind_int(stmt, 1, tile->z);
        // sqlite3_bind_int(stmt, 2, tile->x);
        // sqlite3_bind_int(stmt, 3, tile->y);
        // sqlite3_bind_blob(stmt, 4, tile->blob, strlen(tile->blob), SQLITE_TRANSIENT);
        // executeStatementSingleColResult(db, tileInsertQuery);
        // free(tileInsertQuery);
        // finalizeStatement(stmt);
    }
}

void mergeGpkgs(Gpkg *baseGpkg, Gpkg *newGpkg, int batchSize)
{
    TileBatch *tileBatch;
    sqlite3 *baseDb = baseGpkg->db;
    copyTileMatrix(baseGpkg, newGpkg);

    do
    {
        tileBatch = getTileBatch(newGpkg->db, newGpkg->tileCache, batchSize, newGpkg->current);
        // printBatch(tileBatch);
        insertTileBatch(tileBatch, baseDb, baseGpkg->tileCache);
    } while (tileBatch->current != tileBatch->size);
}

void closeGpkg(Gpkg *gpkg)
{
    sqlite3_close(gpkg->db);
    free(gpkg);
}

void printGpkgInfo(Gpkg *gpkg)
{
    printf("Path: %s\n", gpkg->path);
    printf("Min zoom: %d\n", gpkg->minZoom);
    printf("Max zoom: %d\n", gpkg->maxZoom);
    printf("Current position: %d\n", gpkg->current);
}
