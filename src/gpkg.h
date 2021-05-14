#ifndef GPKG_H_ /* Include guard */
#define GPKG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "statement.h"

typedef struct Gpkg
{
    char *path;
    sqlite3 *db;
    char *tileCache;
    int minZoom;
    int maxZoom;
    int current;
} Gpkg;

Gpkg *readGpkgInfo(char *path);
void closeGpkg(Gpkg *gpkg);
void printGpkgInfo(Gpkg *gpkg);

#endif // GPKG_H_
