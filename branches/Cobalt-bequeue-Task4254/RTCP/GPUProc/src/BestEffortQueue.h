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

template <typename T> inline BestEffortQueue<T>::BestEffortQueue(size_t maxSize, bool dropIfFull)
:
  maxSize(maxSize),
  dropIfFull(dropIfFull),
  freeSpace(maxSize)
{
}


template <typename T> inline bool BestEffortQueue<T>::append(T element)
{
  bool canAppend;

  if (dropIfFull) {
    canAppend = freeSpace.tryDown();
  } else {
    canAppend = freeSpace.down();
  }

  if (canAppend) {
    Queue<T>::append(element);
  }

  return canAppend;
}


template <typename T> inline T BestEffortQueue<T>::remove()
{
  T element = Queue<T>::remove();

  // freed up one spot
  freeSpace.up();

  return element;
}


template <typename T> inline void BestEffortQueue<T>::noMore()
{
  // signal end-of-stream to reader
  Queue<T>::append(NULL);

  // prevent writer from appending
  freeSpace.noMore();
}


    }

}
#endif
