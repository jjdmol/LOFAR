#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <CoInterface/Stream.h>
#include "OMPThread.h"
#include "Station/Generator.h"
#include "Station/PacketReader.h"
#include <vector>
#include <string>
#include <unistd.h>
#include "omp.h"

using namespace LOFAR;
using namespace Cobalt;
using namespace std;

// The number of packets to transmit (note: there are 16 time samples/packet)
#define NUMPACKETS (200000000 / 1024 / 16)

int main( int, char **argv )
{
  INIT_LOGGER( argv[0] );

  // Don't run forever if communication fails for some reason
  alarm(10);

  omp_set_nested(true);
  omp_set_num_threads(16);

  OMPThread::init();

  vector<string> streamDescs(1, "tcp:localhost:54321");

  struct StationID stationID("RS106", "LBA", 200, 16);
  struct BufferSettings settings(stationID, false);

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

          if (!reader.readPacket(packet, settings)) {
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

