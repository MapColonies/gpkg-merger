#ifndef MERGE_H /* Include guard */
#define MERGE_H

#include <string.h>
#include <sqlite3.h>
#include "../tile.h"
#include "MagickWand/MagickWand.h"

void ThrowWandException(MagickWand *);
char *mergeNewToBase(Tile *, sqlite3 *, char *);
char *mergeWands(MagickWand *, MagickWand *);

#endif // MERGE_H
