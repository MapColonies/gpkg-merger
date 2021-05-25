#include "gpkg.h"

#define MIN_ZOOM_QUERY "SELECT MIN(zoom_level) FROM gpkg_tile_matrix"
#define MAX_ZOOM_QUERY "SELECT MAX(zoom_level) FROM gpkg_tile_matrix"
#define CACHE_NAME_QUERY "SELECT table_name FROM gpkg_contents"
#define TILE_MATRIX_QUERY "SELECT * FROM gpkg_tile_matrix"
#define BASE_ZOOM_QUERY "SELECT zoom_level FROM gpkg_tile_matrix where zoom_level=0 and matrix_width=1 and matrix_height=1"
#define EXTENT_QUERY "SELECT min_x, min_y, max_x, max_y FROM gpkg_contents"

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

typedef struct Extent
{
    double minX;
    double minY;
    double maxX;
    double maxY;
} Extent;

// pthread_mutex_t getTileLock;
pthread_mutex_t insertTileLock;
int countAll = 0;
int countTiles = 0;

int max(double num1, double num2)
{
    return (num1 > num2) ? num1 : num2;
}

int min(double num1, double num2)
{
    return (num1 > num2) ? num2 : num1;
}

char *getAddIndexQuery(char *tileCache)
{
    char *sql = (char *)malloc(500 * sizeof(char));
    sprintf(sql, "CREATE UNIQUE INDEX IF NOT EXISTS index_tiles on %s (zoom_level, tile_row, tile_column)", tileCache);
    return sql;
}

char *getTileInsertQuery(char *tileCache, Tile *tile)
{
    // char *blobAllocation = getenv("BLOB_MEMORY_ALLOCATION");
    // int allocationSize = atoi(blobAllocation);
    // printf("Size: %d\n", tile->blobSize);

    // We allocate 2 time the size of the blob because it has HEX representation, so it takes two times the size when represented as string
    int allocationSize = tile->blobSize * 2 + 1000;
    char *sql = (char *)malloc(allocationSize * sizeof(char));
    // char *sql = (char *)malloc(allocationSize * sizeof(char));
    sprintf(sql, "REPLACE INTO %s (zoom_level, tile_column, tile_row, tile_data) VALUES (%d, %d, %d, x'%s')", tileCache, tile->z, tile->x, tile->y, tile->blob);
    return sql;
}

char *getTileCountQuery(char *tileCache)
{
    char *sql = (char *)malloc(500 * sizeof(char));
    sprintf(sql, "SELECT COUNT(*) FROM %s", tileCache);
    return sql;
}

char *getExtentInsertQuery(Extent *extent)
{
    char *sql = (char *)malloc(500 * sizeof(char));
    sprintf(sql, "UPDATE gpkg_contents SET min_x=%0.20f, max_x=%0.20f, min_y=%0.20f, max_y=%0.20f", extent->minX, extent->maxX, extent->minY, extent->maxY);
    return sql;
}

Extent *getExtent(sqlite3 *db)
{
    sqlite3_stmt *stmt = prepareStatement(db, EXTENT_QUERY);

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
    sqlite3_stmt *stmt = prepareStatement(db, tileInsertQuery);
    // char *res = executeStatementSingleColResult(db, tileInsertQuery);
    executeStatementSingleColResult(db, tileInsertQuery);
    // printf("res: %s", res);

    free(tileInsertQuery);
    // free(res);
    finalizeStatement(stmt);
}

void addIndex(sqlite3 *db, char *tileCache)
{
    char *query = getAddIndexQuery(tileCache);
    char *res = executeStatementSingleColResult(db, query);
    free(res);
    free(query);
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

char *readTileCache(sqlite3 *db)
{
    char *tileCache = executeStatementSingleColResult(db, CACHE_NAME_QUERY);
    // TODO: check if failed
    return tileCache;
}

int readMinZoomLevel(sqlite3 *db)
{
    char *minZoomString = executeStatementSingleColResult(db, MIN_ZOOM_QUERY);
    // TODO: check if failed
    int minZoom = atoi(minZoomString);
    free(minZoomString);
    return minZoom;
}

int readMaxZoomLevel(sqlite3 *db)
{
    char *maxZoomString = executeStatementSingleColResult(db, MAX_ZOOM_QUERY);
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

    char *query = getExtentInsertQuery(baseExtent);
    char *res = executeStatementSingleColResult(baseDb, query);
    free(res);
    free(query);
    free(baseExtent);
    free(newExtent);
}

// void convertIfBase1ToBase2(Gpkg *gpkg)
// {
//     // TODO: convert
//     char *query = executeStatementSingleColResult(gpkg->db, BASE_ZOOM_QUERY);
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
    // if (db == NULL)
    // {
    //     return NULL;
    // }

    // gpkg->db = db;
    gpkg->tileCache = readTileCache(db);
    gpkg->minZoom = readMinZoomLevel(db);
    gpkg->maxZoom = readMaxZoomLevel(db);
    sqlite3_close(db);
    gpkg->current = 0;

    return gpkg;
}

