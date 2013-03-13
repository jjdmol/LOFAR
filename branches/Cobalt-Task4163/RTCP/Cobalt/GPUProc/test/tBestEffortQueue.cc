#include <lofar_config.h>

#include <OpenCL_Support.h>
#include <BestEffortQueue.h>
#include <unistd.h>

using namespace LOFAR;
using namespace Cobalt;
using namespace std;

void test_drop()
{
  // check if blocks are dropped if queue is full
  size_t queueSize = 10;
  BestEffortQueue<size_t> queue(queueSize, true);

  // queue has free space -- append should succeed
  for (size_t i = 0; i < queueSize; ++i) {
    ASSERT(queue.append(100 + i));
    ASSERT(queue.size() == i + 1);
  }

  // queue is full -- append should fail
  ASSERT(!queue.append(1000));
  ASSERT(queue.size() == queueSize);

  // removal should succeed
  for (size_t i = 0; i < queueSize; ++i) {
    ASSERT(queue.remove() == 100 + i);
    ASSERT(queue.size() == queueSize - i - 1);
  }

  ASSERT(queue.empty());
}

void test_nondrop()
{
  size_t queueSize = 10;
  BestEffortQueue<size_t> queue(queueSize, false);

  // queue has free space -- append should succeed
  for (size_t i = 0; i < queueSize; ++i) {
    ASSERT(queue.append(100 + i));
    ASSERT(queue.size() == i + 1);
  }

# pragma omp parallel sections
  {
#   pragma omp section
    {
      // push more -- append should always succeed
      for (size_t i = 0; i < queueSize; ++i) {
        ASSERT(queue.append(100 + i));
      }
    }

#   pragma omp section
    {
      // pop everything
      for (size_t i = 0; i < queueSize * 2; ++i) {
        ASSERT(queue.remove() > 0);
      }
    }
  }

  ASSERT(queue.empty());
}

void test_nomore()
{
  size_t queueSize = 10;
  BestEffortQueue<size_t> queue(queueSize, false);

  // fill queue
  for (size_t i = 0; i < queueSize; ++i) {
    ASSERT(queue.append(100 + i));
  }

  // end-of-stream
  queue.noMore();

  // can't append anymore
  ASSERT(!queue.append(1));

  // should be able to empty queue until we hit 0
  for (size_t i = 0; i < queueSize; ++i) {
    ASSERT(queue.remove() > 0);
  }

  // 0 signals end-of-queue
  ASSERT(queue.remove() == 0);
  ASSERT(queue.empty());

  // can't append anymore
  ASSERT(!queue.append(1));
}

int main()
{
  INIT_LOGGER( "tBestEffortQueue" );

  // abort program if code blocks
  alarm(5);

  test_drop();
  test_nondrop();
  test_nomore();

  return 0;
}
