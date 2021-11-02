#include "merge.h"

char *mergeNewToBase(Tile *new, Tile *lastExistingTile)
{
    MagickWand *newWand = NewMagickWand(), *baseUpscaled;
    char *hexReturn;
    createWandFromHex(newWand, new->blob);
    if (wandHasAlpha(newWand))
    {
        baseUpscaled = NewMagickWand();
        createWandFromHex(baseUpscaled, lastExistingTile->blob);
        MagickSetImageFormat(baseUpscaled, "PNG");
        
        if (lastExistingTile->z != new->z)
        {
            upscale(baseUpscaled, lastExistingTile, new->z, new->x, new->y);
        }
        hexReturn = mergeWands(baseUpscaled, newWand);
        baseUpscaled = DestroyMagickWand(baseUpscaled);
    }
    else
    {
        hexReturn = new->blob;
    }

    newWand = DestroyMagickWand(newWand);
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

