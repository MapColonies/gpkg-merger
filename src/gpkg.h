#ifndef GPKG_H_ /* Include guard */
#define GPKG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <pthread.h>
#include "env.h"
#include "extent.h"
#include "tileBatch.h"
#include "statement.h"
#include "ImageWand/merge.h"
#include "threadPool/threadPool.h"

typedef struct Gpkg
{
    int minZoom;
    int maxZoom;
    long current;
    char *path;
    char *tileCache;
} Gpkg;

Gpkg *readGpkgInfo(char *path);
void closeGpkg(Gpkg *gpkg);
void printGpkgInfo(Gpkg *gpkg);
void mergeGpkgs(Gpkg *baseGpkg, Gpkg *newGpkg, int batchSize);
void mergeGpkgsNoThreads(Gpkg *baseGpkg, Gpkg *newGpkg, int batchSize);

#endif // GPKG_H_
