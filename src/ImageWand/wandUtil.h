#ifndef WANDUTIL_H /* Include guard */
#define WANDUTIL_H

#include <string.h>
#include "MagickWand/MagickWand.h"

void handleError(unsigned char *, MagickWand *);
unsigned char *hexToByteArray(unsigned char *);
void createWandFromHex(MagickWand *, unsigned char *);
unsigned char *hexFromWand(MagickWand *);
MagickBooleanType wandHasAlpha(MagickWand *);

#endif // WANDUTIL_H
