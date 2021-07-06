#include "env.h"

int getVacuumCount()
{
    char *vacuumCount = getenv("VACUUM_COUNT");
    if (vacuumCount)
    {
        return atoi(vacuumCount);
    }
    return 4000000;
}