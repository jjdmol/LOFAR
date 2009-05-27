//# Always <lofar_config.h> first!
#include <lofar_config.h>

#include <AsyncCommunication.h>

#include <Common/Timer.h>
#include <Interface/Exceptions.h>

#include <cassert>
#include <map>
#include <iostream>

#define USE_TIMING 0

namespace LOFAR {
namespace RTCP {

#if defined HAVE_MPI

AsyncCommunication::AsyncCommunication(MPI_Comm comm)
:
  itsCommunicator(comm),
  itsCurrentReadHandle(0),
  itsCurrentWriteHandle(0)
{
}

// returns handle to this read
int AsyncCommunication::asyncRead(void* buf, unsigned size, unsigned source, int tag)
{
    AsyncRequest* req = new AsyncRequest();

    int res = MPI_Irecv(buf, size, MPI_BYTE, source, tag, itsCommunicator, &req->mpiReq);
    if (res != MPI_SUCCESS) {
	THROW(CNProcException,"MPI_Irecv() failed");
    }

    req->buf = buf;
    req->size = size;
    req->rank = source;
    req->tag = tag;

    int handle = itsCurrentReadHandle++;
    itsReadHandleMap[handle] = req;

    return handle;
}

// returns handle to this write
int AsyncCommunication::asyncWrite(const void* buf, unsigned size, unsigned dest, int tag)
{
    AsyncRequest* req = new AsyncRequest();

    int res = MPI_Isend((void*)buf, size, MPI_BYTE, dest, tag, itsCommunicator, &req->mpiReq);
    if (res != MPI_SUCCESS) {
	THROW(CNProcException,"MPI_Isend() failed");
    }

    req->buf = (void*)buf;
    req->size = size;
    req->rank = dest;
    req->tag = tag;

    int handle = itsCurrentWriteHandle++;
    itsWriteHandleMap[handle] = req;

    return handle;
}


void AsyncCommunication::waitForRead(int handle)
{
    AsyncRequest* req = itsReadHandleMap[handle];
    MPI_Status status;

    int res = MPI_Wait(&req->mpiReq, &status);
    if (res != MPI_SUCCESS) {
	THROW(CNProcException,"MPI_Wait() failed");
    }

    // done, now remove from map, and free req
    itsReadHandleMap.erase(handle);
    delete req;
}

void AsyncCommunication::waitForWrite(int handle)
{
    AsyncRequest* req = itsWriteHandleMap[handle];
    MPI_Status status;

    int res = MPI_Wait(&req->mpiReq, &status);
    if (res != MPI_SUCCESS) {
	THROW(CNProcException,"MPI_Wait() failed");
    }

    // done, now remove from map, and free req
    itsWriteHandleMap.erase(handle);
    delete req;
}

// returns the handle of the read that was done.
int AsyncCommunication::waitForAnyRead(void*& buf, unsigned& size, unsigned& source, int& tag)
{
    MPI_Status status;
    int count = itsReadHandleMap.size();
    MPI_Request reqs[count];
    int mapping[count];

    ASSERT( count > 0 );
    
    int i = 0;
    for (std::map<int, AsyncRequest*>::const_iterator it = itsReadHandleMap.begin(); it != itsReadHandleMap.end(); it++) {
	int handle = it->first;
	AsyncRequest* r = it->second;
	reqs[i] = r->mpiReq;
	mapping[i] = handle;
	i++;
    }

    NSTimer waitAnyTimer("MPI_Waitany", USE_TIMING, true);
    waitAnyTimer.start();
    int index = -1;
    int res = MPI_Waitany(count, reqs, &index, &status);
    waitAnyTimer.stop();

    if (res != MPI_SUCCESS) {
	THROW(CNProcException,"MPI_Waitany() failed");
    }

    if (index == MPI_UNDEFINED) {
	THROW(CNProcException,"MPI_Waitany() failed: no (pending) receives");
    }

    int handle = mapping[index];
    AsyncRequest* req = itsReadHandleMap[handle];

    buf = req->buf;
    size = req->size;
    source = req->rank;
    tag = req->tag;

    itsReadHandleMap.erase(handle);
    delete req;
    return handle;
}


void AsyncCommunication::waitForAllReads()
{
    int count = itsReadHandleMap.size();
    MPI_Request reqs[count];
    MPI_Status status[count];

    if( count == 0 ) {
      // nothing to wait for
      return;
    }

    int i = 0;
    for (std::map<int, AsyncRequest*>::const_iterator it = itsReadHandleMap.begin(); it != itsReadHandleMap.end(); it++) {
	AsyncRequest* r = it->second;
	reqs[i] = r->mpiReq;
	i++;
    }

    int res = MPI_Waitall(count, reqs, status);
    if (res != MPI_SUCCESS) {
	THROW(CNProcException,"MPI_Waitall() failed");
    }

    for (std::map<int, AsyncRequest*>::const_iterator it = itsReadHandleMap.begin(); it != itsReadHandleMap.end(); it++) {
	AsyncRequest* r = it->second;
	delete r;
    }
    itsReadHandleMap.clear();
    itsCurrentReadHandle = 0;
}


void AsyncCommunication::waitForAllWrites()
{
    int count = itsWriteHandleMap.size();
    MPI_Request reqs[count];
    MPI_Status status[count];

    if( count == 0 ) {
      // nothing to wait for
      return;
    }

    int i = 0;
    for (std::map<int, AsyncRequest*>::const_iterator it = itsWriteHandleMap.begin(); it != itsWriteHandleMap.end(); it++) {
	AsyncRequest* r = it->second;
	reqs[i] = r->mpiReq;
	i++;
    }

    int res = MPI_Waitall(count, reqs, status);
    if (res != MPI_SUCCESS) {
	THROW(CNProcException,"MPI_Waitall() failed");
    }

    for (std::map<int, AsyncRequest*>::const_iterator it = itsWriteHandleMap.begin(); it != itsWriteHandleMap.end(); it++) {
	AsyncRequest* r = it->second;
	delete r;
    }
    itsWriteHandleMap.clear();
    itsCurrentWriteHandle = 0;
}


#endif // HAVE_MPI

} // namespace RTCPs
} // namespace LOFAR
