#ifndef GPUPROC_BESTEFFORTQUEUE_H
#define GPUPROC_BESTEFFORTQUEUE_H

#include "Common/Thread/Queue.h"

namespace LOFAR
{
    namespace RTCP 
    {
        template<typename T> class BestEffortQueue: public Queue<T>
        {
        public:
          // Create a best-effort queue with room for `maxSize' elements.
          // If `dropIfFull' is true, appends are dropped if the queue
          // has reached `maxSize'.
          BestEffortQueue(size_t maxSize, bool dropIfFull);

          // Add an element. Returns true if append succeeded, false if element
          // was dropped.
          bool append(T);

          // Remove an element
          T remove();

          // Signal end-of-stream.
          void noMore();

        private:
          const size_t maxSize;
          const bool dropIfFull;

          // contains the amount of free space in the queue
          Semaphore freeSpace;
        };
    }
}

#include "BestEffortQueue.tcc"

#endif
