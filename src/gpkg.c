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

    // We allocate 2 time the size of the blob because it has HEX representation, so it takes two times the size when represented as string
    int allocationSize = tile->blobSize * 2 + 500;
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
    // printf("Hello7\n");
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

void readTileCache(Gpkg *gpkg)
{
    char *tileCache = executeStatementSingleColResult(gpkg->db, CACHE_NAME_QUERY);
    // TODO: check if failed
    gpkg->tileCache = tileCache;
}

void readMinZoomLevel(Gpkg *gpkg)
{
    char *minZoom = executeStatementSingleColResult(gpkg->db, MIN_ZOOM_QUERY);
    // TODO: check if failed
    gpkg->minZoom = atoi(minZoom);
    free(minZoom);
}

void readMaxZoomLevel(Gpkg *gpkg)
{
    char *maxZoom = executeStatementSingleColResult(gpkg->db, MAX_ZOOM_QUERY);
    // TODO: check if failed
    gpkg->maxZoom = atoi(maxZoom);
    free(maxZoom);
}

void updateExtent(Gpkg *baseGpkg, Gpkg *newGpkg)
{
    Extent *baseExtent = getExtent(baseGpkg->db);
    Extent *newExtent = getExtent(newGpkg->db);

    baseExtent->minX = MIN(baseExtent->minX, newExtent->minX);
    baseExtent->maxX = MAX(baseExtent->maxX, newExtent->maxX);
    baseExtent->minY = MIN(baseExtent->minY, newExtent->minY);
    baseExtent->maxY = MAX(baseExtent->maxY, newExtent->maxY);

    char *query = getExtentInsertQuery(baseExtent);
    char *res = executeStatementSingleColResult(baseGpkg->db, query);
    free(res);
    free(query);
    free(baseExtent);
    free(newExtent);
}

void convertIfBase1ToBase2(Gpkg *gpkg)
{
    // TODO: convert
    char *query = executeStatementSingleColResult(gpkg->db, BASE_ZOOM_QUERY);
    free(query);
}

int openGpkg(Gpkg *gpkg)
{
    int rc = sqlite3_open(gpkg->path, &gpkg->db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s, error: %s\n", gpkg->path, sqlite3_errmsg(gpkg->db));
        return -1;
    }
    return 0;
}

Gpkg *readGpkgInfo(char *path)
{
    Gpkg *gpkg = (Gpkg *)malloc(sizeof(Gpkg));
    gpkg->path = path;

    int ok = openGpkg(gpkg);
    if (ok == -1)
    {
        return NULL;
    }

    readTileCache(gpkg);
    readMinZoomLevel(gpkg);
    readMaxZoomLevel(gpkg);
    gpkg->current = 0;

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
        char *res = executeStatementSingleColResult(baseGpkg->db, query);
        free(res);
        free(query);

        free(tileMatrix);
        rc = sqlite3_step(stmt);
    } while (rc == SQLITE_ROW);
}

void mergeTileBatch(TileBatch *tileBatch, sqlite3 *db, char *tileCache)
{
    for (int i = 0; i < tileBatch->size; i++)
    {
        Tile *tile = getNextTile(tileBatch);
        // pthread_mutex_lock(&insertTileLock);
        Tile *baseTile = getTile(db, tileCache, tile->z, tile->x, tile->y);
        // pthread_mutex_unlock(&insertTileLock);

        // Merge tiles if tile exists in base gpkg
        if (baseTile != NULL)
        {
            char *blob = merge(baseTile->blob, tile->blob);
            free(tile->blob);
            tile->blob = blob;
            tile->blobSize = strlen(blob);
            freeTile(baseTile);
        }

        // insertTile(db, tileCache, tile);
    }
    tileBatch->current = 0;
}

