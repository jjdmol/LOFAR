#include "BestEffortQueue.h"

namespace LOFAR
{
    namespace RTCP 
    {

template <typename T> inline BestEffortQueue<T>::BestEffortQueue(size_t maxSize, bool dropIfFull)
:
  maxSize(maxSize),
  dropIfFull(dropIfFull),
  freeSpace(maxSize),
  flushing(false)
{
}


template <typename T> inline bool BestEffortQueue<T>::append(T element)
{
  bool canAppend;

  // can't append if we're emptying the queue
  if (flushing)
    return false;

  // determine whether we can append
  if (dropIfFull) {
    canAppend = freeSpace.tryDown();
  } else {
    canAppend = freeSpace.down();
  }

  // append if possible
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
  if (flushing)
    return;

  // mark queue as flushing
  flushing = true;

  // prevent writer from blocking
  freeSpace.noMore();

  // signal end-of-stream to reader
  Queue<T>::append(NULL);
}


    }

}
