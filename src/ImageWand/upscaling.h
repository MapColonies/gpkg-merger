#include <sqlite3.h>
#include "MagickWand/MagickWand.h"
#include "wandUtil.h"
#include "../tile.h"

void lastTileSQLQuery(char *, char *, int);
Tile *bindParametersToQueryAndExecute(sqlite3 *, char *, int[], int);
Tile *getLastExistingTile(int, int, int, sqlite3 *, char *);
void upscale(MagickWand *baseWand, Tile *baseTile, int newZoomLevel, int newX, int newY);
