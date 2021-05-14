#include "gpkg.h"

#define MIN_ZOOM_QUERY "SELECT MIN(zoom_level) FROM gpkg_tile_matrix"
#define MAX_ZOOM_QUERY "SELECT MAX(zoom_level) FROM gpkg_tile_matrix"
#define CACHE_NAME_QUERY "SELECT table_name FROM gpkg_contents"

void readTileCacheLevel(Gpkg *gpkg)
{
    const char *tileCache = executeStatement(gpkg->db, CACHE_NAME_QUERY);
    // TODO: check if failed
    gpkg->tileCache = strdup(tileCache);
}

void readMinZoomLevel(Gpkg *gpkg)
{
    const char *minZoom = executeStatement(gpkg->db, MIN_ZOOM_QUERY);
    // TODO: check if failed
    gpkg->minZoom = atoi(minZoom);
}

void readMaxZoomLevel(Gpkg *gpkg)
{
    const char *maxZoom = executeStatement(gpkg->db, MAX_ZOOM_QUERY);
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
