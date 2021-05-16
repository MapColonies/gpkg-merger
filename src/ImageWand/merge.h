#ifndef MERGE_H /* Include guard */
#define MERGE_H

#include <string.h>
#include "MagickWand/MagickWand.h"

void ThrowWandException(MagickWand *);
unsigned char *merge(char *hexValue, char *hexValue2);

#endif // MERGE_H
