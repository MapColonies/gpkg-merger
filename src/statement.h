#ifndef STATEMENT_H_ /* Include guard */
#define STATEMENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "extent.h"

#define INSERT_TILE_MATRIX_QUERY "REPLACE INTO gpkg_tile_matrix (table_name, zoom_level, matrix_width, matrix_height, tile_width, tile_height, pixel_x_size, pixel_y_size) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
#define MIN_ZOOM_QUERY "SELECT MIN(zoom_level) FROM gpkg_tile_matrix"
#define MAX_ZOOM_QUERY "SELECT MAX(zoom_level) FROM gpkg_tile_matrix"
#define CACHE_NAME_QUERY "SELECT table_name FROM gpkg_contents"
#define TILE_MATRIX_QUERY "SELECT * FROM gpkg_tile_matrix"
#define BASE_ZOOM_QUERY "SELECT zoom_level FROM gpkg_tile_matrix where zoom_level=0 and matrix_width=1 and matrix_height=1"
#define EXTENT_QUERY "SELECT min_x, min_y, max_x, max_y FROM gpkg_contents"
#define QUERY_SIZE 500

sqlite3_stmt *prepareStatement(sqlite3 *db, char *query, int flags);

/**
 * @brief Execute sql query with single row and column result
 * 
 * @param db The database to run the query on
 * @param query Sql query to run
 * @return char* The query result
 */
char *executeQuerySingleColResult(sqlite3 *db, char *query);

/**
 * @brief Execute sql statement with single row and column result
 * 
 * @param db The database to run the query on
 * @param stmt Sql statement to run
 * @param finalize Should finalize statement
 * @return char* The query result
 */
char *executeStatementSingleColResult(sqlite3 *db, sqlite3_stmt *stmt, int finalize);

void finalizeStatement(sqlite3_stmt *stmt);

void vacuum(sqlite3 *db);

int getBlobSize(sqlite3 *db, sqlite3_stmt *stmt, char *tileCache, int z, int x, int y);

int getDBCacheSize(sqlite3 *db);

void setDBCacheSize(sqlite3 *db, int size);

char *getAddIndexQuery(char *tileCache);

char *getTileCountQuery(char *tileCache);

sqlite3_stmt *getExtentInsertStmt(sqlite3 *db, Extent *extent);

sqlite3_stmt *getBatchSelectStmt(sqlite3 *db, char *tileCache);

sqlite3_stmt *getBlobSizeSelectStmt(sqlite3 *db, char *tileCache);

sqlite3_stmt *getTileSelectStmt(sqlite3 *db, char *tileCache);

sqlite3_stmt *getTileInsertStmt(sqlite3 *db, char *tileCache);

void bindTileSelect(sqlite3_stmt *stmt, int x, int y, int z);

void bindTileInsert(sqlite3_stmt *stmt, int x, int y, int z, char *blob);

void bindBatchSelect(sqlite3_stmt *stmt, int limit, int offset);

#endif // STATEMENT_H_
