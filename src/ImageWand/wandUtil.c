#include "wandUtil.h"

void handleError(unsigned char *message, MagickWand *wand)
{
    char *description;
    ExceptionType severity;

    description = MagickGetException(wand, &severity);
    (void)fprintf(stderr, "ERROR: failed with reason: %s\n", message);
    (void)fprintf(stderr, "%s %s %lu, reason: %s\n", GetMagickModule(), description);
    description = (char *)MagickRelinquishMemory(description);
    exit(-1);
}

unsigned char *hexToByteArray(unsigned char *hex)
{
    size_t length = strlen(hex);
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

void createWandFromHex(MagickWand *wand, unsigned char *hex)
{
    unsigned char *byteArray;
    byteArray = hexToByteArray(hex);
    MagickBooleanType status = MagickReadImageBlob(wand, byteArray, strlen(hex) / 2 + 1);
    free(byteArray);
    if (status == MagickFalse)
        handleError("Could not create wand from hex", wand);
}

unsigned char *hexFromWand(MagickWand *wand)
{
    size_t length;
    unsigned char *blob = MagickGetImageBlob(wand, &length);
    unsigned char *pos = (unsigned char *)malloc((length * 2 + 1) * sizeof(unsigned char));
    int i;
    for (i = 0; i < length; i++)
    {
        sprintf(pos + (i * 2), "%02x", blob[i]);
    }
    free(blob);
    return pos;
}

MagickBooleanType wandHasAlpha(MagickWand *wand)
{
    return MagickGetImageAlphaChannel(wand) == MagickTrue;
}
