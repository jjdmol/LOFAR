#include <lofar_config.h>
#include "Generator.h"
#include "PacketReader.h"
#include "OMPThread.h"
#include <Common/LofarLogger.h>
#include <Interface/Stream.h>
#include <vector>
#include <string>
#include <unistd.h>
#include "omp.h"

using namespace LOFAR;
using namespace RTCP;
using namespace std;

// Duration of the test (seconds)
#define DURATION 2

// The number of packets to transmit (note: there are 16 time samples/packet)
#define NUMPACKETS (200000000/1024/16)


int main( int, char **argv ) {
  INIT_LOGGER( argv[0] );

  // Don't run forever if communication fails for some reason
  alarm(10);

  omp_set_nested(true);
  omp_set_num_threads(16);

  OMPThread::init();

  vector<string> streamDescs(1, "tcp:localhost:54321");

  struct StationID stationID("RS106", "LBA", 200, 16);
  struct BufferSettings settings;

  settings.station = stationID;
  settings.nrBeamlets = 61;
  settings.nrBoards = 1;

  settings.nrSamples = (2 * stationID.clockMHz * 1000000 / 1024);// & ~0xFL;
  settings.nrFlagRanges = 64;

  settings.dataKey = stationID.hash();

  Generator g(settings, streamDescs);

  bool error = false;

  #pragma omp parallel sections num_threads(2)
  {
    #pragma omp section
    {
      // Generate packets

      try {
        g.process();
      } catch(Exception &ex) {
        LOG_ERROR_STR("Caught exception: " << ex);
        error = true;
      }
    }

    #pragma omp section
    { 
      // Read and verify the generated packets

      try {
        SmartPtr<Stream> inputStream = createStream(streamDescs[0], true);
        PacketReader reader("", *inputStream);

        for(size_t nr = 0; nr < NUMPACKETS; ++nr) {
          struct RSP packet;

          if (!reader.readPacket(packet)) {
            reader.logStatistics();

            ASSERT(false);
          }
        }

        // We received NUMPACKETS packets, kill the generator
        g.stop();
      } catch(Exception &ex) {
        LOG_ERROR_STR("Caught exception: " << ex);
        error = true;
      }
    }
  }

  return error ? 1 : 0;
}

