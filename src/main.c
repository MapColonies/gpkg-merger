#include <stdio.h>
#include <stdlib.h>
#include "gpkg.h"

// void getBlobs()
// {
// }

int main(int argc, char **argv)
{
    // Require input of 2 paths (base and new gpkg)
    if (argc != 3)
    {
        exit(-1);
    }

    // Get full path to gpkg files
    char *baseGpkgPath = realpath(argv[1], NULL);
    char *newGpkgPath = realpath(argv[2], NULL);

    // Read gpkgs
    Gpkg *baseGpkg = readGpkgInfo(baseGpkgPath);
    Gpkg *newGpkg = readGpkgInfo(newGpkgPath);

    printf("Hello World!\n");
    printGpkgInfo(baseGpkg);
    printGpkgInfo(newGpkg);
    printf("%s %s\n", baseGpkg->path, newGpkg->path);

    // Close gpkgs
    closeGpkg(baseGpkg);
    closeGpkg(newGpkg);

    return 0;
}
