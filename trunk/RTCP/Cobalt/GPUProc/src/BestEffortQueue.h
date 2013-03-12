#ifndef GPUPROC_BESTEFFORTQUEUE_H
#define GPUPROC_BESTEFFORTQUEUE_H

#include "Common/Thread/Queue.h"
#include "Common/Thread/Semaphore.h"

namespace LOFAR
{
  namespace Cobalt
  {
    /*
     * Implements a best-effort queue. The queue has a maximum size,
     * at which point further append()s are blocked until an item
     * is removed. If `dropIfFull` is set, append() will not block,
     * but discard the item instead.
     *
     * The noMore() function signals the end-of-stream, at which point
     * an 0 or NULL element is inserted into the stream. The reader
     * thus has to consider the value 0 as end-of-stream.
     */
    template<typename T>
    class BestEffortQueue : public Queue<T>
    {
    public:
      // Create a best-effort queue with room for `maxSize' elements.
      // If `dropIfFull' is true, appends are dropped if the queue
      // has reached `maxSize'.
      BestEffortQueue(size_t maxSize, bool dropIfFull);

      // Add an element. Returns true if append succeeded, false if element
      // was dropped.
      bool append(T);

      // Remove an element -- 0 or NULL signals end-of-stream.
      T remove();

      // Signal end-of-stream.
      void noMore();

    private:
      const size_t maxSize;
      const bool dropIfFull;

      // contains the amount of free space in the queue
      Semaphore freeSpace;

      // true if the queue is being flushed
      bool flushing;
    };
  }
}

#include "BestEffortQueue.tcc"

#endif
