#include <sqlite3.h>
#include "MagickWand/MagickWand.h"
#include "wandUtil.h"
#include "../tile.h"

#define TILE_WIDTH 256
#define TILE_HEIGHT 256
#define ZOOM_LEVEL_COUNT 25
#define X_Y_COORDS_FOR_ALL_ZOOM_LEVELS ZOOM_LEVEL_COUNT << 1 // Multiply by 2
#define SQL_QUERY_SIZE 2000
#define SQL_SUBQUERY_SIZE 80

void lastTileSQLQuery(char *, char *);
Tile *bindParametersToQueryAndExecute(sqlite3 *, char *, int[]);
Tile *getLastExistingTile(int, int, int, sqlite3 *, char *);
void upscale(MagickWand *baseWand, Tile *baseTile, int newZoomLevel, int newX, int newY);
