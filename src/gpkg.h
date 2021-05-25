#ifndef GPKG_H_ /* Include guard */
#define GPKG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <pthread.h>
#include "tileBatch.h"
#include "statement.h"
#include "ImageWand/merge.h"
#include "threadPool/threadPool.h"

typedef struct Gpkg
{
    char *path;
    char *tileCache;
    int minZoom;
    int maxZoom;
    int current;
} Gpkg;

Gpkg *readGpkgInfo(char *path);
void closeGpkg(Gpkg *gpkg);
void printGpkgInfo(Gpkg *gpkg);
void mergeGpkgs(Gpkg *baseGpkg, Gpkg *newGpkg, int batchSize);

#endif // GPKG_H_