void insertTileBatch(TileBatch *tileBatch, sqlite3 *db, char *tileCache)
{
    for (int i = 0; i < tileBatch->size; i++)
    {
        // printf("Hello6 i=%d\n", i);
        Tile *tile = getNextTile(tileBatch);
        // printf("Hello3 i=%d\n", i);

        insertTile(db, tileCache, tile);
    }
}

void printFinishedBatch()
{
    printf("Merged %d/%d tiles\n", countTiles, countAll);
}

void work(void **args)
{
    Gpkg *baseGpkg = (Gpkg *)args[0];
    // Gpkg *newGpkg = (Gpkg *)args[1];
    TileBatch *tileBatch = (TileBatch *)args[1];

    sqlite3 *baseDb = baseGpkg->db;
    // sqlite3 *newDb = newGpkg->db;
    int count = 0;
    int size = 0;

    // Get tile batch
    // pthread_mutex_lock(&getTileLock);
    // tileBatch = getTileBatch(newGpkg->db, newGpkg->tileCache, batchSize, newGpkg->current);
    // size = tileBatch->size;
    // count += size;
    // newGpkg->current = count;
    // pthread_mutex_unlock(&getTileLock);

    mergeTileBatch(tileBatch, baseDb, baseGpkg->tileCache);

    // Insert batch
    // pthread_mutex_lock(&insertTileLock);
    // printf("Hello2\n");
    insertTileBatch(tileBatch, baseDb, baseGpkg->tileCache);
    countTiles = countTiles + tileBatch->size;
    // printFinishedBatch();
    // pthread_mutex_unlock(&insertTileLock);

    freeBatch(tileBatch);
    // printf("Hello4\n");
}

void mergeGpkgs(Gpkg *baseGpkg, Gpkg *newGpkg, int batchSize)
{
    // TileBatch *tileBatch;
    sqlite3 *baseDb = baseGpkg->db;
    sqlite3 *newDb = newGpkg->db;
    copyTileMatrix(baseGpkg, newGpkg);
    updateExtent(baseGpkg, newGpkg);

    char *tileCountQuery = getTileCountQuery(newGpkg->tileCache);
    char *countAllString = executeStatementSingleColResult(newDb, tileCountQuery);
    countAll = atoi(countAllString);
    free(tileCountQuery);
    printf("Working on merging %d tile into base gpkg\n", countAll);
    int count = 0;
    int size = 0;

    // pthread_mutex_init(&getTileLock, NULL);
    // pthread_mutex_init(&insertTileLock, NULL);
    // tpool_t *threadPool = tpool_create(5);

    int amount = countAll / batchSize;
    if (countAll % batchSize != 0)
    {
        amount++;
    }

    for (int i = 0; i < amount; i++)
    {
        TileBatch *tileBatch = getTileBatch(newGpkg->db, newGpkg->tileCache, batchSize, newGpkg->current);
        size = tileBatch->size;
        count += size;
        newGpkg->current = count;
        // do
        // {
        void *args[] = {baseGpkg, tileBatch};
        // tpool_add_work(threadPool, work, args);
        // printf("Current %d\n", newGpkg->current);
        // tileBatch = getTileBatch(newGpkg->db, newGpkg->tileCache, batchSize, newGpkg->current);
        // size = tileBatch->size;
        mergeTileBatch(tileBatch, baseDb, baseGpkg->tileCache);
        insertTileBatch(tileBatch, baseDb, baseGpkg->tileCache);

        // count += size;
        // newGpkg->current = count;
        // freeBatch(tileBatch);
        printf("Merged %d/%d tiles\n", count, countAll);
        // } while (size != 0);
    }

    // printf("Hello\n");
    // tpool_wait(threadPool);
    // printf("Hello\n");
    // tpool_destroy(threadPool);
    // pthread_mutex_destroy(&getTileLock);
    // pthread_mutex_destroy(&insertTileLock);

    // Add tile index
    addIndex(baseDb, baseGpkg->tileCache);
    free(countAllString);
}

void closeGpkg(Gpkg *gpkg)
{
    sqlite3_close(gpkg->db);
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
