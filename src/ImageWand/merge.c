// #include "merge.h"
// #define ThrowWandException(wand)                                                          \
//     {                                                                                     \
//         char                                                                              \
//             *description;                                                                 \
//                                                                                           \
//         ExceptionType                                                                     \
//             severity;                                                                     \
//                                                                                           \
//         description = MagickGetException(wand, &severity);                                \
//         (void)fprintf(stderr, "%s %s %lu, reason: %s\n", GetMagickModule(), description); \
//         description = (char *)MagickRelinquishMemory(description);                        \
//         exit(-1);                                                                         \
//     }
// MagickBooleanType wandHasAlpha(MagickWand *wand)
// {
//     return MagickGetImageAlphaChannel(wand) == MagickTrue;
// }
// unsigned char *hexToByteArray(unsigned char *hex, size_t length)
// {
//     char *position = hex;
//     size_t byteArrayLength = length / 2;
//     unsigned char *byteArray = (unsigned char *)malloc((byteArrayLength + 1) * sizeof(unsigned char));
//     for (size_t count = 0; count < byteArrayLength; count++)
//     {
//         sscanf(position, "%2hhx", &byteArray[count]);
//         position += 2;
//     }
//     byteArray[byteArrayLength] = '0';
//     return byteArray;
// }
// void createWandFromHex(MagickWand *wand, unsigned char *hex)
// {
//     unsigned char *byteArray;
//     byteArray = hexToByteArray(hex, strlen(hex));
//     MagickBooleanType status = MagickReadImageBlob(wand, byteArray, strlen(hex) / 2 + 1);
//     free(byteArray);
//     if (status == MagickFalse)
//         ThrowWandException(wand);
// }
// unsigned char *hexFromWand(MagickWand *wand)
// {
//     size_t length;
//     unsigned char *blob = MagickGetImageBlob(wand, &length);
//     unsigned char *pos = (unsigned char *)malloc((length * 2 + 1) * sizeof(unsigned char));
//     int i;
//     for (i = 0; i < length; i++)
//     {
//         sprintf(pos + (i * 2), "%02x", blob[i]);
//     }
//     free(blob);
//     return pos;
// }
// unsigned char *merge(char *firstImageHex, char *secondImageHex)
// {
//     MagickWand *firstWand, *secondWand;
//     MagickBooleanType status;
//     unsigned char *hexReturn;
//     firstWand = NewMagickWand();
//     secondWand = NewMagickWand();
//     createWandFromHex(firstWand, firstImageHex);
//     createWandFromHex(secondWand, secondImageHex);
//     if (wandHasAlpha(secondWand))
//     {
//         // Image has transparency - we need to composite tiles.
//         status = MagickCompositeImage(firstWand, secondWand, OverCompositeOp, MagickFalse, 0, 0);
//         if (status == MagickFalse)
//             ThrowWandException(firstWand);
//         hexReturn = hexFromWand(firstWand);
//     }
//     else
//     {
//         // Image has no transparency - it is full. Return it.
//         hexReturn = hexFromWand(secondWand);
//     }
//     firstWand = DestroyMagickWand(firstWand);
//     secondWand = DestroyMagickWand(secondWand);
//     return hexReturn;
// }

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

MagickBooleanType wandHasAlpha(MagickWand *wand)
{
    return MagickGetImageAlphaChannel(wand) == MagickTrue;
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
        ThrowWandException(wand);
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

unsigned char *merge(char *firstImageHex, char *secondImageHex)
{
    MagickWand *firstWand, *secondWand;
    MagickBooleanType status;
    unsigned char *hexReturn;
    secondWand = NewMagickWand();
    createWandFromHex(secondWand, secondImageHex);
    if (wandHasAlpha(secondWand))
    {
        // Image has transparency - we need to composite tiles.
        firstWand = NewMagickWand();
        createWandFromHex(firstWand, firstImageHex);
        status = MagickCompositeImage(firstWand, secondWand, OverCompositeOp, MagickFalse, 0, 0);
        if (status == MagickFalse)
            ThrowWandException(firstWand);
        hexReturn = hexFromWand(firstWand);
        firstWand = DestroyMagickWand(firstWand);
    }
    else
    {
        // Image has no transparency - it is full. Return it.
        hexReturn = hexFromWand(secondWand);
    }
    secondWand = DestroyMagickWand(secondWand);
    return hexReturn;
}