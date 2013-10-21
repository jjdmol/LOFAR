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
//# $Id: BlockID.h 26419 2013-09-09 11:19:56Z mol $

#include "TABTranspose.h"

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
  LOG_DEBUG_STR("Written block");
}


void Subband::read(Stream &stream) {
  LOG_DEBUG_STR("Reading block id");
  stream.read(&id, sizeof id);
  LOG_DEBUG_STR("Read block id");

  size_t dim1, dim2;
  stream.read(&dim1, sizeof dim1);
  stream.read(&dim2, sizeof dim2);
  data.resize(boost::extents[dim1][dim2]);
  stream.read(data.origin(), data.num_elements() * sizeof *data.origin());
  LOG_DEBUG_STR("Read block");
}


Block::Block( size_t nrSubbands, size_t nrSamples, size_t nrChannels )
:
  data(boost::extents[nrSubbands][nrSamples][nrChannels]),
  subbandWritten(nrSubbands, false),
  nrSubbandsLeft(nrSubbands)
{
}


void Block::addSubband( const Subband &subband ) {
  ASSERT(nrSubbandsLeft > 0);

  memcpy(data[subband.id.subband].origin(), subband.data.origin(), subband.data.size() * sizeof subband.data.origin());
  subbandWritten[subband.id.subband] = true;

  nrSubbandsLeft--;
}


void Block::zeroRemainingSubbands() {
  for (size_t subbandIdx = 0; subbandIdx < subbandWritten.size(); ++subbandIdx) {
    if (!subbandWritten[subbandIdx]) {
      memset(data[subbandIdx].origin(), 0, data[subbandIdx].size() * sizeof data[subbandIdx].origin());
    }
  }
}


bool Block::complete() const {
  return nrSubbandsLeft == 0;
}


BlockCollector::BlockCollector( Pool<Block> &outputPool, size_t maxBlocksInFlight )
:
  outputPool(outputPool),
  maxBlocksInFlight(maxBlocksInFlight),
  canDrop(maxBlocksInFlight > 0),
  lastEmitted(-1)
{
}


void BlockCollector::addSubband( const Subband &subband ) {
  ScopedLock sl(mutex);

  const size_t &blockIdx = subband.id.block;

  if (!have(blockIdx)) {
    if (canDrop) {
      if ((ssize_t)blockIdx <= lastEmitted) {
	// too late -- discard packet
	return;
      }
    } else {
      // if we can't drop, we shouldn't have written
      // this block yet.
      ASSERT((ssize_t)blockIdx > lastEmitted);
    }

    fetch(blockIdx);
  }

  // augment existing block
  SmartPtr<Block> &block = blocks.at(blockIdx);

  block->addSubband(subband);

  if (block->complete()) {
    // Block is complete -- send it downstream,
    // and everything before it. We know we won't receive
    // data from earlier blocks, because all subbands
    // are sent in-order.
    emitUpTo(blockIdx);
  }
}


void BlockCollector::finish() {
  ScopedLock sl(mutex);

  if (!blocks.empty()) {
    emitUpTo(maxBlock());
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

  if (canDrop && blocks.size() >= maxBlocksInFlight) {
    // No more room -- force out oldest block
    emit(minBlock());
  }

  blocks[block] = outputPool.free.remove();
}


Sender::Sender( Stream &stream, size_t queueSize, bool canDrop )
:
  stream(stream),
  queue(queueSize, canDrop),
  thread(this, &Sender::sendLoop)
{
}


bool Sender::finish()
{
  queue.noMore();

  thread.wait();

  return !thread.caughtException();
}


void Sender::append( SmartPtr<Subband> subband )
{
  queue.append(subband);
}


void Sender::sendLoop()
{
  SmartPtr<Subband> subband;

  while( (subband = queue.remove()) != NULL) {
    subband->write(stream);
  }
}


Receiver::Receiver( Stream &stream, CollectorMap &collectors )
:
  stream(stream),
  collectors(collectors),
  thread(this, &Receiver::receiveLoop)
{
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

      ASSERTSTR(collectors.find(fileIdx) != collectors.end(), "Received a piece of TAB " << fileIdx << ", which is unknown to me");

      LOG_DEBUG_STR("TAB " << fileIdx << ": Adding subband " << subband.id.subband);

      collectors.at(fileIdx)->addSubband(subband);
    }
  } catch (Stream::EndOfStreamException &) {
  }
}

} // namespace TABTranspose
} // namespace Cobalt
} // namespace LOFAR

