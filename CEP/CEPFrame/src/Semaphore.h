#ifndef  SEMAPHORE_H
#define  SEMAPHORE_H

#include <pthread.h>
 
class Semaphore
{
    Semaphore(unsigned level = 0);
    ~Semaphore();

    void up(unsigned count = 1);
    void down(unsigned count = 1);
    
  private:
    pthread_mutex_t mutex;
    pthread_cond_t  condition;
    unsigned	    level;
};
 
#endif
