#include <lofar_config.h>

#include <GPUProc/Station/StationInput.h>
#include <InputProc/Station/PacketFactory.h>
#include <InputProc/SampleType.h>

#include <UnitTest++.h>
#include <mpi.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

typedef SampleType<i16complex> SampleT;
const BoardMode mode(SampleT::bitMode(), 200);

class TestPacketFactory: public PacketFactory
{
public:
  TestPacketFactory()
  :
    PacketFactory(mode)
  {
  }

protected:
  virtual bool makePayload( struct RSP &packet )
  {
    size_t n = 1; // don't write 0, its the neutral value

    for(size_t beamlet = 0; beamlet < packet.header.nrBeamlets; beamlet++) {
      for(size_t timeslot = 0; timeslot < packet.header.nrBlocks; timeslot++) {
        struct RSP::Payload::samples16bit_t &sample = 
          packet.payload.samples16bit[packet.header.nrBlocks * beamlet + timeslot];

        sample.Xr = n++;
        sample.Xi = n++;
        sample.Yr = n++;
        sample.Yi = n++;
      }
    }

    return true;
  }
};

struct MPIDataTester {
public:
  const size_t nrSubbands;
  const size_t nrSamples;

  MPIData<SampleT> data;

  MPIDataTester()
  :
    nrSubbands(10),
    nrSamples(1024),
    data(nrSubbands, nrSamples)
  {
    data.block = 0;
    data.from = TimeStamp::now(mode.clockHz());
    data.to   = data.from + nrSamples;
  }

  // Test whether the data in the packet is the ONLY data written.
  //
  // Warning: DESTROYS the contents of this->data
  void checkAndClearPacketWritten( struct RSP &packet, const vector<ssize_t> &beamletIndices );
};

void MPIDataTester::checkAndClearPacketWritten( struct RSP &packet, const vector<ssize_t> &beamletIndices )
{
  for(size_t beamlet = 0; beamlet < packet.header.nrBeamlets; beamlet++) {
    const ssize_t beamletIdx = beamletIndices[beamlet];
    if (beamletIdx == -1)
      continue;

    size_t nrWrittenSamples = 0;

    for(size_t timeslot = 0; timeslot < packet.header.nrBlocks; timeslot++) {
      const uint64_t ts = packet.timeStamp() + timeslot - data.read_offsets[beamletIdx];
      if (ts < data.from || ts >= data.to)
        continue;

      SampleT &sample = data.mpi_samples[beamletIdx][ts - data.from];

      CHECK_EQUAL(real(packet.sample(beamlet, timeslot, 'X')), real(sample.x));
      CHECK_EQUAL(imag(packet.sample(beamlet, timeslot, 'X')), imag(sample.x));
      CHECK_EQUAL(real(packet.sample(beamlet, timeslot, 'Y')), real(sample.y));
      CHECK_EQUAL(imag(packet.sample(beamlet, timeslot, 'Y')), imag(sample.y));

      CHECK(data.metaData[beamletIdx].flags.test(ts - data.from));

      nrWrittenSamples++;

      /* clear this sample */
      sample.x = 0;
      sample.y = 0;
      data.metaData[beamletIdx].flags.exclude(ts - data.from);
    }
  }

  /* There should be nothing left, and all data should be 0 */
  for (size_t sb = 0; sb < nrSubbands; sb++) {
    for (size_t t = 0; t < nrSamples; t++) {
      CHECK_EQUAL(0, real(data.mpi_samples[sb][t].x));
      CHECK_EQUAL(0, imag(data.mpi_samples[sb][t].x));
      CHECK_EQUAL(0, real(data.mpi_samples[sb][t].y));
      CHECK_EQUAL(0, imag(data.mpi_samples[sb][t].y));
    }

    CHECK_EQUAL(0UL, data.metaData[sb].flags.count());
  }
}

