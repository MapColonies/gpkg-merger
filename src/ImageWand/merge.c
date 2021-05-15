#include <string.h>
#include "MagickWand/MagickWand.h"

int main(int argc, const char *argv[])
{
    MagickWand
        *wand1,
        *wand2,
        *wand3,
        *result;

    MagickWandGenesis();
    wand1 = NewMagickWand();
    wand2 = NewMagickWand();

    MagickReadImage(wand1, "/home/roees/Desktop/img1.png");
    MagickReadImage(wand2, "/home/roees/Desktop/img2.jpg");

    // convert one.png two.png +clone -combine displaceMask.png
    wand3 = CloneMagickWand(wand2);
    MagickAddImage(wand1, wand2);
    MagickAddImage(wand1, wand3);
    MagickSetFirstIterator(wand1);
    result = MagickCombineImages(wand1, MagickGetImageColorspace(wand1));
    wand3 = CloneMagickWand(wand1);
    MagickWriteImage(result,"merge.png");

    wand1 = DestroyMagickWand(wand1);
    wand2 = DestroyMagickWand(wand2);
    wand3 = DestroyMagickWand(wand3);
    result = DestroyMagickWand(result);
    MagickWandTerminus();

    return 0;
}