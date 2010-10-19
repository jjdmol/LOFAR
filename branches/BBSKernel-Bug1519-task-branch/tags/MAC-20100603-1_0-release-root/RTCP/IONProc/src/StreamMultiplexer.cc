//# StreamMultiplexer.cc: 
//#
//# Copyright (C) 2010
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: FileDescriptorBasedStream.cc 14333 2009-10-28 08:43:36Z romein $

#include <lofar_config.h>

#include <StreamMultiplexer.h>



namespace LOFAR {
namespace RTCP {


template <typename K, typename V> void StreamMultiplexer::Map<K, V>::insert(K key, V value)
{
  itsMutex.lock();
  itsMap[key] = value;
  itsReevaluate.broadcast();
  itsMutex.unlock();
}


template <typename K, typename V> V StreamMultiplexer::Map<K, V>::remove(K key)
{
  itsMutex.lock();
  std::map<unsigned, Request *>::iterator it;
  
  while ((it = itsMap.find(key)) == itsMap.end())
    itsReevaluate.wait(itsMutex);
  
  V v = it->second;
  itsMap.erase(it);
  itsMutex.unlock();

  return v;
}


StreamMultiplexer::StreamMultiplexer(Stream &stream)
:
  itsStream(stream),
  itsReceiveThread(this, &StreamMultiplexer::receiveThread, 16384)
{
}


StreamMultiplexer::~StreamMultiplexer()
{
  RequestMsg msg;
  msg.type = RequestMsg::STOP_REQ;

  itsSendMutex.lock();
  itsStream.write(&msg, sizeof msg);
  itsSendMutex.unlock();
}


void StreamMultiplexer::registerChannel(MultiplexedStream *stream, unsigned channel)
{
  RequestMsg msg;

  msg.type	= RequestMsg::REGISTER;
  msg.reqPtr	= &stream->itsRequest;
  msg.size	= channel; // FIXME: abuse size field

  itsSendMutex.lock();
  itsStream.write(&msg, sizeof msg);
  itsSendMutex.unlock();

  stream->itsPeerRequestAddr = itsOutstandingRegistrations.remove(channel);
}


void StreamMultiplexer::receiveThread()
{
  while (1) {
    RequestMsg msg;
    itsStream.read(&msg, sizeof msg);

    switch (msg.type) {
      case RequestMsg::RECV_REQ : msg.reqPtr->msg = msg;
				  msg.reqPtr->received.up();
				  break;

      case RequestMsg::RECV_ACK : itsStream.read(msg.recvPtr, msg.size);
				  *msg.sizePtr = msg.size;
				  msg.recvFinished->up();
				  break;

      case RequestMsg::REGISTER : itsOutstandingRegistrations.insert(msg.size, msg.reqPtr);
				  break;

      case RequestMsg::STOP_REQ : return;
    }
  }
}


size_t StreamMultiplexer::tryRead(MultiplexedStream *stream, void *ptr, size_t size)
{
  Semaphore  recvFinished;
  RequestMsg msg;

  msg.type	   = RequestMsg::RECV_REQ;
  msg.size	   = size;
  msg.reqPtr       = stream->itsPeerRequestAddr;
  msg.sizePtr	   = &size;
  msg.recvPtr	   = ptr;
  msg.recvFinished = &recvFinished;

  itsSendMutex.lock();
  itsStream.write(&msg, sizeof msg);
  itsSendMutex.unlock();

  recvFinished.down();

  return size;
}


size_t StreamMultiplexer::tryWrite(MultiplexedStream *stream, const void *ptr, size_t size)
{
  stream->itsRequest.received.down();

  RequestMsg ack = stream->itsRequest.msg;

  ack.type = RequestMsg::RECV_ACK;
  ack.size = std::min(size, ack.size);

  itsSendMutex.lock();
  itsStream.write(&ack, sizeof ack);
  itsStream.write(ptr, ack.size);
  itsSendMutex.unlock();

  return ack.size;
}


MultiplexedStream::MultiplexedStream(StreamMultiplexer &multiplexer, unsigned channel)
:
  itsMultiplexer(multiplexer)
{
  itsMultiplexer.registerChannel(this, channel);
}


MultiplexedStream::~MultiplexedStream()
{
}


size_t MultiplexedStream::tryRead(void *ptr, size_t size)
{
  return itsMultiplexer.tryRead(this, ptr, size);
}


size_t MultiplexedStream::tryWrite(const void *ptr, size_t size)
{
  return itsMultiplexer.tryWrite(this, ptr, size);
}


} // namespace RTCP
} // namespace LOFAR
