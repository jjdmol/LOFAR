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
//# $Id$

#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <Stream/StringStream.h>
#include <CoInterface/TABTranspose.h>

#include <UnitTest++.h>
#include <boost/format.hpp>
#include <omp.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace LOFAR::Cobalt::TABTranspose;
using namespace std;
using boost::format;

SUITE(Block) {
  TEST(OrderedArrival) {
    const size_t nrSubbands = 10;
    const size_t nrSamples = 1024;
    const size_t nrChannels = 64;

    Block block(0, 0, nrSubbands, nrSamples, nrChannels);

    for (size_t subbandIdx = 0; subbandIdx < nrSubbands; ++subbandIdx) {
      SmartPtr<Subband> subband = new Subband(nrSamples, nrChannels);
      subband->id.subband = subbandIdx;

      CHECK(!block.complete());
      block.addSubband(subband);
    }

    CHECK(block.complete());
  }

  TEST(UnorderedArrival) {
    const size_t nrSubbands = 10;
    const size_t nrSamples = 1024;
    const size_t nrChannels = 64;

    Block block(0, 0, nrSubbands, nrSamples, nrChannels);

    // Our increment needs to be co-prime to nrSubbands for
    // the ring to visit all elements.
    const size_t subbandIncrement = 7;

    CHECK(nrSubbands % subbandIncrement != 0);
    CHECK(subbandIncrement % nrSubbands != 0);

    for (size_t n = 0; n < nrSubbands; ++n) {
      // avoid starting at 0, because the ordered test already does
      size_t subbandIdx = (3 + n * subbandIncrement) % nrSubbands;

      SmartPtr<Subband> subband = new Subband(nrSamples, nrChannels);
      subband->id.subband = subbandIdx;

      CHECK(!block.complete());
      block.addSubband(subband);
    }

    CHECK(block.complete());
  }

  TEST(TransposeSpeed) {
    size_t nrChannelsList[] = { 1, 16, 256 };

    for (size_t c = 0; c < sizeof nrChannelsList / sizeof nrChannelsList[0]; c++) {
      const size_t nrSubbands = 488;
      const size_t nrChannels = nrChannelsList[c];
      const size_t nrSamples = 196608 / nrChannels;

      // Our increment needs to be co-prime to nrSubbands for
      // the ring to visit all elements.
      const size_t subbandIncrement = 7;

      CHECK(nrSubbands % subbandIncrement != 0);
      CHECK(subbandIncrement % nrSubbands != 0);

      Block block(0, 0, nrSubbands, nrSamples, nrChannels);

      for (size_t n = 0; n < nrSubbands; ++n) {
        size_t subbandIdx = (3 + n * subbandIncrement) % nrSubbands;

        SmartPtr<Subband> subband = new Subband(nrSamples, nrChannels);
        subband->id.subband = subbandIdx;

        block.addSubband(subband);
      }

      BeamformedData output(
        boost::extents[nrSamples][nrSubbands][nrChannels],
        boost::extents[nrSubbands][nrChannels]);

      NSTimer transposeTimer(str(format("Block::write for %u subbands, %u channels, %u samples") % nrSubbands % nrChannels % nrSamples), true, true);
      transposeTimer.start();
      block.write(output);
      transposeTimer.stop();
    }
  }
}


/* A basic BlockCollector setup */
struct Fixture {
  static const size_t nrSubbands = 10;
  static const size_t nrSamples = 1024;
  static const size_t nrChannels = 64;
  static const size_t nrBlocks = 3;

  Pool<BeamformedData> outputPool;
  BlockCollector ctr;

  Fixture()
  :
    outputPool("Fixture::outputPool"),
    ctr(outputPool, 0, nrSubbands, nrChannels, nrSamples)
  {
    for (size_t i = 0; i < nrBlocks; ++i) {
      outputPool.free.append(new BeamformedData(
        boost::extents[nrSamples][nrSubbands][nrChannels],
        boost::extents[nrSubbands][nrChannels]), false);
    }
  }
};

