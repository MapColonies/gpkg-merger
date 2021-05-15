#ifndef MERGE_H /* Include guard */
#define MERGE_H

#include <string.h>
#include "MagickWand/MagickWand.h"

void ThrowWandException(MagickWand *);
unsigned char *merge(/* char *, char *, char * */);

#endif // MERGE_H
