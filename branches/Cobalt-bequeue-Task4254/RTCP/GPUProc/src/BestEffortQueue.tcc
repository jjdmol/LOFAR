#include "BestEffortQueue.h"

namespace LOFAR
{
    namespace RTCP 
    {

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