SUITE(MPIData) {
  TEST_FIXTURE(MPIDataTester, write) {
    TestPacketFactory factory;
    struct RSP packet;

    // Straight 1:1 mapping, discard unused subbands
    const size_t nrBeamlets = mode.nrBeamletsPerBoard();
    ASSERT(nrBeamlets >= nrSubbands);

    vector<ssize_t> beamletIndices(nrBeamlets, -1);
    for (size_t sb = 0; sb < nrSubbands; ++sb)
      beamletIndices[sb] = sb;

    const size_t timesPerPacket = 16;
    for (ssize_t offset = -timesPerPacket - 1; offset < (ssize_t)(nrSamples + timesPerPacket + 1); offset++) {
      LOG_INFO_STR("Trying offset " << offset);

      // Clear block
      data.mpi_samples.reset();
      for (size_t i = 0; i < nrSubbands; ++i)
        data.metaData[i].flags.reset();

      // Create a packet [offset, offset + mode.nrBeamletsPerBoard()) relative
      // to our block
      factory.makePacket(packet, data.from + offset, 0);

      // Write data
      bool spill = data.write(packet, &beamletIndices[0]);

      // Validate whether we spill into the next block
      if ((uint64_t)packet.timeStamp() + packet.header.nrBlocks - 1 > data.to - 1) {
        // last sample is beyond data.to
        CHECK_EQUAL(true, spill);
      } else if ((uint64_t)packet.timeStamp() + packet.header.nrBlocks - 1 == data.to - 1) {
        // last sample is last sample in block
        CHECK_EQUAL(true, spill);
      } else {
        CHECK_EQUAL(false, spill);
      }

      // Validate result
      checkAndClearPacketWritten(packet, beamletIndices);
    }
  }

  TEST_FIXTURE(MPIDataTester, beamletIndices) {
    TestPacketFactory factory;
    struct RSP packet;

    // Create a shuffled mapping, inserting -1 for unused beamlets
    const size_t nrBeamlets = mode.nrBeamletsPerBoard();
    vector<ssize_t> beamletIndices(nrBeamlets, -1);

    for (size_t sb = 0; sb < nrSubbands; sb++)
      beamletIndices[sb] = (sb * 1031) % nrSubbands; // 1031 is prime, creating a ring
    ASSERT(1031 > nrSubbands); // sufficient for nrSubbands to be coprime to 1031

    // Write data
    factory.makePacket(packet, data.from, 0);
    (void)data.write(packet, &beamletIndices[0]);

    // Validate mapping
    checkAndClearPacketWritten(packet, beamletIndices);
  }

  TEST_FIXTURE(MPIDataTester, read_offsets) {
    TestPacketFactory factory;
    struct RSP packet;

    // Create a sorted mapping, inserting -1 for unused beamlets
    const size_t nrBeamlets = mode.nrBeamletsPerBoard();
    vector<ssize_t> beamletIndices(nrBeamlets, -1);

    for (size_t sb = 0; sb < nrSubbands; sb++)
      beamletIndices[sb] = sb;

    // Give subbband 3 an offset of -2, causing it to be written
    // 2 samples further.
    ASSERT(3 < nrSubbands);
    const ssize_t timesPerPacket = 16;
    for (ssize_t offset = - timesPerPacket - 1; offset <= timesPerPacket + 1; ++offset ) {
      data.read_offsets[3] = offset;

      // Write data, such that only subband 3 spills
      if (offset <= 0) {
        factory.makePacket(packet, data.from + nrSamples - timesPerPacket + offset, 0);

        bool spill = data.write(packet, &beamletIndices[0]);
        CHECK_EQUAL(true, spill);
      } else {
        factory.makePacket(packet, data.from - offset, 0);

        bool spill = data.write(packet, &beamletIndices[0]);
        CHECK_EQUAL(false, spill);
      }

      // Validate result
      checkAndClearPacketWritten(packet, beamletIndices);
    }
  }
}

SUITE(StationMetaData) {
  TEST(BlockGeneration) {
    Parset ps;

    /*
     * 1 second observation, with blocks of ~300ms
     *
     * will require 4 blocks [-1..3], which is within
     * the 5 blocks that StationMetaData will create in its
     * queue.
     */

    ps.add("Observation.VirtualInstrument.stationList", "[CS001]");
    ps.add("Observation.antennaSet",                    "LBA_INNER");
    ps.add("Cobalt.blockSize",                "50000");
    ps.add("Observation.nrBeams",             "1");
    ps.add("Observation.Beam[0].subbandList", "[0..9]");
    ps.add("Observation.startTime",           "2014-01-01 00:00:00");
    ps.add("Observation.stopTime",            "2014-01-01 00:00:01");
    ps.add("Observation.Dataslots.CS001LBA.DataslotList",  "[0..9]");
    ps.add("Observation.Dataslots.CS001LBA.RSPBoardList",  "[10*0]");
    ps.updateSettings();

    // Just send everything to rank 0
    SubbandDistribution subbandDistribution;
    for (size_t sb = 0; sb < ps.nrSubbands(); ++sb)
      subbandDistribution[0].push_back(sb);

    // Compute meta data
    StationMetaData<SampleT> sm(ps, 0, subbandDistribution);
    sm.computeMetaData();

    // Validate the blocks
    for (ssize_t block = -1; block < 3; ++block) {
      ASSERT(!sm.metaDataPool.filled.empty());
      SmartPtr< MPIData<SampleT> > data = sm.metaDataPool.filled.remove();

      ASSERT(data != NULL);
      CHECK_EQUAL(block, data->block);
    }

    // computeMetaData closes off with a NULL
    ASSERT(!sm.metaDataPool.filled.empty());
    SmartPtr< MPIData<SampleT> > data = sm.metaDataPool.filled.remove();
    CHECK(data == NULL);

    CHECK(sm.metaDataPool.filled.empty());
  }
}

int main(int argc, char **argv) {
  INIT_LOGGER("tStationInput");

  MPI_Init(&argc, &argv);

  int result = UnitTest::RunAllTests() > 0;

  MPI_Finalize();

  return result;
}
