#include "merge.h"


#define ThrowWandException(wand)                                                          \
    {                                                                                     \
        char                                                                              \
            *description;                                                                 \
                                                                                          \
        ExceptionType                                                                     \
            severity;                                                                     \
                                                                                          \
        description = MagickGetException(wand, &severity);                                \
        (void)fprintf(stderr, "%s %s %lu, reason: %s\n", GetMagickModule(), description); \
        description = (char *)MagickRelinquishMemory(description);                        \
        exit(-1);                                                                         \
    }

unsigned char *hexToByteArray(unsigned char *hex, size_t length)
{
    char *position = hex;
    unsigned char *byteArray = (unsigned char *)malloc(((length / 2) + 1) * sizeof(unsigned char));

    for (size_t count = 0; count < length / 2; count++)
    {
        sscanf(position, "%2hhx", &byteArray[count]);
        position += 2;
    }

    return byteArray;
}

void merge(char *hexValue, char *hexValue2, char *filename) {
    MagickWand *wand1, *wand2;
    MagickBooleanType status;

    MagickWandGenesis();

    wand1 = NewMagickWand();
    wand2 = NewMagickWand();

    status = MagickReadImageBlob(wand1, hexToByteArray(hexValue, strlen(hexValue)), strlen(hexValue));

    if (status == MagickFalse)
        ThrowWandException(wand1);

    status = MagickReadImageBlob(wand2, hexToByteArray(hexValue2, strlen(hexValue2)), strlen(hexValue2));

    if (status == MagickFalse)
        ThrowWandException(wand2);

    status = MagickAddImage(wand1, wand2);

    if (status == MagickFalse)
        ThrowWandException(wand1);

    MagickSetFirstIterator(wand1);
    status = MagickCompositeImage(wand1, wand2, OverlayCompositeOp, MagickFalse, 0, 0);

    if (status == MagickFalse)
        ThrowWandException(wand1);

    status = MagickWriteImage(wand1, filename);

    if (status == MagickFalse)
        ThrowWandException(wand1);

    wand1 = DestroyMagickWand(wand1);
    wand2 = DestroyMagickWand(wand2);
    MagickWandTerminus();
}
