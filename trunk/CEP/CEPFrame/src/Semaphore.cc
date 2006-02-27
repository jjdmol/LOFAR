#include "Semaphore.h"


Semaphore::Semaphore(unsigned level)
:
    level(level)
{
    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&condition, 0);
}


Semaphore::~Semaphore()
{
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&mutex);
}


void Semaphore::up(unsigned count)
{
    pthread_mutex_lock(&mutex);
    level += count;

    if (count == 1)
	pthread_cond_signal(&condition);
    else
	pthread_cond_broadcast(&condition);

    pthread_mutex_unlock(&mutex);
}


void Semaphore::down(unsigned count)
{
    pthread_mutex_lock(&mutex);

    while (level < count)
	pthread_cond_wait(&condition, &mutex);

    level -= count;
    pthread_mutex_unlock(&mutex);
}
