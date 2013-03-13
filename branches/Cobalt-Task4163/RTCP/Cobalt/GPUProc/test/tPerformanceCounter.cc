#include <lofar_config.h>

#include <OpenCL_Support.h>
#include <PerformanceCounter.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace Cobalt;
using namespace std;

cl::Context context;
vector<cl::Device> devices;

// test a performance counter without events
void test_simple()
{
  PerformanceCounter counter("test", true);
}

// test a single event
void test_event()
{
  PerformanceCounter counter("test", true);

  // create a buffer and a queue to send the buffer
  cl::CommandQueue queue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);

  MultiArraySharedBuffer<float, 1> buffer(boost::extents[1024], queue, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY);

  // transfer the buffer and record the performance
  buffer.hostToDevice(CL_TRUE);
  counter.doOperation(buffer.event, 0, 0, buffer.bytesize());

  // wait for all scheduled events to pass
  counter.waitForAllOperations();

  struct PerformanceCounter::figures total = counter.getTotal();

  // validate results
  ASSERT(total.nrEvents == 1);

  ASSERT(total.nrBytesRead == 0);
  ASSERT(total.nrBytesWritten == buffer.bytesize());

  ASSERT(total.runtime > 0.0);
}

int main()
{
  INIT_LOGGER( "tPerformanceCounter" );

  createContext(context, devices);

  test_simple();
  test_event();

  return 0;
}
