#include <stdio.h>
#include <stdlib.h>
#include "gpkg.h"
#include "tileBatch.h"

int main(int argc, char **argv)
{
    // Require input of 2 paths (base and new gpkg)
    if (argc != 4)
    {
        printf("Usage: a.out <path_to_base_gpkg> <path_to_new_gpkg> <batch_size>\n");
        exit(-1);
    }

    // Get full path to gpkg files
    char *baseGpkgPath = realpath(argv[1], NULL);
    char *newGpkgPath = realpath(argv[2], NULL);
    int batchSize = atoi(argv[3]);

    // Read gpkgs
    Gpkg *baseGpkg = readGpkgInfo(baseGpkgPath);
    Gpkg *newGpkg = readGpkgInfo(newGpkgPath);

    // TileBatch *tileBatch = getTileBatch(newGpkg->db, newGpkg->tileCache, batchSize, newGpkg->current);
    // printBatch(tileBatch);

    // Close gpkgs
    closeGpkg(baseGpkg);
    closeGpkg(newGpkg);

    return 0;
}