void copyTileMatrix(sqlite3 *baseDb, sqlite3 *newDb, char *baseTileCache)
{
    sqlite3_stmt *stmt = prepareStatement(newDb, TILE_MATRIX_QUERY);
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

        char *query = getTileMatrixInsertQuery(baseTileCache, tileMatrix);
        char *res = executeStatementSingleColResult(baseDb, query);
        free(res);
        free(query);

        free(tileMatrix);
        rc = sqlite3_step(stmt);
    } while (rc == SQLITE_ROW);
}

void mergeTileBatch(TileBatch *tileBatch, TileBatch *baseTileBatch)
{
    for (int i = 0; i < tileBatch->size; i++)
    {
        Tile *tile = getNextTile(tileBatch);
        // pthread_mutex_lock(&insertTileLock);
        Tile *baseTile = getNextTile(baseTileBatch);
        // pthread_mutex_unlock(&insertTileLock);

        // Merge tiles if tile exists in base gpkg
        // printf("Hello\n");
        if (baseTile != NULL)
        {
            char *blob = merge(baseTile->blob, tile->blob);

            // If returned blob is not same blob (merge was required)
            if (tile->blob != blob)
            {
                free(tile->blob);
                tile->blob = blob;
                tile->blobSize = strlen(blob);
            }
            // freeTile(baseTile);
        }

        // insertTile(db, tileCache, tile);
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
    // Gpkg *newGpkg = (Gpkg *)args[1];
    TileBatch *tileBatch = (TileBatch *)args[1];
    char *baseGpkgPath = (char *)args[2];
    char *tileCache = (char *)args[3];

    int count = 0;
    int size = 0;

    // Get tile batch
    // pthread_mutex_lock(&getTileLock);
    // tileBatch = getTileBatch(newGpkg->db, newGpkg->tileCache, batchSize, newGpkg->current);
    // size = tileBatch->size;
    // count += size;
    // newGpkg->current = count;
    // pthread_mutex_unlock(&getTileLock);

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
    char *countAllString = executeStatementSingleColResult(newDb, tileCountQuery);

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

    for (int i = 0; i < amount; i++)
    {
        TileBatch *tileBatch = getTileBatch(newDb, newGpkg->tileCache, batchSize, newGpkg->current);
        size = tileBatch->size;
        count += size;
        newGpkg->current = count;

        TileBatch *baseTileBatch = getCorrespondingBatch(tileBatch, baseDb, baseGpkg->tileCache);

        mergeTileBatch(tileBatch, baseTileBatch);
        insertTileBatch(tileBatch, baseDb, baseGpkg->tileCache);

        printf("Merged %d/%d tiles\n", count, countAll);
    }
    sqlite3_close(baseDb);
    sqlite3_close(newDb);

    // Add tile index
    baseDb = openGpkg(baseGpkg->path, SQLITE_OPEN_READWRITE);
    addIndex(baseDb, baseGpkg->tileCache);
    sqlite3_close(baseDb);
    free(countAllString);
}

void mergeGpkgs(Gpkg *baseGpkg, Gpkg *newGpkg, int batchSize)
{
    sqlite3 *baseDb = openGpkg(baseGpkg->path, SQLITE_OPEN_READWRITE);
    sqlite3 *newDb = openGpkg(newGpkg->path, SQLITE_OPEN_READONLY);
    copyTileMatrix(baseDb, newDb, baseGpkg->tileCache);
    updateExtent(baseDb, newDb);
    sqlite3_close(baseDb);

    char *tileCountQuery = getTileCountQuery(newGpkg->tileCache);
    char *countAllString = executeStatementSingleColResult(newDb, tileCountQuery);

    countAll = atoi(countAllString);
    free(tileCountQuery);
    printf("Working on merging %d tile into base gpkg\n", countAll);
    int count = 0;
    int size = 0;

    pthread_mutex_init(&insertTileLock, NULL);
    tpool_t *threadPool = tpool_create(5);

    int amount = countAll / batchSize;
    if (countAll % batchSize != 0)
    {
        amount++;
    }

    for (int i = 0; i < amount; i++)
    {
        TileBatch *tileBatch = getTileBatch(newDb, newGpkg->tileCache, batchSize, newGpkg->current);
        size = tileBatch->size;
        count += size;
        newGpkg->current = count;

        pthread_mutex_lock(&insertTileLock);
        baseDb = openGpkg(baseGpkg->path, SQLITE_OPEN_READONLY);
        TileBatch *baseTileBatch = getCorrespondingBatch(tileBatch, baseDb, baseGpkg->tileCache);

        sqlite3_close(baseDb);
        pthread_mutex_unlock(&insertTileLock);

        void *args[] = {baseTileBatch, tileBatch, baseGpkg->path, baseGpkg->tileCache};
        tpool_add_work(threadPool, work, args);
    }

    sqlite3_close(newDb);
    tpool_wait(threadPool);
    tpool_destroy(threadPool);
    pthread_mutex_destroy(&insertTileLock);

    // Add tile index
    baseDb = openGpkg(baseGpkg->path, SQLITE_OPEN_READWRITE);
    addIndex(baseDb, baseGpkg->tileCache);
    sqlite3_close(baseDb);
    free(countAllString);
}

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
    printf("Current position: %d\n", gpkg->current);
}
