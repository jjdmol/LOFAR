//# TABTranspose.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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
//# $Id$

#include <lofar_config.h>
#include "TABTranspose.h"

#include <Common/LofarLogger.h>
#include <boost/format.hpp>
#include <algorithm>

using namespace std;
using boost::format;

namespace LOFAR {
namespace Cobalt {
namespace TABTranspose {

Subband::Subband( size_t nrSamples, size_t nrChannels )
: 
  data(boost::extents[nrSamples][nrChannels])
{
  id.fileIdx = 0;
  id.subband = 0;
  id.block   = 0;
}


void Subband::write(Stream &stream) const {
  stream.write(&id, sizeof id);

  size_t dim1 = data.shape()[0];
  size_t dim2 = data.shape()[1];
  stream.write(&dim1, sizeof dim1);
  stream.write(&dim2, sizeof dim2);
  stream.write(data.origin(), data.num_elements() * sizeof *data.origin());

  LOG_DEBUG_STR("Written block "  << id);
}


void Subband::read(Stream &stream) {
  LOG_DEBUG_STR("Reading block id");
  stream.read(&id, sizeof id);
  LOG_DEBUG_STR("Read block id " << id);

  size_t dim1, dim2;
  stream.read(&dim1, sizeof dim1);
  stream.read(&dim2, sizeof dim2);
  data.resize(boost::extents[dim1][dim2]);
  stream.read(data.origin(), data.num_elements() * sizeof *data.origin());
  LOG_DEBUG_STR("Read block " << id);
}


std::ostream &operator<<(std::ostream &str, const Subband::BlockID &id)
{
  return str << "file " << id.fileIdx << " block " << id.block << " subband " << id.subband;
}


Block::Block( size_t fileIdx, size_t blockIdx, size_t nrSubbands, size_t nrSamples, size_t nrChannels )
:
  fileIdx(fileIdx),
  blockIdx(blockIdx),
  nrSamples(nrSamples),
  nrSubbands(nrSubbands),
  nrChannels(nrChannels),
  subbandCache(nrSubbands, NULL),
  nrSubbandsLeft(nrSubbands),
  writeTimer("Block: data transpose/zeroing", true, true)
{
}


void Block::addSubband( SmartPtr<Subband> &subband ) {
  ASSERT(nrSubbandsLeft > 0);

  const Subband::BlockID id = subband->id;

  // Only add subbands that match our ID
  ASSERTSTR(id.fileIdx == fileIdx,  "Got fileIdx " << id.fileIdx << ", expected " << fileIdx);
  ASSERTSTR(id.block   == blockIdx, "Got block " << id.block << ", expected " << blockIdx);

  ASSERT(id.subband < nrSubbands);
  ASSERT(subband->data.shape()[0] == nrSamples);
  ASSERT(subband->data.shape()[1] == nrChannels);

  // Subbands should not arrive twice
  ASSERT(subbandCache[id.subband] == NULL);

  subbandCache[id.subband] = subband;
  ASSERT(subband == NULL);

  nrSubbandsLeft--;

  LOG_DEBUG_STR("Block: adding " << id << ", " << nrSubbandsLeft << " subbands left");
}


void Block::write( BeamformedData &output ) {
  // Check dimensions
  ASSERT( output.samples.shape()[0] == nrSamples );
  ASSERT( output.samples.shape()[1] == nrSubbands );
  ASSERT( output.samples.shape()[2] == nrChannels );

  // Set annotation
  output.setSequenceNumber(blockIdx);

  // Set data
  writeTimer.start();
  for (size_t subbandIdx = 0; subbandIdx < subbandCache.size(); ++subbandIdx) {
    float *dst = &output.samples[0][subbandIdx][0];
    const ptrdiff_t dst_stride = &output.samples[1][0][0] - &output.samples[0][0][0];

    if (subbandCache[subbandIdx] != NULL) {
      // Transpose subband
      Subband &subband = *subbandCache[subbandIdx];

      const float *src = &subband.data[0][0];
      const ptrdiff_t src_stride = &subband.data[1][0] - &subband.data[0][0];

      if (src_stride == 1) {
        for (size_t t = 0; t < nrSamples; ++t) {
          *dst = *src;
          src ++;
          dst += dst_stride;
        }
      } else {
        for (size_t t = 0; t < nrSamples; ++t) {
          memcpy(dst, src, src_stride * sizeof *src);
          src += src_stride;
          dst += dst_stride;
        }
      }
    } else {
      // Write zeroes
      if (nrChannels == 1) {
        for (size_t t = 0; t < nrSamples; ++t) {
          *dst = 0.0;
          dst += dst_stride;
        }
      } else {
        for (size_t t = 0; t < nrSamples; ++t) {
          memset(dst, 0, nrChannels * sizeof *dst);
          dst += dst_stride;
        }
      }
    }
  }
  writeTimer.stop();

  // Report summary
  size_t nrLost = std::count(subbandCache.begin(), subbandCache.end(), (Subband*)NULL);

  LOG_INFO_STR("Block: written " << (nrSubbands - nrLost) << " subbands, lost " << nrLost << " subbands.");
}


bool Block::complete() const {
  return nrSubbandsLeft == 0;
}


BlockCollector::BlockCollector( Pool<BeamformedData> &outputPool, size_t fileIdx, size_t nrSubbands, size_t nrChannels, size_t nrSamples, size_t nrBlocks, size_t maxBlocksInFlight )
:
  inputQueue((1 + maxBlocksInFlight) * nrSubbands, false), // drop = false: we drop at the output, not at the input
  outputPool(outputPool),

