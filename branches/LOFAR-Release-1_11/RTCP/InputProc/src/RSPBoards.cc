#include <lofar_config.h>
#include "RSPBoards.h"
#include "OMPThread.h"
#include <Common/LofarLogger.h>
#include <omp.h>

namespace LOFAR {
namespace RTCP {


RSPBoards::RSPBoards( const std::string &logPrefix, const BufferSettings &settings, const std::vector<std::string> &streamDescriptors )
:
  logPrefix(logPrefix),
  settings(settings),
  streamDescriptors(streamDescriptors),
  nrBoards(streamDescriptors.size())
{
}

void RSPBoards::process()
{
  std::vector<OMPThread> threads(nrBoards * 2);

  ASSERT(nrBoards > 0);

  LOG_INFO_STR( logPrefix << "Start" );

  #pragma omp parallel sections num_threads(3)
  {
    #pragma omp section
    {
      // start all boards
      LOG_INFO_STR( logPrefix << "Starting all boards" );
      #pragma omp parallel for num_threads(nrBoards)
      for (size_t i = 0; i < nrBoards; ++i) {
        OMPThread::ScopedRun sr(threads[i]);
  
        try {
          processBoard(i);
        } catch(Exception &ex) {
          LOG_ERROR_STR("Caught exception: " << ex);
        }
      }

      // we're done
      stop();
    }

    #pragma omp section
    {
      // start all log statistics
      LOG_INFO_STR( logPrefix << "Starting all log statistics" );
      #pragma omp parallel for num_threads(nrBoards)
      for (size_t i = 0; i < nrBoards; ++i) {
        OMPThread::ScopedRun sr(threads[i + nrBoards]);
  
        try {
          for(;;) {
            if (usleep(999999) == -1 && errno == EINTR)
              break;

            logStatistics();
          }
        } catch(Exception &ex) {
          LOG_ERROR_STR("Caught exception: " << ex);
        }
      }
    }

    #pragma omp section
    {
      // wait until we have to stop
      LOG_INFO_STR( logPrefix << "Waiting for stop signal" );
      waiter.waitForever();     

      // kill all boards
      LOG_INFO_STR( logPrefix << "Stopping all boards" );
      #pragma omp parallel for num_threads(threads.size())
      for (size_t i = 0; i < threads.size(); ++i)
        try {
          threads[i].kill();
        } catch(Exception &ex) {
          LOG_ERROR_STR("Caught exception: " << ex);
        }
    }
  }

  LOG_INFO_STR( logPrefix << "End" );
}

void RSPBoards::stop()
{
  waiter.cancelWait();
}


}
}
