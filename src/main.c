#include <stdio.h>
#include <stdlib.h>
#include "gpkg.h"
#include "threadPool/threadPool.h"
#include "tileBatch.h"

int count;

void work(void *args)
{
    count++;
    printf("%d\n", count);
}

int main(int argc, char **argv)
{
    // TODO: get alot of gpkgs and merge into one

    // Require input of 2 paths (base and new gpkg) and wanted batch size
    if (argc != 4)
    {
        printf("Usage: %s <path_to_base_gpkg> <path_to_new_gpkg> <batch_size>\n", argv[0]);
        exit(-1);
    }

    // TODO: check if path exists and is file

    // Get full path to gpkg files
    char *baseGpkgPath = realpath(argv[1], NULL);
    char *newGpkgPath = realpath(argv[2], NULL);
    int batchSize = atoi(argv[3]);

    // Read gpkgs
    Gpkg *baseGpkg = readGpkgInfo(baseGpkgPath);
    Gpkg *newGpkg = readGpkgInfo(newGpkgPath);

    // Merge gpkgs
    mergeGpkgs(baseGpkg, newGpkg, 1000);

    // Close gpkgs
    closeGpkg(baseGpkg);
    closeGpkg(newGpkg);

    return 0;
}
