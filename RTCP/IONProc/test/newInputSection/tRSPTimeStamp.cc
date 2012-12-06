#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Interface/RSPTimeStamp.h>

int main( int, char **argv )
{
  INIT_LOGGER(argv[0]);

  unsigned clock = 200 * 1000 * 1000;

  {
    TimeStamp ts(0,0,clock);

    for (int64 i = 0; i < clock * 3; ++i) {
      ++ts;

      #define REPORT "(ts == " << ts << ", i == " << i << ")"

      ASSERTSTR( (int64)ts == i, REPORT );

      ASSERTSTR( ts.getSeqId() == i / clock, REPORT );

      ASSERTSTR( ts.getBlockId() == i % clock, REPORT );
    }
  }
}
