#include <lofar_config.h>
#include "SampleBuffer.h"
#include "SampleType.h"
#include "OMPThread.h"
#include <Common/LofarLogger.h>
#include <vector>
#include <string>
#include "omp.h"

using namespace LOFAR;
using namespace RTCP;
using namespace std;

// Duration of the test (seconds)
#define DURATION 2

// The number of packets to transmit (note: there are 16 time samples/packet)
#define NUMPACKETS (200000000/1024/16)

template<typename T> void test( struct BufferSettings &settings )
{
  SampleBuffer< SampleType<T> > buffer_create(settings, true);

  SampleBuffer< SampleType<T> > buffer_read(settings, false);
}


int main( int, char **argv ) {
  INIT_LOGGER( argv[0] );

  // Don't run forever if communication fails for some reason
  alarm(10);

  omp_set_nested(true);
  omp_set_num_threads(16);

  OMPThread::init();

  unsigned clock = 200 * 1000 * 1000;
  struct StationID stationID("RS106", "LBA", clock, 16);
  struct BufferSettings settings;

  settings.station = stationID;
  settings.nrBeamlets = 61;
  settings.nrBoards = 1;

  settings.nrSamples = (2 * stationID.clock / 1024);// & ~0xFL;
  settings.nrFlagRanges = 64;

  settings.dataKey = 0x12345678;

  LOG_INFO("Test 16-bit complex");
  test<i16complex>(settings);

  LOG_INFO("Test 8-bit complex");
  test<i8complex>(settings);

  LOG_INFO("Test 4-bit complex");
  test<i4complex>(settings);


  return 0;
}

