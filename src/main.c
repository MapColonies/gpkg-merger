#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "gpkg.h"
#include "threadPool/threadPool.h"
#include "tileBatch.h"

void doesPathExist(char *fullPath, char *file)
{
    if (fullPath == NULL)
    {
        printf("No such file: %s\n", file);
        exit(-1);
    }
}

int main(int argc, char **argv)
{
    time_t start, end;
    double diff;
    time(&start);

    // TODO: get alot of gpkgs and merge into one

    // Require input of 2 paths (base and new gpkg) and wanted batch size
    if (argc != 4)
    {
        printf("Usage: %s <path_to_base_gpkg> <path_to_new_gpkg> <batch_size>\n", argv[0]);
        printf("Example: %s area1.gpkg area2.gpkg 1000\n", argv[0]);
        exit(-1);
    }

    // Get full path to gpkg files
    char *baseGpkgPath = realpath(argv[1], NULL);
    char *newGpkgPath = realpath(argv[2], NULL);
    int batchSize = atoi(argv[3]);

    doesPathExist(baseGpkgPath, argv[1]);
    doesPathExist(newGpkgPath, argv[2]);

    // Read gpkgs
    Gpkg *baseGpkg = readGpkgInfo(baseGpkgPath);
    Gpkg *newGpkg = readGpkgInfo(newGpkgPath);

    if (baseGpkg == NULL && newGpkg != NULL)
    {
        closeGpkg(newGpkg);
        exit(-1);
    }

    if (newGpkg == NULL && baseGpkg != NULL)
    {
        closeGpkg(baseGpkg);
        exit(-1);
    }

    int numThreads = 5;
    char *numThreadsString = getenv("NUM_THREADS");
    if (numThreadsString != NULL)
    {
        int tryParse = atoi(numThreadsString);
        if (tryParse > 0)
        {
            numThreads = tryParse;
        }
    }

    MagickWandGenesis();
    // Merge gpkgs
    mergeGpkgs(baseGpkg, newGpkg, batchSize, numThreads);
    MagickWandTerminus();

    // Close gpkgs
    closeGpkg(baseGpkg);
    closeGpkg(newGpkg);

    // Do some calculation.
    time(&end);
    diff = difftime(end, start);
    printf("time: %.2lf\n", diff);
    return 0;
}
