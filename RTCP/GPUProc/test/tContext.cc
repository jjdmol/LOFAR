#include <lofar_config.h>

#include <OpenCL_Support.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace RTCP;
using namespace std;

// test OpenCL context creation
void test_create() {
  cl::Context context;
  vector<cl::Device> devices;

  createContext(context, devices);
}

int main() {
  INIT_LOGGER( "tContext" );

  test_create();

  return 0;
}
