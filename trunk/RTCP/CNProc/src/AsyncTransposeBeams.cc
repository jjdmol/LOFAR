//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <AsyncTransposeBeams.h>

#include <Interface/CN_Mapping.h>
#include <Interface/TransposedBeamFormedData.h>
#include <Interface/StokesData.h>
#include <Interface/PrintVector.h>
#include <Common/LofarLogger.h>

#include <cassert>


namespace LOFAR {
namespace RTCP {

#if defined HAVE_MPI


union Tag {
  struct {
    unsigned sourceRank :13; /* 0..8191, or two BG/P racks */
    unsigned comm       :2;
    unsigned subband    :8;
    //unsigned beam       :8;
  } info;

  unsigned nr;
};

AsyncTransposeBeams::AsyncTransposeBeams(
  const bool isTransposeInput, const bool isTransposeOutput, unsigned nrSubbands,
  const LocationInfo &locationInfo,
  const std::vector<unsigned> &inputPsets, const std::vector<unsigned> &outputPsets, const std::vector<unsigned> &usedCoresInPset )
:
  itsIsTransposeInput(isTransposeInput),
  itsIsTransposeOutput(isTransposeOutput),
  itsAsyncComm(),
  itsInputPsets(inputPsets),
  itsOutputPsets(outputPsets),
  itsUsedCoresInPset(usedCoresInPset),
  itsLocationInfo(locationInfo),
  itsCommHandles(itsNrCommunications,nrSubbands)
{
}

template <typename T,unsigned DIM> void AsyncTransposeBeams::postReceive(SampleData<T,DIM> *transposedData, unsigned subband, unsigned beam, unsigned psetIndex, unsigned coreIndex)
{
  unsigned pset = itsInputPsets[psetIndex];
  unsigned core = itsUsedCoresInPset[coreIndex];

  unsigned rank = itsLocationInfo.remapOnTree(pset, core); // TODO cache this? maybe in locationInfo itself?

  (void)beam;

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
    //t.info.beam       = beam;
    t.info.subband    = subband;

    //LOG_DEBUG_STR( "Posting to receive beam " << beam << " subband " << subband << " from pset " << pset << ", rank " << rank << ", tag " << t.nr );
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

    unsigned rank = t.info.sourceRank;
    unsigned subband = t.info.subband;

    //LOG_DEBUG_STR( "Received subband " << subband << " from pset ??, rank " << rank << ", tag " << tag );

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


template <typename T, unsigned DIM> void AsyncTransposeBeams::asyncSend(unsigned outputPsetIndex, unsigned coreIndex, unsigned subband, unsigned beam, const SampleData<T,DIM> *inputData)
{
  unsigned pset = itsOutputPsets[outputPsetIndex];
  unsigned core = itsUsedCoresInPset[coreIndex];
  unsigned rank = itsLocationInfo.remapOnTree(pset, core);

  // define what to write
  struct {
    const void   *ptr;
    const size_t size;
  } toWrite[itsNrCommunications] = {
    { inputData->samples[beam].origin(), inputData->samples[beam].num_elements() * sizeof(T) }
  };

  // write it
  for (unsigned h = 0; h < itsNrCommunications; h ++) {
    Tag t;
    t.info.sourceRank = itsLocationInfo.rank();
    t.info.comm       = h;
    t.info.subband    = subband;
    //t.info.beam       = beam;

    //LOG_DEBUG_STR( "Sending beam " << beam << " subband " << subband << " to pset " << pset << ", rank " << rank << ", tag " << t.nr );
    itsAsyncComm.asyncWrite(toWrite[h].ptr, toWrite[h].size, rank, t.nr);
  }
}

// specialisation for StokesData
template void AsyncTransposeBeams::postReceive(SampleData<float,4> *, unsigned, unsigned, unsigned, unsigned);
template void AsyncTransposeBeams::asyncSend(unsigned, unsigned, unsigned, unsigned, const SampleData<float,4> *);

// specialisation for BeamFormedData
template void AsyncTransposeBeams::postReceive(SampleData<fcomplex,4> *, unsigned, unsigned, unsigned, unsigned);
template void AsyncTransposeBeams::asyncSend(unsigned, unsigned, unsigned, unsigned, const SampleData<fcomplex,4> *);

void AsyncTransposeBeams::waitForAllSends()
{
  // this includes the metadata writes...
  itsAsyncComm.waitForAllWrites();
}


#endif // HAVE_MPI

} // namespace RTCP
} // namespace LOFAR
