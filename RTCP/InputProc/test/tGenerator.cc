#include <lofar_config.h>
#include "Generator.h"
#include "PacketReader.h"
#include <Common/LofarLogger.h>
#include <Common/Thread/Thread.h>
#include <Interface/SmartPtr.h>
#include <Interface/Stream.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <unistd.h>
#include "omp.h"

using namespace LOFAR;
using namespace RTCP;
using namespace std;

// Duration of the test (seconds)
#define DURATION 2

void sighandler(int)
{
  /* no-op */
}

int main( int, char **argv ) {
  INIT_LOGGER( argv[0] );

  omp_set_nested(true);
  omp_set_num_threads(16);

  signal(SIGHUP, sighandler);
  siginterrupt(SIGHUP, 1);

  vector<string> streamDescs(1, "tcp:localhost:54321");

  unsigned clock = 200 * 1000 * 1000;
  struct StationID stationID("RS106", "LBA", clock, 16);
  struct BufferSettings settings;

  settings.station = stationID;
  settings.nrBeamlets = 61;
  settings.nrBoards = 1;

  settings.nrSamples = (2 * stationID.clock / 1024);// & ~0xFL;
  settings.nrFlagRanges = 64;

  settings.dataKey = stationID.hash();

  Generator g(settings, streamDescs);

  #pragma omp parallel sections num_threads(3)
  {
    #pragma omp section
    {
      // Generate packets

      try {
        g.process();
      } catch(Exception &ex) {
        LOG_ERROR_STR("Caught exception: " << ex);
      }
    }

    #pragma omp section
    { 
      // Read and verify the generated packets

      try {
        PacketReader reader("", streamDescs[0], settings);

        for(;;) {
          struct RSP packet;

          if (!reader.readPacket(packet)) {
            reader.logStatistics();

            ASSERT(false);
          }
        }
      } catch(Stream::EndOfStreamException &ex) {
      } catch(Exception &ex) {
        LOG_ERROR_STR("Caught exception: " << ex);
      }
    }

    #pragma omp section
    { 
      // Stop the experiment after a while
      
      sleep(DURATION);
      g.stop();
    }
  }

  return 0;
}

