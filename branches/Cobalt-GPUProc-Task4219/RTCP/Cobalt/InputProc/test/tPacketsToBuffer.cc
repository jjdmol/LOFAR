#include <lofar_config.h>
#include "Station/PacketsToBuffer.h"
#include "Buffer/SampleBuffer.h"
#include "SampleType.h"
#include <Common/LofarLogger.h>
#include <Stream/FileStream.h>
#include <string>
#include <time.h>

using namespace LOFAR;
using namespace RTCP;
using namespace std;

template<typename T>
void test( struct BufferSettings &settings, const std::string &filename )
{
  // Create the buffer to keep it around after transfer.process(), or there
  // will be no subscribers and transfer will delete the buffer automatically,
  // at which point we can't attach anymore.
  SampleBuffer< SampleType<T> > buffer(settings, true);

  // Read packets from file
  FileStream fs(filename);

  // Set up transfer
  PacketsToBuffer transfer(fs, settings, 0);

  // Do transfer
  transfer.process();

  // There should be 32 samples in the buffer (16 per packet, 2 packets per
  // file).
  int64 now = (int64)TimeStamp(time(0) + 1, 0, settings.station.clockMHz * 1000000);
  SparseSet<int64> available = buffer.flags[0].sparseSet(0, now);
  ASSERT((size_t)available.count() == 32);
}


int main()
{
  INIT_LOGGER( "tPacketsToBuffer" );

  // Don't run forever if communication fails for some reason
  alarm(10);

  // Fill a BufferSettings object
  struct StationID stationID("RS106", "LBA", 200, 16);
  struct BufferSettings settings(stationID, false);

  // Test various modes
  LOG_INFO("Test 16-bit complex");
  settings.station.bitMode = 16;
  settings.nrBeamletsPerBoard = 61;
  test<i16complex>(settings, "tPacketsToBuffer.in_16bit");

  LOG_INFO("Test 8-bit complex");
  settings.station.bitMode = 8;
  settings.nrBeamletsPerBoard = 122;
  test<i8complex>(settings, "tPacketsToBuffer.in_8bit");

  return 0;
}

