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

#include <CoInterface/TimeFuncs.h>
#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <boost/format.hpp>
#include <algorithm>

using namespace std;
using boost::format;

namespace LOFAR {
namespace Cobalt {
namespace TABTranspose {

// A Subband here is the part of a Block (see below) that contains the data
// values in a subband during a block duration (e.g. 1 second).
Subband::Subband( size_t nrSamples, size_t nrChannels )
: 
  data(boost::extents[nrSamples][nrChannels])
{
  id.fileIdx = 0;
  id.subband = 0;
  id.block   = 0;
}


// Used by a MultiSender in rtcp to write a Subband through a Stream to a
// Receiver in outputProc. 
void Subband::write(Stream &stream) const {
  stream.write(&id, sizeof id);

  size_t dim1 = data.shape()[0];
  size_t dim2 = data.shape()[1];
  stream.write(&dim1, sizeof dim1);
  stream.write(&dim2, sizeof dim2);
  stream.write(data.origin(), data.num_elements() * sizeof *data.origin());

  LOG_DEBUG_STR("Written block "  << id);
}


// Used by a Receiver in outputProc to read a Subband through a Stream from a
// MultiSender in rtcp.
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


// A Block here is a block duration (e.g. 1 sec) of data with all the Subbands
// that should go info a single file (part).
Block::Block( size_t fileIdx, size_t blockIdx, size_t nrSubbands, size_t nrSamples, size_t nrChannels )
:
  fileIdx(fileIdx),
  blockIdx(blockIdx),
  nrSamples(nrSamples),
  nrSubbands(nrSubbands),
  nrChannels(nrChannels),
  subbandCache(nrSubbands, NULL),
  nrSubbandsLeft(nrSubbands)
{
}


// Used by the BlockCollector to add a received Subband to a Block.
// Runs in outputProc.
void Block::addSubband( SmartPtr<Subband> &subband ) {
  ASSERT(nrSubbandsLeft > 0);

  const Subband::BlockID id = subband->id;

  // Only add subbands that match our ID
  ASSERTSTR(id.fileIdx == fileIdx,  "Got fileIdx " << id.fileIdx << ", expected " << fileIdx);
  ASSERTSTR(id.subband < nrSubbands, "Got subband " << id.subband << ", expected < " << nrSubbands);
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

  /*
   * Input:
   *   the set of subbandCache[nrSubbands],
   *   each of which is either NULL, 
   *   or (indirectly) a pointer to [nrSamples][nrChannels]
   * Output:
   *   output.samples[nrSamples][nrSubbands][nrChannels]
   *
   * Which means we're transposing nrChannels * sizeof(float) bytes at a time,
   * which is especially painful if nrChannels == 1.
   *
   * The implemented strategy coalesces the writes in batches. Large batches
   * will cause TLB misses due to the larger number of arrays we need to index.
   */
  const size_t MAXBATCHSIZE = 4;

  // Stride between samples in output
  const ptrdiff_t dst_sample_stride = output.samples.strides()[0];
  // Stride between subbands in output
  const ptrdiff_t dst_sb_stride = output.samples.strides()[1];

  const vector<float> zeroes(nrChannels * nrSamples, 0.0f);

  // Four threads gives the best performance on CEP2, see figures given by 'tTABTranspose | grep write'
# pragma omp parallel for num_threads(4)
  for (size_t subbandBase = 0; subbandBase < subbandCache.size(); subbandBase += MAXBATCHSIZE) {
    // Determine actual batch size
    const size_t BATCHSIZE = std::min(MAXBATCHSIZE, subbandCache.size() - subbandBase);

    // Collect source pointers for all our subbands, or to our zeroes array if
    // a subband is not available. Each element points to an array of dimensions
    //   [nrSamples][nrChannels]
    const float *src[MAXBATCHSIZE];

    for (size_t i = 0; i < BATCHSIZE; ++i) {
      size_t subbandIdx = subbandBase + i;

      src[i] = subbandCache[subbandIdx] ? subbandCache[subbandIdx]->data.origin() : &zeroes[0];
    }

    // Pointer to walk over all samples
    float *dst = &output.samples[0][subbandBase][0];

    if (nrChannels == 1) {
      /* Use assignment to copy data */
      for (size_t t = 0; t < nrSamples; ++t) {
        float *sample = dst;
        dst += dst_sample_stride;

        for (size_t i = 0; i < BATCHSIZE; ++i) {
          *sample = src[i][t];

          sample += dst_sb_stride;
        }
      }
    } else {
      /* Use memcpy to copy data */
      for (size_t t = 0; t < nrSamples; ++t) {
        float *sample = dst;
        dst += dst_sample_stride;

        for (size_t i = 0; i < BATCHSIZE; ++i) {
          memcpy(sample, &src[i][t * nrChannels], nrChannels * sizeof(float));

          sample += dst_sb_stride;
        }
      }
    }
  }

  // Report summary
  size_t nrLost = std::count(subbandCache.begin(), subbandCache.end(), (Subband*)NULL);

  LOG_INFO_STR("Block: written " << (nrSubbands - nrLost) << " subbands, lost " << nrLost << " subbands.");
}


bool Block::complete() const {
  return nrSubbandsLeft == 0;
}


// The BlockCollector collects blocks from different rtcp processes for a TAB.
// More precisely, we have one BlockCollector per file (i.e. part).
BlockCollector::BlockCollector( Pool<BeamformedData> &outputPool, size_t fileIdx, size_t nrSubbands, size_t nrChannels, size_t nrSamples, size_t nrBlocks, size_t maxBlocksInFlight )
:
  inputQueue(str(format("BlockCollector::inputQueue [file %u]") % fileIdx), (1 + maxBlocksInFlight) * nrSubbands, false), // drop = false: we drop at the output, not at the input, but we do want to protect against unbounded growth
  outputQueue(str(format("BlockCollector::outputQueue [file %u]") % fileIdx)),
  outputPool(outputPool),

