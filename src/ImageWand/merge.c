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
    size_t byteArrayLength = length / 2;
    unsigned char *byteArray = (unsigned char *)malloc((byteArrayLength + 1) * sizeof(unsigned char));

    for (size_t count = 0; count < byteArrayLength; count++)
    {
        sscanf(position, "%2hhx", &byteArray[count]);
        position += 2;
    }
    byteArray[byteArrayLength] = '0';

    return byteArray;
}

unsigned char *merge(char *hexValue, char *hexValue2)
{
    MagickWand *wand1, *wand2;
    MagickBooleanType status;

    // MagickWandGenesis();

    wand1 = NewMagickWand();
    wand2 = NewMagickWand();

    unsigned char *byteArr = hexToByteArray(hexValue, strlen(hexValue));
    status = MagickReadImageBlob(wand1, byteArr, strlen(hexValue) / 2 + 1);
    free(byteArr);

    if (status == MagickFalse)
        ThrowWandException(wand1);

    byteArr = hexToByteArray(hexValue2, strlen(hexValue2));
    status = MagickReadImageBlob(wand2, byteArr, strlen(hexValue2) / 2 + 1);
    free(byteArr);

    if (status == MagickFalse)
        ThrowWandException(wand2);

    status = MagickAddImage(wand1, wand2);

    if (status == MagickFalse)
        ThrowWandException(wand1);

    MagickSetFirstIterator(wand1);
    status = MagickCompositeImage(wand1, wand2, OverCompositeOp, MagickFalse, 0, 0);

    if (status == MagickFalse)
        ThrowWandException(wand1);

    size_t length;
    unsigned char *blob = MagickGetImageBlob(wand1, &length);
    unsigned char *pos = (unsigned char *)malloc((length * 2 + 1) * sizeof(unsigned char));

    if (status == MagickFalse)
        ThrowWandException(wand1);

    wand1 = DestroyMagickWand(wand1);
    wand2 = DestroyMagickWand(wand2);
    // MagickWandTerminus();

    int i;
    for (i = 0; i < length; i++)
    {
        sprintf(pos + (i * 2), "%02x", blob[i]);
    }
    free(blob);

    return pos;
}
