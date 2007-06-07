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

#include <Common/Timer.h>
#include <CS1_IONProc/TH_ZoidServer.h>
#include <Transport/DataHolder.h>

#include <cassert>

namespace LOFAR
{

extern "C"
{
#include <lofar.h>
  void *lofar_cn_to_ion_allocate_cb(int len);
}


std::vector<TH_ZoidServer *> TH_ZoidServer::theirTHs;


static void sendCompleted(void *buf, void *arg)
{
  TH_ZoidServer *th = static_cast<TH_ZoidServer *>(arg);

  if (th->bytesToSend == 0)
    pthread_cond_signal(&th->dataSent);

  pthread_mutex_unlock(&th->mutex);
}

ssize_t lofar_ion_to_cn(void * /*buf*/ /* out:arr:size=+1:zerocopy:userbuf */,
		        size_t *count /* inout:ptr */)
{
  TH_ZoidServer *th = TH_ZoidServer::theirTHs[__zoid_calling_process_id()];

  pthread_mutex_lock(&th->mutex);

  while (th->bytesToSend == 0)
    pthread_cond_wait(&th->newSendDataAvailable, &th->mutex);

  if (*count > th->bytesToSend)
    *count = th->bytesToSend;

  __zoid_register_userbuf(th->sendBufferPtr, sendCompleted, th);
  th->sendBufferPtr += *count;
  th->bytesToSend -= *count;

  return *count;
}


void *lofar_cn_to_ion_allocate_cb(int len)
{
  TH_ZoidServer *th = TH_ZoidServer::theirTHs[__zoid_calling_process_id()];
  pthread_mutex_lock(&th->mutex);

  while (th->bytesToReceive == 0)
    pthread_cond_wait(&th->newReceiveBufferAvailable, &th->mutex);

  return TH_ZoidServer::theirTHs[__zoid_calling_process_id()]->receiveBufferPtr;
  // still holding lock
}


ssize_t lofar_cn_to_ion(const void *buf /* in:arr:size=+1:zerocopy:userbuf */,
		        size_t count /* in:obj */)
{
  // still holding lock
  TH_ZoidServer *th = TH_ZoidServer::theirTHs[__zoid_calling_process_id()];

  if (count > th->bytesToReceive)
    count = th->bytesToReceive;

  th->receiveBufferPtr += count;
  th->bytesToReceive -= count;

  if (th->bytesToReceive == 0)
    pthread_cond_signal(&th->dataReceived);

  pthread_mutex_unlock(&th->mutex);
  return count;
}


TH_ZoidServer::TH_ZoidServer(unsigned core)
:
  itsCore(core),
  bytesToSend(0)
{
  if (core >= theirTHs.size())
    theirTHs.resize(core + 1);

  theirTHs[core] = this;
  pthread_cond_init(&newSendDataAvailable, 0);
  pthread_cond_init(&newReceiveBufferAvailable, 0);
  pthread_cond_init(&dataSent, 0);
  pthread_cond_init(&dataReceived, 0);
  pthread_mutex_init(&mutex, 0);
}


TH_ZoidServer::~TH_ZoidServer()
{
  pthread_cond_destroy(&newSendDataAvailable);
  pthread_cond_destroy(&newReceiveBufferAvailable);
  pthread_cond_destroy(&dataSent);
  pthread_cond_destroy(&dataReceived);
  pthread_mutex_destroy(&mutex);
}


bool TH_ZoidServer::sendBlocking(void *buf, int nbytes, int, DataHolder *)
{
  pthread_mutex_lock(&mutex);

  sendBufferPtr = (char *) buf;
  bytesToSend = nbytes;
  pthread_cond_signal(&newSendDataAvailable);

  while (bytesToSend > 0)
    pthread_cond_wait(&dataSent, &mutex);

  pthread_mutex_unlock(&mutex);

  return true;
}


bool TH_ZoidServer::recvBlocking(void *buf, int nbytes, int, int, DataHolder *)
{
  pthread_mutex_lock(&mutex);
  receiveBufferPtr = (char *) buf;
  pthread_cond_signal(&newReceiveBufferAvailable);

  for (bytesToReceive = nbytes; bytesToReceive > 0;)
    pthread_cond_wait(&dataReceived, &mutex);

  pthread_mutex_unlock(&mutex);

  return true;
}

}
