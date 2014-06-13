#include <lofar_config.h>
#include "MPIUtil.h"
#include "MPIUtil2.h"

#include <iomanip>

#include <Common/LofarLogger.h>

//#define DEBUG_MPI

#ifdef DEBUG_MPI
#define DEBUG(str)  LOG_DEBUG_STR(str)
#else
#define DEBUG(str)
#endif

#include <ctime>
#include <boost/lexical_cast.hpp>
#include <Common/Thread/Mutex.h>
#include <CoInterface/SmartPtr.h>

using namespace std;

namespace LOFAR {

  namespace Cobalt {

    Mutex MPIMutex;

    MPI::MPI()
    :
      itsRank(0),
      itsSize(1),
      itsIsInitialised(false)
    {
#ifdef HAVE_MPI
      // Pull rank/size from environment, as MPI_init has
      // not yet been called
      const char *rankstr, *sizestr;

      // OpenMPI rank
      if ((rankstr = getenv("OMPI_COMM_WORLD_RANK")) != NULL)
        itsRank = boost::lexical_cast<int>(rankstr);

      // MVAPICH2 rank
      if ((rankstr = getenv("MV2_COMM_WORLD_RANK")) != NULL)
        itsRank = boost::lexical_cast<int>(rankstr);

      // OpenMPI size
      if ((sizestr = getenv("OMPI_COMM_WORLD_SIZE")) != NULL)
        itsSize = boost::lexical_cast<int>(sizestr);

      // MVAPICH2 size
      if ((sizestr = getenv("MV2_COMM_WORLD_SIZE")) != NULL)
        itsSize = boost::lexical_cast<int>(sizestr);
#endif
    }

    void MPI::init(int argc, char **argv)
    {
      itsIsInitialised = true;

  #ifdef HAVE_MPI
      ScopedLock sl(MPIMutex);

      // Initialise and query MPI
      int provided_mpi_thread_support;

      LOG_INFO("----- Initialising MPI");
      if (MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided_mpi_thread_support) != MPI_SUCCESS) {
        LOG_FATAL("MPI_Init_thread failed");
        exit(1);
      }

      // Verify the rank/size settings we assumed earlier
      int real_rank;
      int real_size;

      MPI_Comm_rank(MPI_COMM_WORLD, &real_rank);
      MPI_Comm_size(MPI_COMM_WORLD, &real_size);

      ASSERT(itsRank == real_rank);
      ASSERT(itsSize == real_size);

      MPIPoll::instance().start();
  #endif
    }

    MPI::~MPI()
    {
      if (!initialised())
        return;

  #ifdef HAVE_MPI
      ScopedLock sl(MPIMutex);

      MPIPoll::instance().stop();

      MPI_Finalize();
  #endif
    }

    MPI mpi;

    void *MPIAllocator::allocate(size_t size, size_t alignment)
    {
      ScopedLock sl(MPIMutex);

      ASSERT(alignment == 1); // Don't support anything else yet, although MPI likely aligns for us
      ASSERT(mpi.initialised());

      void *ptr;

      int error = ::MPI_Alloc_mem(size, MPI_INFO_NULL, &ptr);
      ASSERT(error == MPI_SUCCESS);

      return ptr;
    }


    void MPIAllocator::deallocate(void *ptr)
    {
      ScopedLock sl(MPIMutex);

      int error = ::MPI_Free_mem(ptr);
      ASSERT(error == MPI_SUCCESS);
    }

    MPIAllocator mpiAllocator;

    namespace {
      typedef int (*MPI_SEND)(void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request*);

      // Generic send method
      MPI_Request Guarded_MPI_Send(MPI_SEND sendMethod, const void *ptr, size_t numBytes, int destRank, int tag) {
        DEBUG("SEND: size " << numBytes << " tag " << hex << tag << dec << " to " << destRank);

        ASSERT(numBytes > 0);
        ASSERT(tag >= 0); // Silly MPI requirement

        //SmartPtr<ScopedLock> sl = MPI_threadSafe() ? 0 : new ScopedLock(MPIMutex);

        MPI_Request request;

        int error;

        error = sendMethod(const_cast<void*>(ptr), numBytes, MPI_BYTE, destRank, tag, MPI_COMM_WORLD, &request);
        ASSERT(error == MPI_SUCCESS);

        return request;
      }
    }


    MPI_Request Guarded_MPI_Issend(const void *ptr, size_t numBytes, int destRank, int tag) {
      return Guarded_MPI_Send(::MPI_Issend, ptr, numBytes, destRank, tag);
    }

    MPI_Request Guarded_MPI_Irsend(const void *ptr, size_t numBytes, int destRank, int tag) {
      return Guarded_MPI_Send(::MPI_Irsend, ptr, numBytes, destRank, tag);
    }

    MPI_Request Guarded_MPI_Ibsend(const void *ptr, size_t numBytes, int destRank, int tag) {
      return Guarded_MPI_Send(::MPI_Ibsend, ptr, numBytes, destRank, tag);
    }

    MPI_Request Guarded_MPI_Isend(const void *ptr, size_t numBytes, int destRank, int tag) {
      return Guarded_MPI_Send(::MPI_Isend, ptr, numBytes, destRank, tag);
    }

    MPI_Request Guarded_MPI_Irecv(void *ptr, size_t numBytes, int srcRank, int tag) {
      DEBUG("RECV: size " << numBytes << " tag " << hex << tag);

      ASSERT(tag >= 0); // Silly MPI requirement

      //SmartPtr<ScopedLock> sl = MPI_threadSafe() ? 0 : new ScopedLock(MPIMutex);

      MPI_Request request;

      int error;

      error = ::MPI_Irecv(ptr, numBytes, MPI_BYTE, srcRank, tag, MPI_COMM_WORLD, &request);
      ASSERT(error == MPI_SUCCESS);

      return request;
    }

  }
}