  fileIdx(fileIdx),
  nrBlocks(nrBlocks),
  nrSubbands(nrSubbands),
  nrChannels(nrChannels),
  nrSamples(nrSamples),

  maxBlocksInFlight(maxBlocksInFlight),
  canDrop(maxBlocksInFlight > 0),
  lastEmitted(-1),

  addSubbandTimer("BlockCollector::addSubband", true, true),
  fetchTimer("BlockCollector: fetch new block", true, true),

  inputThread(this, &BlockCollector::inputLoop),
  outputThread(this, &BlockCollector::outputLoop)
{
  ASSERT(nrSubbands > 0);
  ASSERT(nrChannels > 0);
  ASSERT(nrSamples > 0);
}


BlockCollector::~BlockCollector()
{
  // Make SURE the threads can finish, regardless of whether finish() was called
  inputQueue.noMore();
  outputQueue.append(NULL);
}


void BlockCollector::addSubband( SmartPtr<Subband> &subband ) {
  inputQueue.append(subband);
}


void BlockCollector::inputLoop() {
  SmartPtr<Subband> subband;

  while ((subband = inputQueue.remove()) != NULL) {
    _addSubband(subband);
  }
}


void BlockCollector::outputLoop() {
  SmartPtr<Block> block;

  while ((block = outputQueue.remove()) != NULL) {
    SmartPtr<BeamformedData> output = outputPool.free.remove();

    block->write(*output);

    outputPool.filled.append(output);
  }

  outputPool.filled.append(NULL);
}


void BlockCollector::_addSubband( SmartPtr<Subband> &subband ) {
  NSTimer::StartStop ss(addSubbandTimer);

  LOG_DEBUG_STR("BlockCollector: Add " << subband->id);

  const size_t &blockIdx = subband->id.block;

  ASSERT(nrBlocks == 0 || blockIdx < nrBlocks);

  if (!have(blockIdx)) {
    if (canDrop) {
      if ((ssize_t)blockIdx <= lastEmitted) {
	      // too late -- discard packet
        LOG_DEBUG_STR("BlockCollector: Dropped subband " << subband->id.subband  << " of file " << subband->id.fileIdx);
	      return;
      }
    } else {
      // if we can't drop, we shouldn't have written
      // this block yet.
      ASSERTSTR((ssize_t)blockIdx > lastEmitted, "Received block " << blockIdx << ", but already emitted up to " << lastEmitted << " for file " << subband->id.fileIdx << " subband " << subband->id.subband);
    }

    fetch(blockIdx);
  }

  SmartPtr<Block> &block = blocks.at(blockIdx);

  block->addSubband(subband);

  if (block->complete()) {
    // Block is complete -- send it downstream,
    // and everything before it. We know we won't receive
    // data from earlier blocks, because all subbands
    // are sent in-order.
    emitUpTo(blockIdx);

    if (nrBlocks > 0 && blockIdx == nrBlocks - 1) {
      // Received last block -- wrap up
      LOG_INFO_STR("BlockCollector: Received last block of file " << fileIdx);

      ASSERT(blocks.empty());

      // Signal end-of-stream
      outputQueue.append(NULL);
    }
  }
}


void BlockCollector::finish() {
  // Wait for all input to be processed
  inputQueue.noMore();
  inputThread.wait();

  // Wrap-up remainder
  if (!blocks.empty()) {
    emitUpTo(maxBlock());
  }

  if (!canDrop) {
    // Should have received everything
    ASSERT(nrBlocks == 0 || (ssize_t)nrBlocks == lastEmitted + 1);
  }

  // Signal end-of-stream
  outputQueue.append(NULL);
  outputThread.wait();
}


size_t BlockCollector::minBlock() const {
  ASSERT(!blocks.empty());

  return blocks.begin()->first;
}

size_t BlockCollector::maxBlock() const {
  ASSERT(!blocks.empty());

  return blocks.rbegin()->first;
}


void BlockCollector::emit(size_t blockIdx) {
  // should emit in-order
  if (!canDrop) {
    ASSERT((ssize_t)blockIdx == lastEmitted + 1);
  } else {
    ASSERT((ssize_t)blockIdx > lastEmitted);
  }
  lastEmitted = blockIdx;

  // clear data we didn't receive
  SmartPtr<Block> &block = blocks.at(blockIdx);

  LOG_DEBUG_STR("BlockCollector: emitting block " << blockIdx << " of file " << fileIdx);
  
  // emit to outputPool.filled()
  outputQueue.append(block);

  // remove from our administration
  blocks.erase(blockIdx);
}


void BlockCollector::emitUpTo(size_t block) {
  while (!blocks.empty() && minBlock() <= block) {
    emit(minBlock());
  }
}


bool BlockCollector::have(size_t block) const {
  return blocks.find(block) != blocks.end();
}


void BlockCollector::fetch(size_t block) {
  ASSERT(!have(block));

  // Make sure we don't exceed our maximum cache size
  if (canDrop && blocks.size() >= maxBlocksInFlight) {
    // No more room -- force out oldest block
    emit(minBlock());
  }

  // Add and annotate
  ASSERT(!have(block));
  blocks[block] = new Block(fileIdx, block, nrSubbands, nrSamples, nrChannels);
}


Receiver::Receiver( Stream &stream, CollectorMap &collectors )
:
  stream(stream),
  collectors(collectors),
  thread(this, &Receiver::receiveLoop)
{
}


Receiver::~Receiver()
{
  kill();
}


void Receiver::kill()
{
  thread.cancel();
}


bool Receiver::finish()
{
  thread.wait();

  for (CollectorMap::iterator i = collectors.begin(); i != collectors.end(); ++i)
    i->second->finish();

  return !thread.caughtException();
}


void Receiver::receiveLoop()
{
  try {
    for(;;) {
      SmartPtr<Subband> subband = new Subband;

      subband->read(stream);

      const size_t fileIdx = subband->id.fileIdx;

      ASSERTSTR(collectors.find(fileIdx) != collectors.end(), "Received a piece of file " << fileIdx << ", which is unknown to me");

      //LOG_DEBUG_STR("File " << fileIdx << ": Adding subband " << subband.id);

      collectors.at(fileIdx)->addSubband(subband);
    }
  } catch (Stream::EndOfStreamException &) {
  }
}


MultiReceiver::MultiReceiver( const std::string &servicePrefix, Receiver::CollectorMap &collectors )
:
  servicePrefix(servicePrefix),
  collectors(collectors),
  thread(this, &MultiReceiver::listenLoop)
{
}


MultiReceiver::~MultiReceiver()
{
  kill(0);
}


void MultiReceiver::kill(size_t minNrClients)
{
  {
    ScopedLock sl(mutex);

    while(clients.size() < minNrClients) {
      LOG_DEBUG_STR("MultiReceiver::kill: " << clients.size() << " clients connected, waiting for " << minNrClients);
      newClient.wait(mutex);
    }

  }

  // Kill listenLoop, but keep clients alive
  thread.cancel();

  // Wait for thread to die so we can access `clients'
  // safely.
  thread.wait();

  // Kill all client connections
  for (size_t i = 0; i < clients.size(); ++i) {
    if (minNrClients == 0) {
      clients[i].receiver->kill();
    } else {
      clients[i].receiver->finish();
    }
  }
}


void MultiReceiver::listenLoop()
{
  for(;;) {
    // Accept a new client
    SmartPtr<PortBroker::ServerStream> stream;
   
    try {
      stream = new PortBroker::ServerStream(servicePrefix, true);
    } catch(SocketStream::TimeOutException &) {
      // fail silently if no client connected
      LOG_DEBUG_STR("TABTranspose::MultiReceiver: Timed out");
      break;
    }

    LOG_DEBUG_STR("TABTranspose::MultiReceiver: Accepted resource " << stream->getResource());

    // Dispatch a Receiver for the client
    dispatch(stream.release());
  }
}


void MultiReceiver::dispatch( PortBroker::ServerStream *stream )
{
  ScopedLock sl(mutex);

  struct Client client;

  client.stream = stream;
  client.receiver = new Receiver(*stream, collectors);

  clients.push_back(client);

  // Inform of change in collectors
  newClient.signal();

  LOG_DEBUG_STR("TABTranspose::MultiReceiver: Dispatched client for resource " << stream->getResource());
}


MultiSender::MultiSender( const HostMap &hostMap, size_t queueSize, bool canDrop )
:
  hostMap(hostMap)
{
  for (HostMap::const_iterator i = hostMap.begin(); i != hostMap.end(); ++i) {
    if(find(hosts.begin(), hosts.end(), i->second) == hosts.end())
      hosts.push_back(i->second);
  }

  for (vector<struct Host>::const_iterator i = hosts.begin(); i != hosts.end(); ++i) {
    queues[*i] = new BestEffortQueue< SmartPtr<struct Subband> >(queueSize, canDrop);
  }
}


void MultiSender::process()
{
#pragma omp parallel for num_threads(hosts.size())
  for (int i = 0; i < (ssize_t)hosts.size(); ++i) {
    try {
      const struct Host &host = hosts[i];

      LOG_DEBUG_STR("MultiSender: Connecting to " << host.hostName << ":" << host.brokerPort << ":" << host.service);

      PortBroker::ClientStream stream(host.hostName, host.brokerPort, host.service);

      LOG_DEBUG_STR("MultiSender->" << host.hostName << ": connected");

      SmartPtr< BestEffortQueue< SmartPtr<struct Subband> > > &queue = queues.at(host);

      LOG_DEBUG_STR("MultiSender->" << host.hostName << ": processing queue");

      SmartPtr<struct Subband> subband;
      NSTimer sendTimer(str(format("Send Subband to %s") % host.hostName), true, true);

      while ((subband = queue->remove()) != NULL) {
        sendTimer.start();
        subband->write(stream);
        sendTimer.stop();
      }

      LOG_DEBUG_STR("MultiSender->" << host.hostName << ": done");
    } catch (Exception &ex) {
      LOG_ERROR_STR("Caught exception: " << ex);
    }
  }
}


void MultiSender::append( SmartPtr<struct Subband> &subband )
{
  // Find the host to send these data to
  const struct Host &host = hostMap.at(subband->id.fileIdx);

  // Append the data to the respective queue
  queues.at(host)->append(subband);
}


void MultiSender::finish()
{
  for (vector<struct Host>::const_iterator i = hosts.begin(); i != hosts.end(); ++i) {
    queues.at(*i)->noMore();
  }
}

} // namespace TABTranspose
} // namespace Cobalt
} // namespace LOFAR

