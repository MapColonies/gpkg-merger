#include "../gpkg.h"
#include "upscaling.h"

int main()
{
    time_t start, end;
    double diff;
    time(&start);

    MagickWandGenesis();
    char *baseGpkgPath = realpath("/home/roees/Documents/gpkgs/artzi_fixed.gpkg", NULL);
    Gpkg *baseGpkg = readGpkgInfo(baseGpkgPath);
    char *newGpkgPath = realpath("/home/roees/Documents/gpkgs/area1.gpkg", NULL);
    Gpkg *newGpkg = readGpkgInfo(newGpkgPath);
    mergeGpkgs(baseGpkg, newGpkg, 1000);
    // mergeGpkgsNoThreads(baseGpkg, newGpkg, 1000);

    closeGpkg(baseGpkg);
    closeGpkg(newGpkg);
    MagickWandTerminus();

    time(&end);
    diff = difftime(end, start);
    printf("time: %.2lf\n", diff);
    return 0;
}
