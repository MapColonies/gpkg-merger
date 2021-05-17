#include "gpkg.h"

#define MIN_ZOOM_QUERY "SELECT MIN(zoom_level) FROM gpkg_tile_matrix"
#define MAX_ZOOM_QUERY "SELECT MAX(zoom_level) FROM gpkg_tile_matrix"
#define CACHE_NAME_QUERY "SELECT table_name FROM gpkg_contents"
#define TILE_MATRIX_QUERY "SELECT * FROM gpkg_tile_matrix"
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
    char *sql = (char *)malloc(300 * sizeof(char));
    sprintf(sql, "CREATE UNIQUE INDEX IF NOT EXISTS index_tiles on %s (zoom_level, tile_row, tile_column)", tileCache);
    return sql;
}

char *getTileInsertQuery(char *tileCache, Tile *tile)
{
    char *sql = (char *)malloc(1000000 * sizeof(char));
    sprintf(sql, "REPLACE INTO %s (zoom_level, tile_column, tile_row, tile_data) VALUES (%d, %d, %d, x'%s')", tileCache, tile->z, tile->x, tile->y, tile->blob);
    return sql;
}

char *getTileCountQuery(char *tileCache)
{
    char *sql = (char *)malloc(200 * sizeof(char));
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
    // char *tileInsertQuery = (char *)malloc(100000000 * sizeof(char));
    // sprintf(tileInsertQuery, "REPLACE INTO %s (zoom_level, tile_column, tile_row, tile_data) VALUES (?, ?, ?, ?)", tileCache);
    // // char *sql = "REPLACE INTO %s (zoom_level, tile_column, tile_row, tile_data) VALUES(?,?,?,?)";
    // // return sql;

    char *tileInsertQuery = getTileInsertQuery(tileCache, tile);
    // printf("%s\n", tileInsertQuery);
    sqlite3_stmt *stmt = prepareStatement(db, tileInsertQuery);
    // sqlite3_bind_int(stmt, 1, tile->z);
    // sqlite3_bind_int(stmt, 2, tile->x);
    // sqlite3_bind_int(stmt, 3, tile->y);
    // sqlite3_bind_blob(stmt, 4, tile->blob, strlen(tile->blob), SQLITE_TRANSIENT);
    char *res = executeStatementSingleColResult(db, tileInsertQuery);
    free(res);
    finalizeStatement(stmt);
    free(tileInsertQuery);
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

void openGpkg(Gpkg *gpkg)
{
    int rc = sqlite3_open(gpkg->path, &gpkg->db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s, error: %s\n", gpkg->path, sqlite3_errmsg(gpkg->db));
        closeGpkg(gpkg);
        exit(-1);
    }
}

Gpkg *readGpkgInfo(char *path)
{
    Gpkg *gpkg = (Gpkg *)malloc(sizeof(Gpkg));
    gpkg->path = path;

    openGpkg(gpkg);
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

void insertTileBatch(TileBatch *tileBatch, sqlite3 *db, char *tileCache)
{
    for (int i = 0; i < tileBatch->size; i++)
    {
        Tile *tile = getNextTile(tileBatch);
        Tile *baseTile = getTile(db, tileCache, tile->z, tile->x, tile->y);

        // Merge tiles if tile exists in base gpkg
        if (baseTile->blob != NULL)
        {
            // char *pythonMerge = (char *)malloc(10000000 * sizeof(char));
            // sprintf(pythonMerge, "python3 ./merge.py %s %s", baseTile->blob, tile->blob);
            // FILE *fp = popen(pythonMerge, "r");

            // if (fp == NULL)
            // {
            //     printf("Failed to run merge command");
            //     exit(1);
            // }

            // exit(-1);

            // printf("%s\n", pythonMerge);
            // exit(-1);

            // char combinedBlob[1000000];
            // fgets(combinedBlob, sizeof(combinedBlob), fp);

            // while (fgets(combinedBlob, sizeof(combinedBlob), fp) != NULL)
            //     ;

            // printf("%s\n", combinedBlob);
            char *blob = merge(baseTile->blob, tile->blob);
            free(tile->blob);
            // tile->blob = strdup(pythonMerge);
            // tile->blob = combinedBlob;
            tile->blob = blob;
            // pclose(fp);
        }

        // printf("yessss %s\n", tile->blob);

        insertTile(db, tileCache, tile);

        freeTile(baseTile);

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
    sqlite3 *newDb = newGpkg->db;
    copyTileMatrix(baseGpkg, newGpkg);
    updateExtent(baseGpkg, newGpkg);

    char *tileCountQuery = getTileCountQuery(newGpkg->tileCache);
    char *countAll = executeStatementSingleColResult(newDb, tileCountQuery);
    free(tileCountQuery);
    printf("Working on merging %s tile into base gpkg\n", countAll);
    int count = 0;
    int size = 0;

    do
    {
        printf("Current %d\n", newGpkg->current);
        tileBatch = getTileBatch(newGpkg->db, newGpkg->tileCache, batchSize, newGpkg->current);
        size = tileBatch->size;
        insertTileBatch(tileBatch, baseDb, baseGpkg->tileCache);

        count += size;
        newGpkg->current = count;
        freeBatch(tileBatch);
        printf("Merged %d/%s tiles\n", count, countAll);
    } while (size != 0);

    // Add tile index
    addIndex(baseDb, baseGpkg->tileCache);
    free(countAll);
}

void closeGpkg(Gpkg *gpkg)
{
    sqlite3_close(gpkg->db);
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