struct Fixture_Loss: public Fixture {
  static const size_t maxInFlight = 2;

  BlockCollector ctr_loss;

  Fixture_Loss()
  :
    ctr_loss(outputPool, 0, nrSubbands, nrChannels, nrSamples, 0, maxInFlight)
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

        SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
        sb->id.block = blockIdx;
        sb->id.subband = subbandIdx;

        ctr_loss.addSubband(sb);
      }
    }

    // shouldn't have emitted anything yet
    CHECK_EQUAL(0UL, outputPool.filled.size());

    ctr_loss.finish();
    // should have emitted all blocks, plus NULL
    CHECK_EQUAL(maxInFlight + 1, outputPool.filled.size());
  }

};

SUITE(BlockCollector) {
  TEST_FIXTURE(Fixture, OneBlock) {
    // add all subbands for block 0
    for (size_t subbandIdx = 0; subbandIdx < nrSubbands; ++subbandIdx) {
      SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
      sb->id.block = 0;
      sb->id.subband = subbandIdx;

      ctr.addSubband(sb);
    }

    ctr.finish();
    CHECK_EQUAL(2UL,          outputPool.filled.size());
    CHECK_EQUAL(nrBlocks - 1, outputPool.free.size());
  }

  TEST_FIXTURE(Fixture, AllBlocks) {
    // add all subbands for all blocks
    for (size_t blockIdx = 0; blockIdx < nrBlocks; ++blockIdx) {
      for (size_t subbandIdx = 0; subbandIdx < nrSubbands; ++subbandIdx) {
        SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
        sb->id.block = blockIdx;
        sb->id.subband = subbandIdx;

        ctr.addSubband(sb);
      }
    }

    ctr.finish();
    CHECK_EQUAL(nrBlocks + 1, outputPool.filled.size());
    CHECK_EQUAL(0UL,          outputPool.free.size());
  }

  TEST_FIXTURE(Fixture, OutOfOrder) {
    // we add different subbands for each block to allow for any arrival order to be tried.
    // for each block, we add one subband and consider the rest to be lost, to keep the test simple.

    // max of 2 blocks in flight
    BlockCollector ctr_loss(outputPool, 0, nrSubbands, nrChannels, nrSamples, 0, 2);

    // we add blocks [1,3], enough to keep in flight
    {
      SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
      sb->id.block = 1;
      sb->id.subband = 0;
      ctr_loss.addSubband(sb);
    }
    {
      SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
      sb->id.block = 3;
      sb->id.subband = 1;
      ctr_loss.addSubband(sb);
    }

    // we let block 0 arrive, which could (erroneously) emit block 1
    // in favour of block 0.
    {
      SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
      sb->id.block = 0;
      sb->id.subband = 2;
      ctr_loss.addSubband(sb);
    }

    // if ok, we now have [1,3] still, and should be able to add block 2,
    // causing block 1 to be emitted.
    {
      SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
      sb->id.block = 2;
      sb->id.subband = 3;
      ctr_loss.addSubband(sb);
    }

    // emit remaining blocks: [2,3]
    ctr_loss.finish();

    // should have emitted blocks [1,2,3], plus terminating NULL
    CHECK_EQUAL(4UL, outputPool.filled.size());
  }

  TEST_FIXTURE(Fixture, Loss_OneSubband) {
    // add some subbands for block 0
    for (size_t subbandIdx = 0; subbandIdx < nrSubbands; ++subbandIdx) {
      // We'll drop subband 3
      if (subbandIdx == 3)
        continue;

      SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
      sb->id.block = 0;
      sb->id.subband = subbandIdx;

      ctr.addSubband(sb);
    }

    ctr.finish();
    CHECK_EQUAL(2UL, outputPool.filled.size());
/*
    // add all subbands for block 1
    for (size_t subbandIdx = 0; subbandIdx < nrSubbands; ++subbandIdx) {
      SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
      sb->id.block = 1;
      sb->id.subband = subbandIdx;

      ctr.addSubband(sb);
    }

    // both blocks are now emitted, because subband 3 of block 0 is
    // considered lost by the arrival of subband 3 of block 1
    ctr.finish();
    CHECK_EQUAL(2UL,            outputPool.filled.size());
    CHECK_EQUAL(nrBlocks - 2UL, outputPool.free.size());
*/
  }

  TEST_FIXTURE(Fixture_Loss, Loss_MaxBlocksInFlight) {
    // add one subband for a new block, causing block
    // 0 to spill.
    {
      SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
      sb->id.block = maxInFlight;
      sb->id.subband = 0;

      ctr_loss.addSubband(sb);
    }

    ctr_loss.finish();

    // all blocks should have been forced out
    CHECK_EQUAL(maxInFlight + 1,        outputPool.filled.size());
    CHECK_EQUAL(nrBlocks - maxInFlight, outputPool.free.size());
/*
    // let subband 3 arrive late
    {
      SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
      sb->id.block = 0;
      sb->id.subband = 3;

      ctr_loss.addSubband(sb);
    }

    // there should be no change, even though this would have completed
    // a block, the block was already emitted
    ctr_loss.finish();
    CHECK_EQUAL(1UL,                          outputPool.filled.size());
    CHECK_EQUAL(nrBlocks - maxInFlight - 1UL, outputPool.free.size());
*/
  }

  TEST_FIXTURE(Fixture, Finish) {
    // add some subbands for all blocks
    for (size_t blockIdx = 0; blockIdx < nrBlocks; ++blockIdx) {
      for (size_t subbandIdx = 0; subbandIdx < nrSubbands; ++subbandIdx) {
        // We'll drop subband 3
        if (subbandIdx == 3)
          continue;

        SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
        sb->id.block = blockIdx;
        sb->id.subband = subbandIdx;

        ctr.addSubband(sb);
      }
    }

    // should have emitted everything, plus
    // the terminating NULL entry
    ctr.finish();
    CHECK_EQUAL(nrBlocks + 1, outputPool.filled.size());
    CHECK_EQUAL(0UL,          outputPool.free.size());
  }
}

