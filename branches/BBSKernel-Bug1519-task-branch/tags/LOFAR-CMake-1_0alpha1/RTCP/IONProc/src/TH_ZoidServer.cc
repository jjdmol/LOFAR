//# TH_ZoidServer.cc: In-memory transport mechanism
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#if defined HAVE_ZOID

#include <Common/Timer.h>
#include <Transport/DataHolder.h>
#include <TH_ZoidServer.h>

#include <algorithm>


namespace LOFAR {
namespace RTCP {

extern "C"
{
#include <lofar.h>
  void *lofar_cn_to_ion_zerocopy_allocate_cb(int len);
}


//std::vector<TH_ZoidServer *> TH_ZoidServer::theirTHs;
std::vector<TH_ZoidServer *> TH_ZoidServer::theirTHs;


void TH_ZoidServer::sendCompleted(void * /*buf*/, void *arg)
{
  TH_ZoidServer *th = static_cast<TH_ZoidServer *>(arg);

  if (th->bytesToSend == 0)
    pthread_cond_signal(&th->dataSent);

  pthread_mutex_unlock(&th->sendMutex);
}

ssize_t lofar_ion_to_cn_zerocopy(void   * /*buf*/ /* out:arr:size=+1:zerocopy:userbuf */,
				 size_t *count /* inout:ptr */)
{
  TH_ZoidServer *th = TH_ZoidServer::theirTHs[__zoid_calling_process_id()];
  //LOG_DEBUG_STR("lofar_ion_to_cn_zerocopy(..., " << *count << "), __zoid_calling_process_id() = " << __zoid_calling_process_id());

  pthread_mutex_lock(&th->sendMutex);

  while (th->bytesToSend == 0)
    pthread_cond_wait(&th->newSendDataAvailable, &th->sendMutex);

  if (*count > th->bytesToSend)
    *count = th->bytesToSend;

  __zoid_register_userbuf(th->sendBufferPtr, TH_ZoidServer::sendCompleted, th);
  th->sendBufferPtr += *count;
  th->bytesToSend   -= *count;

  return *count;
}


ssize_t lofar_ion_to_cn_onecopy(void   *buf /* out:arr:size=+1 */,
				size_t *count /* inout:ptr */)
{
  TH_ZoidServer *th = TH_ZoidServer::theirTHs[__zoid_calling_process_id()];
  //LOG_DEBUG_STR("lofar_ion_to_cn_onecopy(..., " << *count << "), __zoid_calling_process_id() = " << __zoid_calling_process_id());

  pthread_mutex_lock(&th->sendMutex);

  while (th->bytesToSend == 0)
    pthread_cond_wait(&th->newSendDataAvailable, &th->sendMutex);

  if (*count > th->bytesToSend)
    *count = th->bytesToSend;

  memcpy(buf, th->sendBufferPtr, *count);
  th->sendBufferPtr += *count;
  th->bytesToSend   -= *count;

  if (th->bytesToSend == 0)
    pthread_cond_signal(&th->dataSent);

  pthread_mutex_unlock(&th->sendMutex);

  return *count;
}


void *lofar_cn_to_ion_zerocopy_allocate_cb(int /*len*/)
{
  TH_ZoidServer *th = TH_ZoidServer::theirTHs[__zoid_calling_process_id()];
  pthread_mutex_lock(&th->receiveMutex);

  while (th->bytesToReceive == 0)
    pthread_cond_wait(&th->newReceiveBufferAvailable, &th->receiveMutex);

  return TH_ZoidServer::theirTHs[__zoid_calling_process_id()]->receiveBufferPtr;
  // still holding lock
}


ssize_t lofar_cn_to_ion_zerocopy(const void * /*buf*/ /* in:arr:size=+1:zerocopy:userbuf */,
				 size_t	    count /* in:obj */)
{
  // still holding lock

  TH_ZoidServer *th = TH_ZoidServer::theirTHs[__zoid_calling_process_id()];

  if (count > th->bytesToReceive)
    count = th->bytesToReceive;

  th->receiveBufferPtr += count;
  th->bytesToReceive   -= count;

  if (th->bytesToReceive == 0)
    pthread_cond_signal(&th->dataReceived);

  pthread_mutex_unlock(&th->receiveMutex);
  return count;
}


ssize_t lofar_cn_to_ion_onecopy(const void *buf /* in:arr:size=+1 */,
				size_t	   count /* in:obj */)
{
  TH_ZoidServer *th = TH_ZoidServer::theirTHs[__zoid_calling_process_id()];

  pthread_mutex_lock(&th->receiveMutex);

  while (th->bytesToReceive == 0)
    pthread_cond_wait(&th->newReceiveBufferAvailable, &th->receiveMutex);

  if (count > th->bytesToReceive)
    count = th->bytesToReceive;

  memcpy(TH_ZoidServer::theirTHs[__zoid_calling_process_id()]->receiveBufferPtr, buf, count);
  th->receiveBufferPtr += count;
  th->bytesToReceive   -= count;

  if (th->bytesToReceive == 0)
    pthread_cond_signal(&th->dataReceived);

  pthread_mutex_unlock(&th->receiveMutex);
  return count;
}


TH_ZoidServer::TH_ZoidServer(unsigned core)
:
  itsCore(core),
  bytesToSend(0),
  bytesToReceive(0)
{
  if (theirTHs.size() <= core)
    theirTHs.resize(core + 1);

  theirTHs[core] = this;

  pthread_cond_init(&newSendDataAvailable, 0);
  pthread_cond_init(&newReceiveBufferAvailable, 0);
  pthread_cond_init(&dataSent, 0);
  pthread_cond_init(&dataReceived, 0);
  pthread_mutex_init(&sendMutex, 0);
  pthread_mutex_init(&receiveMutex, 0);
}


TH_ZoidServer::~TH_ZoidServer()
{
  *std::find(theirTHs.begin(), theirTHs.end(), this) = 0;

  pthread_cond_destroy(&newSendDataAvailable);
  pthread_cond_destroy(&newReceiveBufferAvailable);
  pthread_cond_destroy(&dataSent);
  pthread_cond_destroy(&dataReceived);
  pthread_mutex_destroy(&sendMutex);
  pthread_mutex_destroy(&receiveMutex);
}


#if 0
void TH_ZoidServer::createAllTH_ZoidServers(unsigned nrCoresPerPset)
{
  theirTHs.resize(nrCoresPerPset);

  for (unsigned core = 0; core < nrCoresPerPset; core ++)
    theirTHs[core] = new TH_ZoidServer(core);
}


void TH_ZoidServer::deleteAllTH_ZoidServers()
{
  for (unsigned core = 0; core < theirTHs.size(); core ++)
    delete theirTHs[core];

  theirTHs.clear();
}
#endif


bool TH_ZoidServer::init()
{
  return true;
}


bool TH_ZoidServer::sendBlocking(void *buf, int nbytes, int, DataHolder *)
{
  //LOG_DEBUG_STR("TH_ZoidServer::sendBlocking(" << buf << ", " << nbytes << ", ...)");
  pthread_mutex_lock(&sendMutex);

  sendBufferPtr = static_cast<char *volatile>(buf);
  bytesToSend = nbytes;
  pthread_cond_signal(&newSendDataAvailable);

  while (bytesToSend > 0)
    pthread_cond_wait(&dataSent, &sendMutex);

  pthread_mutex_unlock(&sendMutex);

  return true;
}


bool TH_ZoidServer::recvBlocking(void *buf, int nbytes, int, int, DataHolder *)
{
  //LOG_DEBUG_STR("TH_ZoidServer::recvBlocking(" << buf << ", " << nbytes << ", ...)");
  pthread_mutex_lock(&receiveMutex);
  receiveBufferPtr = static_cast<char *volatile>(buf);
  pthread_cond_signal(&newReceiveBufferAvailable);

  for (bytesToReceive = nbytes; bytesToReceive > 0;)
    pthread_cond_wait(&dataReceived, &receiveMutex);

  pthread_mutex_unlock(&receiveMutex);

  return true;
}


// functions below are not supported

int32 TH_ZoidServer::recvNonBlocking(void *, int32, int, int32, DataHolder *)
{
  return false;
}


void TH_ZoidServer::waitForReceived(void *, int, int)
{
}


bool TH_ZoidServer::sendNonBlocking(void *, int, int, DataHolder *)
{
  return false;
}


void TH_ZoidServer::waitForSent(void *, int, int)
{
}


string TH_ZoidServer::getType() const
{
  return "TH_ZoidServer";
}


bool TH_ZoidServer::isClonable() const
{
  return true;
}


TransportHolder *TH_ZoidServer::clone() const
{
  return new TH_ZoidServer(itsCore);
}


void TH_ZoidServer::reset()
{
}

} // namespace RTCP
} // namespace LOFAR

#endif
