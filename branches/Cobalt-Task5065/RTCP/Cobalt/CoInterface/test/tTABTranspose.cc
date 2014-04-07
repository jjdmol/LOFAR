//# tParset.cc
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
//# $Id: tParset.cc 25931 2013-08-05 13:07:35Z klijn $

#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Stream/StringStream.h>
#include <CoInterface/TABTranspose.h>

#include <UnitTest++.h>
#include <boost/format.hpp>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace LOFAR::Cobalt::TABTranspose;
using namespace std;
using boost::format;

SUITE(Block) {
  TEST(orderedArrival) {
    const size_t nrSubbands = 10;
    const size_t nrSamples = 1024;
    const size_t nrChannels = 64;

    Block block(nrSubbands, nrSamples, nrChannels);

    for (size_t subbandIdx = 0; subbandIdx < nrSubbands; ++subbandIdx) {
      Subband subband(nrSamples, nrChannels);
      subband.id.subband = subbandIdx;

      CHECK(!block.complete());
      block.addSubband(subband);
    }

    CHECK(block.complete());
  }

  TEST(unorderedArrival) {
    const size_t nrSubbands = 10;
    const size_t nrSamples = 1024;
    const size_t nrChannels = 64;

    Block block(nrSubbands, nrSamples, nrChannels);

    // Our increment needs to be co-prime to nrSubbands for
    // the ring to visit all elements.
    const size_t subbandIncrement = 7;

    CHECK(nrSubbands % subbandIncrement != 0);
    CHECK(subbandIncrement % nrSubbands != 0);

    for (size_t n = 0; n < nrSubbands; ++n) {
      // avoid starting at 0, because the ordered test already does
      size_t subbandIdx = (3 + n * subbandIncrement) % nrSubbands;

      Subband subband(nrSamples, nrChannels);
      subband.id.subband = subbandIdx;

      CHECK(!block.complete());
      block.addSubband(subband);
    }

    CHECK(block.complete());
  }
}


/* A basic BlockCollector setup */
struct Fixture {
  static const size_t nrSubbands = 10;
  static const size_t nrSamples = 1024;
  static const size_t nrChannels = 64;
  static const size_t nrBlocks = 3;

  Pool<Block> outputPool;
  BlockCollector ctr;

  Fixture()
  :
    ctr(outputPool)
  {
    for (size_t i = 0; i < nrBlocks; ++i) {
      outputPool.free.append(new Block(nrSubbands, nrSamples, nrChannels));
    }
  }
};

struct Fixture_Loss: public Fixture {
  static const size_t maxInFlight = 2;

  BlockCollector ctr_loss;

  Fixture_Loss()
  :
    ctr_loss(outputPool, maxInFlight)
  {
    // add some subbands for both blocks
    for (size_t blockIdx = 0; blockIdx < maxInFlight; ++blockIdx) {
      for (size_t subbandIdx = 0; subbandIdx < nrSubbands; ++subbandIdx) {
        // We'll drop subband 3.
        //
        // Note that we drop the same subband for all blocks, to prevent
        // early detection of data loss.
        if (subbandIdx == 3)
          continue;

        Subband sb(nrSamples, nrChannels);
        sb.id.block = blockIdx;
        sb.id.subband = subbandIdx;

        ctr_loss.addSubband(sb);
      }
    }

    // shouldn't have emitted anything yet
    CHECK_EQUAL(0UL, outputPool.filled.size());
  }

};