SUITE(SendReceive) {
  TEST(NullReceiver) {
    StringStream str;
    Receiver::CollectorMap collectors;

    Receiver receiver(str, collectors);

    str.close();

    CHECK(receiver.finish());
  }

  // Test teardown if Receiver is destructed
  TEST(ReceiverDies) {
    StringStream str;
    Receiver::CollectorMap collectors;
    Receiver receiver(str, collectors);
  }

  TEST(IllegalReceive) {
    StringStream str;
    Receiver::CollectorMap collectors;

    Receiver receiver(str, collectors);

    // Receiver is not configured to receive any TABs,
    // so any write should result in the receiveLoop
    // throwing an exception.
    Subband sb;
    sb.id.fileIdx = 0;
    sb.id.block = 0;
    sb.id.subband = 0;
    sb.write(str);
    str.close();

    // Receiver thread should have thrown an exception
    CHECK(!receiver.finish());
  }

  TEST_FIXTURE(Fixture, OneToOne) {
    const size_t nrTABs = nrBlocks; // we know we have enough outputPool.free for nrBlocks TABs
    map<size_t, SmartPtr< Pool<BeamformedData> > > outputPools;
    Receiver::CollectorMap collectors;

    for (size_t i = 0; i < nrTABs; ++i) {
      outputPools[i] = new Pool<BeamformedData>(str(format("OneToOne::outputPool[%u]") % i));
      for (size_t b = 0; b < nrBlocks; ++b) {
        outputPools[i]->free.append(new BeamformedData(
          boost::extents[nrSamples][nrSubbands][nrChannels],
          boost::extents[nrSubbands][nrChannels]), false);
      }

      collectors[i] = new BlockCollector(*outputPools[i], i, nrSubbands, nrChannels, nrSamples);
    }

    StringStream str;

    Receiver receiver(str, collectors);

    for (size_t i = 0; i < nrTABs * nrSubbands; ++i) {
      SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
      sb->id.fileIdx = i % nrTABs;
      sb->id.block = 0;
      sb->id.subband = i / nrTABs;

      /* fill data */
      size_t x = 0;
      for (size_t s = 0; s < nrSamples; ++s)
        for (size_t c = 0; c < nrChannels; ++c)
          sb->data[s][c] = (i+1) * ++x;

      sb->write(str);
    }

    str.close();

    // Wait for the receiver to receive and process everything
    CHECK(receiver.finish());

    for (size_t i = 0; i < nrTABs; ++i) {
      // Should have one complete block, plus NULL
      CHECK_EQUAL(2UL, outputPools[i]->filled.size());

      SmartPtr<BeamformedData> block = outputPools[i]->filled.remove();

      CHECK(block != NULL);

      CHECK_EQUAL(0UL, block->sequenceNumber());

      /* check data */
      for (size_t sb = 0; sb < nrSubbands; ++sb) {
        size_t x = 0;

        for (size_t s = 0; s < nrSamples; ++s) {
          for (size_t c = 0; c < nrChannels; ++c) {
            size_t expected = (sb * nrTABs + i + 1) * ++x;
            size_t actual = static_cast<size_t>(block->samples[s][sb][c]);

            if (expected != actual)
              LOG_ERROR_STR("Mismatch at [" << s << "][" << sb << "][" << c << "]");
            CHECK_EQUAL(expected, actual);
          }
        }
      }
    }
  }
}

