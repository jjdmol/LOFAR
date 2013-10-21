//# TABTranspose.h
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

#ifndef LOFAR_COINTERFACE_TABTRANSPOSE_H
#define LOFAR_COINTERFACE_TABTRANSPOSE_H

#include <iostream>
#include <map>
#include <cstring>
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Thread.h>
#include "BestEffortQueue.h"
#include "MultiDimArray.h"
#include "SmartPtr.h"
#include "Pool.h"

namespace LOFAR
{
  namespace Cobalt
  {
    namespace TABTranspose
    {
      /*
       * A piece of data belonging to a certain subband.
       *
       * The constructor fixes the size of the data, but
       * other fields are left to the caller to fill.
       */

      struct Subband {
        MultiDimArray<float, 2> data; // [samples][channels]

        Subband( size_t nrSamples = 0, size_t nrChannels = 0 )
        :
          data(boost::extents[nrSamples][nrChannels])
        {
          id.fileIdx = 0;
          id.subband = 0;
          id.block   = 0;
        }

        struct {
          size_t fileIdx;
          size_t subband;
          size_t block;
        } id;

        void write(Stream &stream) const {
          stream.write(&id, sizeof id);

          size_t dim1 = data.shape()[0];
          size_t dim2 = data.shape()[1];
          stream.write(&dim1, sizeof dim1);
          stream.write(&dim2, sizeof dim2);
          stream.write(data.origin(), data.num_elements() * sizeof *data.origin());
          LOG_DEBUG_STR("Written block");
        }

        void read(Stream &stream) {
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
      };

      /*
       * A block of data, representing for one time slice all
       * subbands.
       *
       * The constructor fixes the size of the data and the number
       * of subbands per block, but other fields are left to the
       * caller to fill.
       */
      class Block {
      public:
        MultiDimArray<float, 3> data; // [subband][samples][channels]
        std::vector<bool> subbandWritten;

        size_t fileIdx;
        size_t block;

        Block( size_t nrSubbands, size_t nrSamples, size_t nrChannels )
        :
          data(boost::extents[nrSubbands][nrSamples][nrChannels]),
          subbandWritten(nrSubbands, false),
          nrSubbandsLeft(nrSubbands)
        {
        }

        /*
         * Add data for a single subband.
         */
        void addSubband( const Subband &subband ) {
          ASSERT(nrSubbandsLeft > 0);

          memcpy(data[subband.id.subband].origin(), subband.data.origin(), subband.data.size() * sizeof subband.data.origin());
          subbandWritten[subband.id.subband] = true;

          nrSubbandsLeft--;
        }

        /*
         * Add data for a single subband.
         */
        void zeroRemainingSubbands() {
          for (size_t subbandIdx = 0; subbandIdx < subbandWritten.size(); ++subbandIdx) {
            if (!subbandWritten[subbandIdx]) {
              memset(data[subbandIdx].origin(), 0, data[subbandIdx].size() * sizeof data[subbandIdx].origin());
            }
          }
        }

        /*
         * Return whether the block is complete, that is, all
         * subbands have been added.
         */
        bool complete() const {
          return nrSubbandsLeft == 0;
        }

      private:
        // The number of subbands left to receive.
        size_t nrSubbandsLeft;
      };

      /*
       * BlockCollector fills Block objects out of individual Subband
       * objects. Fresh blocks are drawn from outputPool.free, and collected
       * blocks are stored in outputPool.filled.
       *
       * The BlockCollector can have several blocks under construction (but
       * no more than maxBlocksInFlight, if >0). Block `b' is emitted to
       * outputPool.filled in ANY of the following cases:
       *
       *   a. all subbands for block 'b' have been received
       *   b. a block younger than 'b' is emitted, which means that
       *      subbands have to arrive in-order to prevent data loss
       *   c. if maxBlocksInFlight > 0, and ALL of the following holds:
       *      - there are maxBlocksInFlight blocks under construction
       *      - a new block is required to store new subbands
       *      - block 'b' is the oldest block
       *   d. finish() is called, which flushes all blocks
       *
       * The finish() call ends by placing a NULL marker in outputPool.filled
       * to indicate the end-of-stream.
       *
       * This class is thread safe.
       */
      class BlockCollector {
      public:
        BlockCollector( Pool<Block> &outputPool, size_t maxBlocksInFlight = 0 )
        :
          outputPool(outputPool),
          maxBlocksInFlight(maxBlocksInFlight),
          canDrop(maxBlocksInFlight > 0),
          lastEmitted(-1)
        {
        }

        void addSubband( const Subband &subband ) {
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

            create(blockIdx);
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

        /*
         * Send all remaining blocks downstream,
         * followed by the end-of-stream marker.
         */
        void finish() {
          ScopedLock sl(mutex);

          if (!blocks.empty()) {
            emitUpTo(maxBlock());
          }

          // Signal end-of-stream
          outputPool.filled.append(NULL);
        }

      private:
        std::map<size_t, SmartPtr<Block> > blocks;
        Pool<Block> &outputPool;
        Mutex mutex;

        // upper limit for blocks.size(), or 0 if unlimited
        const size_t maxBlocksInFlight;

        // whether we are allowed to drop data
        const bool canDrop;
        
        // nr of last emitted block, or -1 if no block has been emitted
        ssize_t lastEmitted;

        size_t minBlock() const {
          ASSERT(!blocks.empty());

          return blocks.begin()->first;
        }

        size_t maxBlock() const {
          ASSERT(!blocks.empty());

          return blocks.rbegin()->first;
        }

        /*
         * Send a certain block downstream.
         */
        void emit(size_t blockIdx) {
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

        /*
         * Send all blocks downstream up to
         * and including `block', in-order, as the
         * writer will expect.
         */
        void emitUpTo(size_t block) {
          while (!blocks.empty() && minBlock() <= block) {
            emit(minBlock());
          }
        }

        /*
         * Do we manage a certain block?
         */
        bool have(size_t block) const {
          return blocks.find(block) != blocks.end();
        }

        /*
         * Fetch a new block.
         */
        void create(size_t block) {
          ASSERT(!have(block));

          if (canDrop && blocks.size() >= maxBlocksInFlight) {
            // No more room -- force out oldest block
            emit(minBlock());
          }

          blocks[block] = outputPool.free.remove();
        }
      };

      class Sender {
      public:
        Sender( Stream &stream, size_t queueSize = 3, bool canDrop = false )
        :
          stream(stream),
          queue(queueSize, canDrop),
          thread(this, &Sender::sendLoop)
        {
        }


        bool finish()
        {
          queue.noMore();

          thread.wait();

          return !thread.caughtException();
        }


        void append( SmartPtr<Subband> subband )
        {
          queue.append(subband);
        }

      private:
        Stream &stream;
        BestEffortQueue< SmartPtr<Subband> > queue;
        Thread thread;

        void sendLoop()
        {
          SmartPtr<Subband> subband;

          while( (subband = queue.remove()) != NULL) {
            subband->write(stream);
          }
        }
      };

      class Receiver {
      public:
        typedef map<size_t, SmartPtr<BlockCollector> > CollectorMap;

        Receiver( Stream &stream, CollectorMap &collectors )
        :
          stream(stream),
          collectors(collectors),
          thread(this, &Receiver::receiveLoop)
        {
        }


        bool finish()
        {
          thread.wait();

          return !thread.caughtException();
        }

      private:
        Stream &stream;
        CollectorMap &collectors;
        Thread thread;


        void receiveLoop()
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
      };
    } // namespace TABTranspose
  } // namespace Cobalt
} // namespace LOFAR

#endif