SUITE(BlockCollector) {
  TEST_FIXTURE(Fixture, OneBlock) {
    // add all subbands for block 0
    for (size_t subbandIdx = 0; subbandIdx < nrSubbands; ++subbandIdx) {
      Subband sb(nrSamples, nrChannels);
      sb.id.block = 0;
      sb.id.subband = subbandIdx;

      ctr.addSubband(sb);
    }

    CHECK_EQUAL(1UL,          outputPool.filled.size());
    CHECK_EQUAL(nrBlocks - 1, outputPool.free.size());
  }

  TEST_FIXTURE(Fixture, AllBlocks) {
    // add all subbands for all blocks
    for (size_t blockIdx = 0; blockIdx < nrBlocks; ++blockIdx) {
      for (size_t subbandIdx = 0; subbandIdx < nrSubbands; ++subbandIdx) {
        Subband sb(nrSamples, nrChannels);
        sb.id.block = blockIdx;
        sb.id.subband = subbandIdx;

        ctr.addSubband(sb);
      }
    }

    CHECK_EQUAL(+nrBlocks, outputPool.filled.size());
    CHECK_EQUAL(0UL,       outputPool.free.size());
  }

  TEST_FIXTURE(Fixture, Loss_OneSubband) {
    // add some subbands for block 0
    for (size_t subbandIdx = 0; subbandIdx < nrSubbands; ++subbandIdx) {
      // We'll drop subband 3
      if (subbandIdx == 3)
        continue;

      Subband sb(nrSamples, nrChannels);
      sb.id.block = 0;
      sb.id.subband = subbandIdx;

      ctr.addSubband(sb);
    }

    // shouldn't have emitted anything yet
    CHECK_EQUAL(0UL, outputPool.filled.size());

    // add all subbands for block 1
    for (size_t subbandIdx = 0; subbandIdx < nrSubbands; ++subbandIdx) {
      Subband sb(nrSamples, nrChannels);
      sb.id.block = 1;
      sb.id.subband = subbandIdx;

      ctr.addSubband(sb);
    }

    // both blocks are now emitted, because subband 3 of block 0 is
    // considered lost by the arrival of subband 3 of block 1
    CHECK_EQUAL(2UL,            outputPool.filled.size());
    CHECK_EQUAL(nrBlocks - 2UL, outputPool.free.size());
  }

  TEST_FIXTURE(Fixture_Loss, Loss_MaxBlocksInFlight) {
    // add one subband for a new block, causing block
    // 0 to spill.
    {
      Subband sb(nrSamples, nrChannels);
      sb.id.block = maxInFlight;
      sb.id.subband = 0;

      ctr_loss.addSubband(sb);
    }

    // the first block should have been forced out
    CHECK_EQUAL(1UL,                          outputPool.filled.size());

    // some blocks are in flight, one has been emitted
    CHECK_EQUAL(nrBlocks - maxInFlight - 1UL, outputPool.free.size());

    // let subband 3 arrive late
    {
      Subband sb(nrSamples, nrChannels);
      sb.id.block = 0;
      sb.id.subband = 3;

      ctr_loss.addSubband(sb);
    }

    // there should be no change, even though this would have completed
    // a block, the block was already emitted
    CHECK_EQUAL(1UL,                          outputPool.filled.size());
    CHECK_EQUAL(nrBlocks - maxInFlight - 1UL, outputPool.free.size());
  }

  TEST_FIXTURE(Fixture, Finish) {
    // add some subbands for all blocks
    for (size_t blockIdx = 0; blockIdx < nrBlocks; ++blockIdx) {
      for (size_t subbandIdx = 0; subbandIdx < nrSubbands; ++subbandIdx) {
        // We'll drop subband 3
        if (subbandIdx == 3)
          continue;

        Subband sb(nrSamples, nrChannels);
        sb.id.block = blockIdx;
        sb.id.subband = subbandIdx;

        ctr.addSubband(sb);
      }
    }

    // shouldn't have emitted anything yet
    CHECK_EQUAL(0UL, outputPool.filled.size());
    CHECK_EQUAL(0UL, outputPool.free.size());

    ctr.finish();

    // should have emitted everything, plus
    // the terminating NULL entry
    CHECK_EQUAL(nrBlocks + 1, outputPool.filled.size());
    CHECK_EQUAL(0UL,          outputPool.free.size());
  }
}

SUITE(SendReceive) {
  TEST(NullSender) {
    StringStream str;

    Sender sender(str);

    sender.finish();
  }

  TEST(NullReceiver) {
    StringStream str;
    Receiver::CollectorMap collectors;

    Receiver receiver(str, collectors);

    str.close();
  }

  TEST_FIXTURE(Fixture, OneToOne) {
    Receiver::CollectorMap collectors;

    // We wrap this in a SmartPtr, but prevent freeing, because we haven't
    // malloc()ed ctr!

    SmartPtr<BlockCollector> collptr(&ctr);
    collectors[0] = collptr;

    try {
      {
        StringStream str;

        Receiver receiver(str, collectors);
        Sender sender(str);

        SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
        sb->id.fileIdx = 0;
        sb->id.block = 0;
        sb->id.subband = 0;

        sender.append(sb);
        sender.finish();

        str.close();
      }

      // force block emission
      ctr.finish();

      CHECK_EQUAL(2UL, outputPool.filled.size()); // block, plus NULL

      // We didn't new() ctr, so don't let it be free()d!!
      collectors[0].release();
    } catch(LOFAR::Exception &ex) {
      LOG_FATAL_STR("Caught exception: " << ex);
      // We didn't new() ctr, so don't let it be free()d!!
      collectors[0].release();
      throw;
    } catch(...) {
      // We didn't new() ctr, so don't let it be free()d!!
      collectors[0].release();

      throw;
    }
  }
}

int main(void)
{
  INIT_LOGGER("tTABTranspose");

  return UnitTest::RunAllTests() > 0;
}