SUITE(MultiReceiver) {
  TEST(MultiReceiverDies) {
    Receiver::CollectorMap collectors;
    MultiReceiver mr("foo", collectors);
  }

  TEST(Connect) {
    Receiver::CollectorMap collectors;
    MultiReceiver mr("foo-", collectors);

    // Connect with multiple clients
    {
      PortBroker::ClientStream cs1("localhost", PortBroker::DEFAULT_PORT, "foo-1", 1);
      PortBroker::ClientStream cs2("localhost", PortBroker::DEFAULT_PORT, "foo-2", 1);

      // Disconnect them too! (~cs)
    }

    mr.kill(2);
  }

  TEST_FIXTURE(Fixture, Transfer) {
    // Set up receiver
    Receiver::CollectorMap collectors;

    collectors[0] = new BlockCollector(outputPool, 0, nrSubbands, nrChannels, nrSamples);

    MultiReceiver mr("foo-", collectors);

    // Connect
    {
      PortBroker::ClientStream cs("localhost", PortBroker::DEFAULT_PORT, "foo-1", 1);

      // Send one block
      {
        SmartPtr<Subband> sb = new Subband(nrSamples, nrChannels);
        sb->id.fileIdx = 0;
        sb->id.block   = 0;
        sb->id.subband = 0;

        sb->write(cs);
      }

      // Disconnect (~cs)
    }

    // Flush receiver, but wait for data to arrive
    mr.kill(1);

    // Flush collecting of blocks
    collectors[0]->finish();

    // Check if data arrived, resulting in one (incomplete) block,
    // plus NULL marker.
    CHECK_EQUAL(2UL, outputPool.filled.size());
  }

  TEST(MultiSender) {
    MultiSender::HostMap hostMap;
    MultiSender msender(hostMap, false);
  }

  TEST(Transpose) {
    // We use the even fileIdx to simulate a sparse set
    #define FILEIDX(tabNr) ((tabNr)*2)

    LOG_DEBUG_STR("Transpose test started");

    const int nrSubbands = 4;
    const size_t nrBlocks = 2;
    const int nrTABs = 2;
    const size_t nrSamples = 16;
    const size_t nrChannels = 1;

    // Give both senders and receivers multiple tasks,
    // but not all the same amount.
    const int nrSenders = 4;
    const int nrReceivers = 2;

    Semaphore sendersDone;

    LOG_DEBUG_STR("Spawning threads");

    // Set up sinks -- can all draw from the same outputPool
#   pragma omp parallel sections num_threads(2)
    {
      // Receivers
#     pragma omp section
      {
#       pragma omp parallel for num_threads(nrReceivers)
        for (int r = 0; r < nrReceivers; ++r) {
          LOG_DEBUG_STR("Receiver thread " << r);

          LOG_DEBUG_STR("Populating outputPools");

          // collect our TABs
          std::map<size_t, SmartPtr< Pool<BeamformedData> > > outputPools;
          Receiver::CollectorMap collectors;

          for (int t = 0; t < nrTABs; ++t) {
            if (t % nrReceivers != r)
              continue;

            outputPools[t] = new Pool<BeamformedData>(str(format("MultiReceiver::Transpose::outputPool[%u]") % t));

            for (size_t i = 0; i < nrBlocks; ++i) {
              outputPools[t]->free.append(new BeamformedData(
                boost::extents[nrSamples][nrSubbands][nrChannels],
                boost::extents[nrSubbands][nrChannels]), false);
            }
            collectors[FILEIDX(t)] = new BlockCollector(*outputPools[t], FILEIDX(t), nrSubbands, nrChannels, nrSamples, nrBlocks);
          }

          LOG_DEBUG_STR("Starting receiver " << r);
          MultiReceiver mr(str(format("foo-%u-") % r), collectors);

          // Wait for end of data
          LOG_DEBUG_STR("Receiver " << r << ": Waiting for senders");
          sendersDone.down();
          LOG_DEBUG_STR("Receiver " << r << ": Shutting down");
          mr.kill(nrSenders);

          // Check output -- everything should have arrived
          for (int t = 0; t < nrTABs; ++t) {
            if (t % nrReceivers != r)
              continue;

            // Check if all blocks arrived, plus NULL marker.
            collectors[FILEIDX(t)]->finish();
            CHECK_EQUAL(nrBlocks + 1UL, outputPools[t]->filled.size());

            for (size_t b = 0; b < nrBlocks; ++b) {
              SmartPtr<BeamformedData> block = outputPools[t]->filled.remove();

              CHECK(block != NULL);

              // Blocks should have arrived in-order
              CHECK_EQUAL(b, block->sequenceNumber());
            }
          }
        }
      }

      // Senders
#     pragma omp section
      {
#       pragma omp parallel for num_threads(nrSenders)
        for (int s = 0; s < nrSenders; ++s) {
          LOG_DEBUG_STR("Sender thread " << s);

          MultiSender::HostMap hostMap;

          for (int t = 0; t < nrTABs; ++t) {
            size_t r = t % nrReceivers;

            struct MultiSender::Host host;

            host.hostName = str(format("127.0.0.%u") % (r+1));
            host.brokerPort = PortBroker::DEFAULT_PORT;
            host.service = str(format("foo-%s-%s") % r % s);

            hostMap[FILEIDX(t)] = host;
          }

          MultiSender msender(hostMap, false);

#         pragma omp parallel sections num_threads(2)
          {
#           pragma omp section
            {
              msender.process();
            }

#           pragma omp section
            {
              // Send blocks
              for (size_t b = 0; b < nrBlocks; ++b) {
                // Send our subbands
                for (int sb = 0; sb < nrSubbands; ++sb) {
                  if (sb % nrSenders != s)
                    continue;

                  // Send all TABs
                  for (int t = 0; t < nrTABs; ++t) {
                    SmartPtr<Subband> subband = new Subband(nrSamples, nrChannels);
                    subband->id.fileIdx = FILEIDX(t);
                    subband->id.block = b;
                    subband->id.subband = sb;

                    LOG_DEBUG_STR("Sender " << s << ": sending TAB " << t << " block " << b << " subband " << sb);
                    msender.append(subband);
                  }
                }
              }

              msender.finish();
            }
          }
        }

        // Signal end of data
        LOG_DEBUG_STR("Senders done.");
        sendersDone.up(nrReceivers);
      }
    }

  }
}

int main(void)
{
  INIT_LOGGER("tTABTranspose");

  omp_set_nested(true);

  PortBroker::createInstance(PortBroker::DEFAULT_PORT);

  return UnitTest::RunAllTests() > 0;
}

