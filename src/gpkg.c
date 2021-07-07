#include "gpkg.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

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

pthread_mutex_t insertTileLock;
int countAll = 0;
int countTiles = 0;

char *getTileInsertQuery(char *tileCache, Tile *tile)
{
    int allocationSize = tile->blobSize * 2 + QUERY_SIZE;
    char *sql = (char *)malloc(allocationSize * sizeof(char));
    sprintf(sql, "REPLACE INTO %s (zoom_level, tile_column, tile_row, tile_data) VALUES (%d, %d, %d, x'%s')", tileCache, tile->z, tile->x, tile->y, tile->blob);
    return sql;
}

Extent *getExtent(sqlite3 *db)
{
    sqlite3_stmt *stmt = prepareStatement(db, EXTENT_QUERY, 0);

    int minX, minY, maxX, maxY;
    Extent *extent = (Extent *)malloc(sizeof(Extent));

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        extent->minX = sqlite3_column_double(stmt, 0);
        extent->minY = sqlite3_column_double(stmt, 1);
        extent->maxX = sqlite3_column_double(stmt, 2);
        extent->maxY = sqlite3_column_double(stmt, 3);
    }
    else
    {
        return NULL;
    }

    finalizeStatement(stmt);
    return extent;
}

void insertTile(sqlite3 *db, char *tileCache, Tile *tile)
{
    char *tileInsertQuery = getTileInsertQuery(tileCache, tile);
    sqlite3_stmt *stmt = prepareStatement(db, tileInsertQuery, 0);
    executeQuerySingleColResult(db, tileInsertQuery);

    free(tileInsertQuery);
    finalizeStatement(stmt);
}

void addIndex(sqlite3 *db, char *tileCache)
{
    char *query = getAddIndexQuery(tileCache);
    char *res = executeQuerySingleColResult(db, query);
    free(res);
    free(query);
}

sqlite3_stmt *getTileMatrixInsertStmt(sqlite3 *db)
{
    sqlite3_stmt *stmt = prepareStatement(db, INSERT_TILE_MATRIX_QUERY, SQLITE_PREPARE_PERSISTENT);
    return stmt;
}

void *bindTileCache(sqlite3_stmt *stmt, char *tileCache, TileMatrix *tileMatrix)
{
    sqlite3_reset(stmt);
    sqlite3_bind_text(stmt, 1, tileCache, -1, NULL);
    sqlite3_bind_int(stmt, 2, tileMatrix->zoomLevel);
    sqlite3_bind_int(stmt, 3, tileMatrix->matrixWidth);
    sqlite3_bind_int(stmt, 4, tileMatrix->matrixHeight);
    sqlite3_bind_int(stmt, 5, tileMatrix->tileWidth);
    sqlite3_bind_int(stmt, 6, tileMatrix->tileHeight);
    sqlite3_bind_double(stmt, 7, tileMatrix->pixleXSize);
    sqlite3_bind_double(stmt, 8, tileMatrix->pixleYSize);
}

char *readTileCache(sqlite3 *db)
{
    char *tileCache = executeQuerySingleColResult(db, CACHE_NAME_QUERY);
    // TODO: check if failed
    return tileCache;
}

int readMinZoomLevel(sqlite3 *db)
{
    char *minZoomString = executeQuerySingleColResult(db, MIN_ZOOM_QUERY);
    // TODO: check if failed
    int minZoom = atoi(minZoomString);
    free(minZoomString);
    return minZoom;
}

int readMaxZoomLevel(sqlite3 *db)
{
    char *maxZoomString = executeQuerySingleColResult(db, MAX_ZOOM_QUERY);
    // TODO: check if failed
    int maxZoom = atoi(maxZoomString);
    free(maxZoomString);
    return maxZoom;
}

void updateExtent(sqlite3 *baseDb, sqlite3 *newDb)
{
    Extent *baseExtent = getExtent(baseDb);
    Extent *newExtent = getExtent(newDb);

    baseExtent->minX = MIN(baseExtent->minX, newExtent->minX);
    baseExtent->maxX = MAX(baseExtent->maxX, newExtent->maxX);
    baseExtent->minY = MIN(baseExtent->minY, newExtent->minY);
    baseExtent->maxY = MAX(baseExtent->maxY, newExtent->maxY);

    sqlite3_stmt *stmt = getExtentInsertStmt(baseDb, baseExtent);
    char *res = executeStatementSingleColResult(baseDb, stmt, 1);
    free(res);
    free(baseExtent);
    free(newExtent);
}

