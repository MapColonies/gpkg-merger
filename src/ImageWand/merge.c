#include "merge.h"
#include "../tile.h"
#include "wandUtil.h"
#include "upscaling.h"

char *mergeNewToBase(Tile *new, sqlite3 *db, char *tableName)
{
    MagickWand *newWand = NewMagickWand(), *baseUpscaled;
    char *hexReturn;
    createWandFromHex(newWand, new->blob);
    MagickBooleanType alpha = wandHasAlpha(newWand);
    if (alpha)
    {
        Tile *lastExistingTile = getLastExistingTile(new->x, new->y, new->z, db, tableName);
        if (lastExistingTile->z != new->z)
        {
            // printf("%s\n", lastExistingTile->blob);
            baseUpscaled = upscale(new->z, new->x, new->y, db, tableName, lastExistingTile);
        }
        hexReturn = mergeWands(baseUpscaled, newWand);
    }
    else
    {
        hexReturn = new->blob;
    }

    // newWand = DestroyMagickWand(newWand);
    // baseUpscaled = DestroyMagickWand(baseUpscaled);
    return hexReturn;
}

char *mergeWands(MagickWand *base, MagickWand *new)
{
    MagickBooleanType status;
    status = MagickCompositeImage(base, new, OverCompositeOp, MagickFalse, 0, 0);
    if (status == MagickFalse)
        handleError("Could not composite two wands", base);

    return hexFromWand(base);
}

// unsigned char *merge(char *firstImageHex, char *secondImageHex)
// {
//     MagickWand *firstWand, *secondWand;
//     MagickBooleanType status;
//     unsigned char *hexReturn;
//     secondWand = NewMagickWand();
//     createWandFromHex(secondWand, secondImageHex);

//     if (wandHasAlpha(secondWand))
//     {
//         firstWand = NewMagickWand();
//         createWandFromHex(firstWand, firstImageHex);

//         status = MagickCompositeImage(firstWand, secondWand, OverCompositeOp, MagickFalse, 0, 0);
//         if (status == MagickFalse)
//             handleError("Could not composite two wands", firstWand);
//         hexReturn = hexFromWand(firstWand);
//         firstWand = DestroyMagickWand(firstWand);
//     }
//     else
//     {
//         hexReturn = secondImageHex;
//     }
//     secondWand = DestroyMagickWand(secondWand);
//     return hexReturn;
// }
