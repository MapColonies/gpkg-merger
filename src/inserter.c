#include "inserter.h"

void start(Inserter *inserter, int amountOfBatches)
{
    inserter->work = 1;
    inserter->size = amountOfBatches;
    pthread_mutex_init(&inserter->insertLock, NULL);
    sem_init(&inserter->wrt, 0, 1);

    while (inserter->work)
    {
        // TileBatch *tileBatch;
        // Critical section
        pthread_mutex_lock(&inserter->insertLock);
        // TileBatch *tileBatch = inserter->batches[inserter->currentInsert];
        int cond = inserter->currentInsert == inserter->currentRead;
        pthread_mutex_unlock(&inserter->insertLock);

        if (cond)
        {
            // pthread_mutex_lock(&inserter->readLock);
            sem_wait(&inserter->wrt);
        }

        pthread_mutex_lock(&inserter->insertLock);
        TileBatch *tileBatch = inserter->batches[inserter->currentInsert];
        inserter->currentInsert = (inserter->currentInsert + 1) % inserter->size;
        // sem_post(&inserter->wrt);
        pthread_mutex_unlock(&inserter->insertLock);
    }
}

void addBatch(Inserter *inserter, TileBatch *tileBatch)
{
    // sem_wait(&inserter->wrt);
    // Critical section
    sem_post(&inserter->wrt);
    // pthread_mutex_unlock(&inserter->readLock);
    pthread_mutex_lock(&inserter->insertLock);
    inserter->batches[inserter->currentRead] = tileBatch;
    inserter->currentRead = (inserter->currentRead + 1) % inserter->size;
    pthread_mutex_unlock(&inserter->insertLock);
    // sem_post(&inserter->wrt);
}