#ifndef STATEMENT_H_ /* Include guard */
#define STATEMENT_H_

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

char *executeStatement(sqlite3 *db, char *query);

#endif // STATEMENT_H_
