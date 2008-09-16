#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_ASYNC_COMMUNICATION_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_ASYNC_COMMUNICATION_H

#if defined HAVE_MPI
#define MPICH_IGNORE_CXX_SEEK
#include <mpi.h>
#endif

#include <map>

namespace LOFAR {
namespace CS1 {

#if defined HAVE_MPI

class AsyncRequest {
public:
  MPI_Request mpiReq;
  void* buf;
  unsigned size;
  unsigned rank;
  int tag;
};

class AsyncCommunication {
  public:
    AsyncCommunication(MPI_Comm communicator = MPI_COMM_WORLD);
    ~AsyncCommunication();

    // returns handle to this read
    int asyncRead(void* buf, unsigned size, unsigned source, int tag);

    // returns handle to this write
    int asyncWrite(const void* buf, unsigned size, unsigned dest, int tag);

    void waitForRead(int handle);
    void waitForWrite(int handle);

    // returns the handle of the read that was done.
    int waitForAnyRead(void*& buf, unsigned& size, unsigned& source, int& tag);

    void waitForAllReads();
    void waitForAllWrites();
			      
private:

    MPI_Comm itsCommunicator;
    int itsCurrentReadHandle;
    int itsCurrentWriteHandle;
    std::map<int, AsyncRequest*> itsReadHandleMap;
    std::map<int, AsyncRequest*> itsWriteHandleMap;
};

#endif // defined HAVE_MPI

} // namespace CS1
} // namespace LOFAR

#endif
