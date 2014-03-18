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


Block::Block( size_t nrSubbands, size_t nrSamples, size_t nrChannels )
:
  SampleData<float,3>(boost::extents[nrSamples][nrSubbands][nrChannels], boost::extents[nrSubbands][nrChannels]),
  subbandWritten(nrSubbands, false),
  fileIdx(0),
  block(0),
  nrSamples(nrSamples),
  nrSubbands(nrSubbands),
  nrChannels(nrChannels),
  nrSubbandsLeft(nrSubbands)
{
}


void Block::addSubband( const Subband &subband ) {
  ASSERT(nrSubbandsLeft > 0);

  // Only add subbands that match our ID
  ASSERTSTR(subband.id.fileIdx == fileIdx, "Got fileIdx " << subband.id.fileIdx << ", expected " << fileIdx);
  ASSERTSTR(subband.id.block   == block, "Got block " << subband.id.block << ", expected " << block);

  // Subbands should not arrive twice
  ASSERT(subbandWritten[subband.id.subband] == false);

  // Weave subbands together
  for (size_t t = 0; t < nrSamples; ++t) {
    // Copy all channels for sample t
    memcpy(&samples[t][subband.id.subband][0], &subband.data[t][0], nrChannels * sizeof *subband.data.origin());
  }

  subbandWritten[subband.id.subband] = true;

  nrSubbandsLeft--;

  LOG_DEBUG_STR("Block: added " << subband.id << ", " << nrSubbandsLeft << " subbands left");
}


void Block::zeroRemainingSubbands() {
  for (size_t subbandIdx = 0; subbandIdx < subbandWritten.size(); ++subbandIdx) {
    if (!subbandWritten[subbandIdx]) {
      LOG_INFO_STR("File " << fileIdx << " block " << block << ": zeroing subband " << subbandIdx);

      for (size_t t = 0; t < nrSamples; ++t) {
        // Zero all channels for sample t
        memset(&samples[t][subbandIdx][0], 0, nrChannels * sizeof *samples.origin());
      }
    }
  }
}


bool Block::complete() const {
  return nrSubbandsLeft == 0;
}


void Block::reset( size_t newFileIdx, size_t newBlockIdx ) {
  // Apply annotation, also for the super class
  fileIdx = newFileIdx;
  block = newBlockIdx;

  setSequenceNumber(newBlockIdx);

  // Mark all subbands as not-written
  for (size_t subbandIdx = 0; subbandIdx < subbandWritten.size(); ++subbandIdx) {
    subbandWritten[subbandIdx] = false;
  }
  nrSubbandsLeft = nrSubbands;
}


BlockCollector::BlockCollector( Pool<Block> &outputPool, size_t fileIdx, size_t nrBlocks, size_t maxBlocksInFlight )
:
  outputPool(outputPool),
  fileIdx(fileIdx),
  nrBlocks(nrBlocks),
  maxBlocksInFlight(maxBlocksInFlight),
  canDrop(maxBlocksInFlight > 0),
  lastEmitted(-1)
{
}


void BlockCollector::addSubband( const Subband &subband ) {
  ScopedLock sl(mutex);

  LOG_DEBUG_STR("BlockCollector: Add " << subband.id);

  const size_t &blockIdx = subband.id.block;

  ASSERT(nrBlocks == 0 || blockIdx < nrBlocks);

  if (!have(blockIdx)) {
    if (canDrop) {
      if ((ssize_t)blockIdx <= lastEmitted) {
	      // too late -- discard packet
        LOG_DEBUG_STR("BlockCollector: Dropped subband " << subband.id.subband  << " of file " << subband.id.fileIdx);
	      return;
      }
    } else {
      // if we can't drop, we shouldn't have written
      // this block yet.
      ASSERTSTR((ssize_t)blockIdx > lastEmitted, "Received block " << blockIdx << ", but already emitted up to " << lastEmitted << " for file " << subband.id.fileIdx << " subband " << subband.id.subband);
    }

    // Note: fetch can release the mutex if it has to wait,
    //       causing any assumptions made earlier to be invalid.
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
      outputPool.filled.append(NULL);
    }
  }
}


void BlockCollector::finish() {
  ScopedLock sl(mutex);

  if (!blocks.empty()) {
    emitUpTo(maxBlock());
  }

  if (!canDrop) {
    // Should have received everything
    ASSERT(nrBlocks == 0 || (ssize_t)nrBlocks == lastEmitted + 1);
  }

  // Signal end-of-stream
  outputPool.filled.append(NULL);
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

  LOG_DEBUG_STR("BlockCollector: emitting block " << blockIdx << " of file " << block->fileIdx);

  block->zeroRemainingSubbands();
  
  // emit to outputPool.filled()
  outputPool.filled.append(block);

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
  if (canDrop && blocks.size() + fetching.size() >= maxBlocksInFlight) {
    // No more room -- force out oldest block
    emit(minBlock());
  }

  if (fetching.find(block) != fetching.end()) {
    LOG_DEBUG_STR("BlockCollector: some thread is already fetching block " << block);

    // Wait for OUR block to be fetched
    do {
      fetchSignal.wait(mutex);
    } while(!have(block));

    return;
  }

  LOG_DEBUG_STR("BlockCollector: fetching block " << block);

  SmartPtr<Block> newBlock;

  fetching[block] = true;
  {
    // Allow other threads to manipulate older blocks while we're waiting for
    // a new free one.
    ScopedLock sl(mutex, true);
    newBlock = outputPool.free.remove();
  }
  fetching.erase(block);

  LOG_DEBUG_STR("BlockCollector: fetched block " << block);

  // Add and annotate
  ASSERT(!have(block));
  blocks[block] = newBlock;
  blocks[block]->reset(fileIdx, block);

  // Signal other threads that are waiting for this block
  fetchSignal.broadcast();
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

  return !thread.caughtException();
}


void Receiver::receiveLoop()
{
  try {
    Subband subband;

    for(;;) {
      subband.read(stream);

      const size_t fileIdx = subband.id.fileIdx;

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

      while ((subband = queue->remove()) != NULL) {
        subband->write(stream);
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