  fileIdx(fileIdx),
  nrBlocks(nrBlocks),
  nrSubbands(nrSubbands),
  nrChannels(nrChannels),
  nrSamples(nrSamples),

  maxBlocksInFlight(maxBlocksInFlight),
  canDrop(maxBlocksInFlight > 0),
  lastEmitted(-1),

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
  outputQueue.append(NULL, false);
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

  NSTimer writeTimer("Block: data transpose/zeroing", true, true);

  while ((block = outputQueue.remove()) != NULL) {
    SmartPtr<BeamformedData> output = outputPool.free.remove();

    writeTimer.start();
    block->write(*output);
    writeTimer.stop();

    outputPool.filled.append(output);
  }

  outputPool.filled.append(NULL);
}


// Used by a Receiver inputThread to add a Subband to a Block into a BlockCollector.
// If this completes a Block (all Subbands for this Block received) and no
// subsequent Blocks are missing something, send it (or them) off into the
// outputQueue for write-back to storage.
void BlockCollector::_addSubband( SmartPtr<Subband> &subband ) {
  LOG_DEBUG_STR("BlockCollector: Add " << subband->id);

  const size_t &blockIdx = subband->id.block;

  ASSERT(nrBlocks == 0 || blockIdx < nrBlocks);

  if (!have(blockIdx)) {
    if (!fetch(blockIdx)) {
      // too late -- discard packet
      LOG_DEBUG_STR("BlockCollector: Dropped subband " << subband->id.subband  << " of file " << subband->id.fileIdx);

      // if we can't drop, we shouldn't have written
      // this block yet.
      ASSERTSTR(!canDrop, "Received block " << blockIdx << ", but already emitted up to " << lastEmitted << " for file " << subband->id.fileIdx << " subband " << subband->id.subband);

      return;
    }
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
  outputQueue.append(NULL, false);
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


// Send the Block with blockIdx for write-back to storage now.
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


bool BlockCollector::fetch(size_t block) {
  if ((ssize_t)block <= lastEmitted)
    // too late -- discard packet
    return false;

  // Make sure we don't exceed our maximum cache size
  if (canDrop && blocks.size() >= maxBlocksInFlight) {
    // No more room -- force out oldest block
    emit(minBlock());

    // We emitted blocks, which can have a higher number than the
    // block we're asked to fetch. This happens under severe data loss,
    // when we get disjunct sets of subbands for each block. In that case,
    // the blocks can arrive out-of-order.
    if ((ssize_t)block <= lastEmitted)
      return false;
  }

  // Add and annotate
  ASSERT(!have(block));
  blocks[block] = new Block(fileIdx, block, nrSubbands, nrSamples, nrChannels);

  return true;
}


// Receives data from an rtcp process. Runs in outputProc.
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


// Listen for and receive connections and data in an outputProc process
// from MultiSender objects at different rtcp processes.
MultiReceiver::MultiReceiver( const std::string &servicePrefix, Receiver::CollectorMap &collectors )
:
  servicePrefix(servicePrefix),
  collectors(collectors),
  thread(this, &MultiReceiver::listenLoop)
{
}


// cancel listeners
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


// Listen for incoming connects on outputProc from rtcp.
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


// Hook up a new Receiver (connected to rtcp) to block collectors,
// both in outputProc.
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


// Maintains the connections of an rtcp process with all its outputProc processes
// it needs to send data to.
MultiSender::MultiSender( const HostMap &hostMap, bool canDrop, double maxRetentionTime )
:
  hostMap(hostMap),
  canDrop(canDrop),
  maxRetentionTime(maxRetentionTime)
{
  for (HostMap::const_iterator i = hostMap.begin(); i != hostMap.end(); ++i) {
    // keep a list of unique hosts
    if(find(hosts.begin(), hosts.end(), i->second) == hosts.end())
      hosts.push_back(i->second);

    // each file gets a drop_rate counter
    drop_rates[i->first] = RunningStatistics("%");
  }

  for (vector<struct Host>::const_iterator i = hosts.begin(); i != hosts.end(); ++i) {
    queues[*i] = new Queue< SmartPtr<struct Subband> >(str(format("MultiSender::queue [to %s]") % i->hostName));
  }
}


MultiSender::~MultiSender()
{
  LOG_INFO_STR("MultiSender: canDrop = " << canDrop << ", maxRetentionTime = " << maxRetentionTime);
  for (HostMap::const_iterator i = hostMap.begin(); i != hostMap.end(); ++i) {
    LOG_INFO_STR("MultiSender: [file " << i->first << " to " << i->second.hostName << "] Dropped " << drop_rates.at(i->first).mean() << "% of the data");
  }
}


// Sets up the connections from an rtcp process to its outputProc processes.
// Then, keep writing blocks until we see a NULL Block ptr.
void MultiSender::process( OMPThreadSet *threadSet )
{
  // We need to register our threads somewhere...
  OMPThreadSet dummySet;

  if (!threadSet)
    threadSet = &dummySet;

#pragma omp parallel for num_threads(hosts.size())
  for (int i = 0; i < (ssize_t)hosts.size(); ++i) {
    try {
      const struct Host &host = hosts[i];

      OMPThreadSet::ScopedRun sr(*threadSet);

      LOG_DEBUG_STR("MultiSender: Connecting to " << host.hostName << ":" << host.brokerPort << ":" << host.service);

      PortBroker::ClientStream stream(host.hostName, host.brokerPort, host.service);

      LOG_DEBUG_STR("MultiSender->" << host.hostName << ": connected");

      SmartPtr< Queue< SmartPtr<struct Subband> > > &queue = queues.at(host);

      LOG_DEBUG_STR("MultiSender->" << host.hostName << ": processing queue");

      SmartPtr<struct Subband> subband;
      NSTimer sendTimer(str(format("Send Subband to %s") % host.hostName), true, true);

      while ((subband = queue->remove()) != NULL) {
        NSTimer::StartStop ss(sendTimer);

        // Note: this can take any amount of time, even hours, even in real-time mode
        subband->write(stream);
      }

      LOG_DEBUG_STR("MultiSender->" << host.hostName << ": done");
    } catch (Exception &ex) {
      LOG_ERROR_STR("Caught exception: " << ex);
    }
  }
}


// The pipeline calls here to write a block for a single file (part).
void MultiSender::append( SmartPtr<struct Subband> &subband )
{
  using namespace TimeSpec;

  // Find the host to send these data to
  const size_t fileIdx = subband->id.fileIdx;
  const struct Host &host = hostMap.at(fileIdx);

  SmartPtr< Queue< SmartPtr<struct Subband> > > &queue = queues.at(host);

  // If oldest packet in queue is too old, drop it in lieu of this new one
  if (canDrop && TimeSpec::now() - queue->oldest() > maxRetentionTime) {
    drop_rates.at(fileIdx).push(100.0);

    // remove oldest item
    SmartPtr<struct Subband> subband = queue->remove();

    // would be weird to have NULL in here while we're appending elements
    ASSERT(subband);
  } else {
    drop_rates.at(fileIdx).push(0.0);
  }

  // Append the data to the respective queue
  queue->append(subband);
}


// The pipeline indicates that it has appended its last block.
// Further appending is denied and a final NULL Block ptr is queued.
void MultiSender::finish()
{
  for (vector<struct Host>::const_iterator i = hosts.begin(); i != hosts.end(); ++i) {
    queues.at(*i)->append(NULL, false);
  }
}

} // namespace TABTranspose
} // namespace Cobalt
} // namespace LOFAR

