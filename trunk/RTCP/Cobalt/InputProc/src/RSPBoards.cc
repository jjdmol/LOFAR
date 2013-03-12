#include <lofar_config.h>
#include "RSPBoards.h"
#include "OMPThread.h"
#include <Common/LofarLogger.h>
#include <omp.h>

namespace LOFAR
{
  namespace Cobalt
  {


    RSPBoards::RSPBoards( const std::string &logPrefix, size_t nrBoards )
      :
      logPrefix(logPrefix),
      nrBoards(nrBoards)
    {
    }

    void RSPBoards::process()
    {
      // References to all threads that will need aborting
      std::vector<OMPThread> threads(nrBoards * 2);

      ASSERT(nrBoards > 0);

      LOG_INFO_STR( logPrefix << "Start" );

# pragma omp parallel sections num_threads(3)
      {
        // Board threads
#   pragma omp section
        {
          // start all boards
          LOG_INFO_STR( logPrefix << "Starting all boards" );
#     pragma omp parallel for num_threads(nrBoards)
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

        // Log threads
#   pragma omp section
        {
          // start all log statistics
          LOG_INFO_STR( logPrefix << "Starting all log statistics" );
#     pragma omp parallel for num_threads(nrBoards)
          for (size_t i = 0; i < nrBoards; ++i) {
            OMPThread::ScopedRun sr(threads[i + nrBoards]);

            try {
              for(;; ) {
                if (usleep(999999) == -1 && errno == EINTR)
                  // got killed
                  break;

                logStatistics();
              }
            } catch(Exception &ex) {
              LOG_ERROR_STR("Caught exception: " << ex);
            }
          }
        }

        // Watcher thread
#   pragma omp section
        {
          // wait until we have to stop
          LOG_INFO_STR( logPrefix << "Waiting for stop signal" );
          waiter.waitForever();

          // kill all boards
          LOG_INFO_STR( logPrefix << "Stopping all boards" );
#     pragma omp parallel for num_threads(threads.size())
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
