//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <AsyncTransposeBeams.h>

#include <Interface/CN_Mapping.h>
#include <Interface/PrintVector.h>
#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>

#include <cassert>

//#define DEBUG

namespace LOFAR {
namespace RTCP {

#if defined HAVE_MPI


union Tag {
  struct {
    unsigned sourceRank :13; /* 0..8191, or two BG/P racks */
    unsigned comm       :2;
    unsigned _dummy     :1;
    unsigned subband    :8;
    unsigned beam       :8;
  } info;

  uint32 nr;

  Tag(): nr(0) {}
};

AsyncTransposeBeams::AsyncTransposeBeams(
  const bool isTransposeInput, const bool isTransposeOutput, unsigned nrSubbands, unsigned nrSubbeams,
  const LocationInfo &locationInfo,
  const std::vector<unsigned> &inputPsets, const std::vector<unsigned> &inputCores, const std::vector<unsigned> &outputPsets, const std::vector<unsigned> &outputCores )
:
  itsIsTransposeInput(isTransposeInput),
  itsIsTransposeOutput(isTransposeOutput),
  itsAsyncComm(),
  itsInputPsets(inputPsets),
  itsInputCores(inputCores),
  itsOutputPsets(outputPsets),
  itsOutputCores(outputCores),
  itsLocationInfo(locationInfo),
  itsCommHandles(itsNrCommunications,nrSubbands),
  itsNrSubbeams(nrSubbeams)
{
}

template <typename T,unsigned DIM> void AsyncTransposeBeams::postReceive(SampleData<T,DIM> *transposedData, unsigned subband, unsigned beam, unsigned psetIndex, unsigned coreIndex)
{
  unsigned pset = itsInputPsets[psetIndex];
  unsigned core = itsInputCores[coreIndex];

  unsigned rank = itsLocationInfo.remapOnTree(pset, core); // TODO cache this? maybe in locationInfo itself?

  // define what to read
  struct {
    void   *ptr;
    size_t size;
  } toRead[itsNrCommunications] = {
    { transposedData->samples[subband].origin(), transposedData->samples[subband].num_elements() * sizeof(T) }
  };

  // read it
  for (unsigned h = 0; h < itsNrCommunications; h ++) {
    Tag t;

    t.info.sourceRank = rank;
    t.info.comm       = h;
    t.info.beam       = beam;
    t.info.subband    = subband;

#ifdef DEBUG
    LOG_DEBUG_STR( "Posting to receive beam " << beam << " subband " << subband << " from pset " << pset << " core " << core << " = rank " << rank << ", tag " << t.nr );
#endif
    itsCommHandles[h][subband] = itsAsyncComm.asyncRead(toRead[h].ptr, toRead[h].size, rank, t.nr);
  }
}

// returns station number (= pset index)
unsigned AsyncTransposeBeams::waitForAnyReceive()
{
  while (true) {
    void     *buf;
    unsigned size, source;
    int      tag;

    Tag t;

    // This read could return any type of message (out of itsCommHandles)
    itsAsyncComm.waitForAnyRead(buf, size, source, tag);

    t.nr = tag;

    unsigned subband = t.info.subband;
#ifdef DEBUG
    unsigned rank = t.info.sourceRank;
    LOG_DEBUG_STR( "Received subband " << subband << " from pset ??, rank " << rank << ", tag " << tag );
#endif
    // mark the right communication handle as received
    itsCommHandles[t.info.comm][subband] = -1;

    // check whether we have received all communications for this psetIndex.
    // This is the case when commHandles are -1.
    bool haveAll = true;

    for (unsigned h = 0; h < itsNrCommunications; h ++) {
      if (itsCommHandles[h][subband] != -1) {
        haveAll = false;
        break;
      }
    }

    if (haveAll)
      return subband;
  }
}


template <typename T, unsigned DIM> void AsyncTransposeBeams::asyncSend(unsigned outputPsetIndex, unsigned coreIndex, unsigned subband, unsigned beam, unsigned subbeam, const SampleData<T,DIM> *inputData)
{
  unsigned pset = itsOutputPsets[outputPsetIndex];
  unsigned core = itsOutputCores[coreIndex];
  unsigned rank = itsLocationInfo.remapOnTree(pset, core);

  // define what to write
  struct {
    const void   *ptr;
    const size_t size;
  } toWrite[itsNrCommunications] = {
    { inputData->samples[beam][subbeam].origin(), inputData->samples[beam][subbeam].num_elements() * sizeof(T) }
  };

  // write it
  for (unsigned h = 0; h < itsNrCommunications; h ++) {
    Tag t;

    t.info.sourceRank = itsLocationInfo.rank();
    t.info.comm       = h;
    t.info.subband    = subband;
    t.info.beam       = beam * itsNrSubbeams + subbeam;

#ifdef DEBUG
    LOG_DEBUG_STR( "Sending beam " << beam << " subband " << subband << " to pset " << pset << " core " << core << " = rank " << rank << ", tag " << t.nr );
#endif
    itsAsyncComm.asyncWrite(toWrite[h].ptr, toWrite[h].size, rank, t.nr);
  }
}

// specialisation for StokesData
template void AsyncTransposeBeams::postReceive(SampleData<float,4> *, unsigned, unsigned, unsigned, unsigned);
template void AsyncTransposeBeams::asyncSend(unsigned, unsigned, unsigned, unsigned, unsigned, const SampleData<float,4> *);

// specialisation for BeamFormedData
template void AsyncTransposeBeams::postReceive(SampleData<fcomplex,3> *, unsigned, unsigned, unsigned, unsigned);
template void AsyncTransposeBeams::asyncSend(unsigned, unsigned, unsigned, unsigned, unsigned, const SampleData<fcomplex,4> *);

void AsyncTransposeBeams::waitForAllSends()
{
  // this includes the metadata writes...
  itsAsyncComm.waitForAllWrites();
}


#endif // HAVE_MPI

} // namespace RTCP
} // namespace LOFAR
