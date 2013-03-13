#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <CoInterface/RSPTimeStamp.h>

using namespace LOFAR;
using namespace Cobalt;

int main( int, char **argv )
{
  INIT_LOGGER(argv[0]);

  unsigned clock = 200 * 1000 * 1000;

  {
    TimeStamp ts(0, 0, clock);

    for (int64 i = 0; i < clock * 3; ++i, ++ts) {
      #define REPORT "(ts == " << ts << ", i == " << i << ")"

      ASSERTSTR( (int64)ts == i, REPORT );

      ASSERTSTR( ts.getSeqId() == 1024 * i / clock, REPORT );

      ASSERTSTR( ts.getBlockId() == 1024 * i % clock / 1024, REPORT );
    }
  }
}
