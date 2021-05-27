#ifndef MERGE_H /* Include guard */
#define MERGE_H

#include "MagickWand/MagickWand.h"
#include "wandUtil.h"
#include "upscaling.h"
#include "../tile.h"

char *mergeNewToBase(Tile *, Tile *);
char *mergeWands(MagickWand *, MagickWand *);

#endif // MERGE_H
