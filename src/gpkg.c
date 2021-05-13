#include "gpkg.h"

#define MIN_ZOOM_QUERY "SELECT MIN(zoom_level) FROM gpkg_tile_matrix"
#define MAX_ZOOM_QUERY "SELECT MAX(zoom_level) FROM gpkg_tile_matrix"

char *executeStatement(Gpkg *gpkg, char *query)
{
    unsigned char *res = NULL;
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(gpkg->db, query, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to execute query: %s, error message: %s\n", query, sqlite3_errmsg(gpkg->db));
        sqlite3_close(gpkg->db);
        return NULL;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        res = strdup(sqlite3_column_text(stmt, 0));
        printf("%s\n", sqlite3_column_text(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return res;
}

void readMinZoomLevel(Gpkg *gpkg)
{
    const char *minZoom = executeStatement(gpkg, MIN_ZOOM_QUERY);
    // TODO: check if failed
    gpkg->minZoom = atoi(minZoom);
}

void readMaxZoomLevel(Gpkg *gpkg)
{
    const char *maxZoom = executeStatement(gpkg, MAX_ZOOM_QUERY);
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

struct Gpkg *readGpkgInfo(char *path)
{
    struct Gpkg *gpkg = (struct Gpkg *)malloc(sizeof(struct Gpkg));
    gpkg->path = path;

    openGpkg(gpkg);
    readMinZoomLevel(gpkg);
    readMaxZoomLevel(gpkg);

    return gpkg;
}

void closeGpkg(Gpkg *gpkg)
{
    sqlite3_close(gpkg->db);
}

void printGpkgInfo(Gpkg *gpkg)
{
    printf("Path: %s\n", gpkg->path);
    printf("Min zoom: %d\n", gpkg->minZoom);
    printf("Max zoom: %d\n", gpkg->maxZoom);
    printf("Current position: %d\n", gpkg->current);
}