// void convertIfBase1ToBase2(Gpkg *gpkg)
// {
//     // TODO: convert
//     char *query = executeQuerySingleColResult(gpkg->db, BASE_ZOOM_QUERY);
//     free(query);
// }

sqlite3 *openGpkg(char *path, int openMode)
{
    sqlite3 *db;
    int rc = sqlite3_open_v2(path, &db, openMode, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s, error: %s\n", path, sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}

Gpkg *readGpkgInfo(char *path)
{
    Gpkg *gpkg = (Gpkg *)malloc(sizeof(Gpkg));
    gpkg->path = path;

    sqlite3 *db = openGpkg(path, SQLITE_OPEN_READONLY);
    gpkg->tileCache = readTileCache(db);
    gpkg->minZoom = readMinZoomLevel(db);
    gpkg->maxZoom = readMaxZoomLevel(db);
    sqlite3_close(db);
    gpkg->current = 0;

    return gpkg;
}

void copyTileMatrix(sqlite3 *baseDb, sqlite3 *newDb, char *baseTileCache)
{
    sqlite3_stmt *stmt = prepareStatement(newDb, TILE_MATRIX_QUERY, 0);
    sqlite3_stmt *matrixInsertStmt = getTileMatrixInsertStmt(baseDb);
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

        bindTileCache(matrixInsertStmt, baseTileCache, tileMatrix);

        char *res = executeStatementSingleColResult(baseDb, matrixInsertStmt, 0);
        free(res);

        free(tileMatrix);
        rc = sqlite3_step(stmt);
    } while (rc == SQLITE_ROW);

    finalizeStatement(matrixInsertStmt);
}

void mergeTileBatch(TileBatch *tileBatch, TileBatch *baseTileBatch)
{
    for (int i = 0; i < tileBatch->size; i++)
    {
        Tile *tile = getNextTile(tileBatch);
        Tile *baseTile = getNextTile(baseTileBatch);

        // Merge tiles if tile exists in base gpkg
        if (baseTile != NULL)
        {
            char *blob = mergeNewToBase(tile, baseTile);
            // If returned blob is not same blob (merge was required)
            if (tile->blob != blob)
            {
                free(tile->blob);
                tile->blob = blob;
                tile->blobSize = strlen(blob);
            }
        }
    }
    baseTileBatch->current = 0;
    tileBatch->current = 0;
}

void insertTileBatch(TileBatch *tileBatch, sqlite3 *db, char *tileCache)
{
    for (int i = 0; i < tileBatch->size; i++)
    {
        Tile *tile = getNextTile(tileBatch);
        insertTile(db, tileCache, tile);
    }
    tileBatch->current = 0;
}

void printFinishedBatch()
{
    printf("Merged %d/%d tiles\n", countTiles, countAll);
}

void work(void **args)
{
    TileBatch *baseTileBatch = (TileBatch *)args[0];
    TileBatch *tileBatch = (TileBatch *)args[1];
    char *baseGpkgPath = (char *)args[2];
    char *tileCache = (char *)args[3];
    int *batchSize = (int *)args[4];

    int count = 0;
    int size = 0;

    mergeTileBatch(tileBatch, baseTileBatch);

    // Insert batch
    pthread_mutex_lock(&insertTileLock);
    sqlite3 *baseDb = openGpkg(baseGpkgPath, SQLITE_OPEN_READWRITE);
    insertTileBatch(tileBatch, baseDb, tileCache);
    countTiles = countTiles + tileBatch->size;
    printFinishedBatch();
    sqlite3_close(baseDb);
    pthread_mutex_unlock(&insertTileLock);
    freeBatch(baseTileBatch);
    freeBatch(tileBatch);
}

void mergeGpkgsNoThreads(Gpkg *baseGpkg, Gpkg *newGpkg, int batchSize)
{
    sqlite3 *baseDb = openGpkg(baseGpkg->path, SQLITE_OPEN_READWRITE);
    sqlite3 *newDb = openGpkg(newGpkg->path, SQLITE_OPEN_READONLY);
    copyTileMatrix(baseDb, newDb, baseGpkg->tileCache);
    updateExtent(baseDb, newDb);

    char *tileCountQuery = getTileCountQuery(newGpkg->tileCache);
    char *countAllString = executeQuerySingleColResult(newDb, tileCountQuery);

    countAll = atoi(countAllString);
    free(tileCountQuery);
    printf("Working on merging %d tile into base gpkg\n", countAll);
    int count = 0;
    int size = 0;

    int amount = countAll / batchSize;
    if (countAll % batchSize != 0)
    {
        amount++;
    }

    // Initialize prepared statements
    sqlite3_stmt *getBatchStmt = getBatchSelectStmt(newDb, newGpkg->tileCache);
    sqlite3_stmt *getBlobSizeStmt = getBlobSizeSelectStmt(baseDb, baseGpkg->tileCache);
    sqlite3_stmt *getTileStmt = getTileSelectStmt(baseDb, baseGpkg->tileCache);

    int tilesUntilVacuum = getVacuumCount();
    for (int i = 0; i < amount; i++)
    {
        TileBatch *tileBatch = getTileBatch(getBatchStmt, batchSize, newGpkg->current);
        size = tileBatch->size;
        count += size;
        tilesUntilVacuum -= size;
        newGpkg->current = count;

        TileBatch *baseTileBatch = getCorrespondingBatch(tileBatch, baseDb, getTileStmt, getBlobSizeStmt, baseGpkg->tileCache);

        mergeTileBatch(tileBatch, baseTileBatch);
        insertTileBatch(tileBatch, baseDb, baseGpkg->tileCache);

        freeBatch(baseTileBatch);
        freeBatch(tileBatch);

        printf("Merged %d/%d tiles\n", count, countAll);

        // Check if should vacuum target DB
        if (tilesUntilVacuum <= 0)
        {
            vacuum(baseDb);
            tilesUntilVacuum = getVacuumCount();
        }
    }
    finalizeStatement(getBatchStmt);
    finalizeStatement(getBlobSizeStmt);
    finalizeStatement(getTileStmt);
    vacuum(baseDb);
    sqlite3_close(baseDb);
    sqlite3_close(newDb);

    // Add tile index
    baseDb = openGpkg(baseGpkg->path, SQLITE_OPEN_READWRITE);
    addIndex(baseDb, baseGpkg->tileCache);
    sqlite3_close(baseDb);
    free(countAllString);
}

// void mergeGpkgs(Gpkg *baseGpkg, Gpkg *newGpkg, int batchSize)
// {
//     sqlite3 *baseDb = openGpkg(baseGpkg->path, SQLITE_OPEN_READWRITE);
//     sqlite3 *newDb = openGpkg(newGpkg->path, SQLITE_OPEN_READONLY);
//     copyTileMatrix(baseDb, newDb, baseGpkg->tileCache);
//     updateExtent(baseDb, newDb);
//     sqlite3_close(baseDb);

//     char *tileCountQuery = getTileCountQuery(newGpkg->tileCache);
//     char *countAllString = executeQuerySingleColResult(newDb, tileCountQuery);

//     countAll = atoi(countAllString);
//     free(tileCountQuery);
//     printf("Working on merging %d tile into base gpkg\n", countAll);
//     int count = 0;
//     int size = 0;

//     pthread_mutex_init(&insertTileLock, NULL);
//     threadpool threadPool = thpool_init(1);

//     int amount = countAll / batchSize;
//     if (countAll % batchSize != 0)
//     {
//         amount++;
//     }
//     for (int i = 0; i < amount; i++)
//     {
//         TileBatch *tileBatch = getTileBatch(newDb, newGpkg->tileCache, batchSize, newGpkg->current);
//         size = tileBatch->size;
//         count += size;
//         newGpkg->current = count;

//         pthread_mutex_lock(&insertTileLock);
//         baseDb = openGpkg(baseGpkg->path, SQLITE_OPEN_READONLY);
//         TileBatch *baseTileBatch = getCorrespondingBatch(tileBatch, baseDb, baseGpkg->tileCache);

//         sqlite3_close(baseDb);
//         pthread_mutex_unlock(&insertTileLock);

//         void *args[] = {baseTileBatch, tileBatch, baseGpkg->path, baseGpkg->tileCache, &size};
//         // TODO: FIX THREADS
//         thpool_add_work(threadPool, work, args);
//     }

//     sqlite3_close(newDb);
//     thpool_wait(threadPool);
//     thpool_destroy(threadPool);
//     pthread_mutex_destroy(&insertTileLock);

//     // Add tile index
//     baseDb = openGpkg(baseGpkg->path, SQLITE_OPEN_READWRITE);
//     addIndex(baseDb, baseGpkg->tileCache);
//     sqlite3_close(baseDb);
//     free(countAllString);
// }

void closeGpkg(Gpkg *gpkg)
{
    free(gpkg->path);
    free(gpkg->tileCache);
    free(gpkg);
}

void printGpkgInfo(Gpkg *gpkg)
{
    printf("Path: %s\n", gpkg->path);
    printf("Min zoom: %d\n", gpkg->minZoom);
    printf("Max zoom: %d\n", gpkg->maxZoom);
    printf("Current position: %ld\n", gpkg->current);
}
