#include "StationStreams.h"
#include "OMPThread.h"
#include <Common/LofarLogger.h>
#include <omp.h>

namespace LOFAR {
namespace RTCP {


StationStreams::StationStreams( const std::string &logPrefix, const StationSettings &settings, const std::vector<std::string> &streamDescriptors )
:
  logPrefix(logPrefix),
  settings(settings),
  streamDescriptors(streamDescriptors),
  nrBoards(streamDescriptors.size())
{
}

void StationStreams::process()
{
  std::vector<OMPThread> threads(nrBoards);

  ASSERT(nrBoards > 0);

  LOG_INFO_STR( logPrefix << "Start" );

  #pragma omp parallel sections num_threads(2)
  {
    #pragma omp section
    {
      // start all boards
      LOG_INFO_STR( logPrefix << "Starting all boards" );
      #pragma omp parallel for num_threads(nrBoards)
      for (size_t i = 0; i < nrBoards; ++i) {
        OMPThread::ScopedRun sr(threads[i]);
  
        processBoard(i);
      }
    }

    #pragma omp section
    {
      // wait until we have to stop
      LOG_INFO_STR( logPrefix << "Waiting for stop signal" );
      waiter.waitForever();     

      // kill all boards
      LOG_INFO_STR( logPrefix << "Stopping all boards" );
      #pragma omp parallel for num_threads(nrBoards)
      for (size_t i = 0; i < nrBoards; ++i)
        threads[i].kill();
    }
  }

  LOG_INFO_STR( logPrefix << "End" );
}

void StationStreams::stop()
{
  waiter.cancelWait();
}


}
}
