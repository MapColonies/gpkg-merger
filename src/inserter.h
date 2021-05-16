
#include <pthread.h>
#include <semaphore.h>
#include "tileBatch.h"

typedef struct Inserter
{
    TileBatch **batches;
    sem_t wrt;
    pthread_mutex_t insertLock;
    // pthread_mutex_t readLock;
    int size;
    int currentRead;
    int currentInsert;
    int work;
} Inserter;

void start(Inserter *inserter, int amountOfBatches);
void addBatch(Inserter *inserter, TileBatch *tileBatch);