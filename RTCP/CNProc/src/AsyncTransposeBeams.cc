//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <AsyncTransposeBeams.h>

#include <Interface/CN_Mapping.h>
#include <Interface/PrintVector.h>
#include <Common/LofarLogger.h>

#include <cassert>


namespace LOFAR {
namespace RTCP {

#if defined HAVE_MPI

#define MAX_RANK 10000000 // used for message identification: id = type*MAX_RANK + rank
#define BASE     100000

AsyncTransposeBeams::AsyncTransposeBeams(
  const bool isTransposeInput, const bool isTransposeOutput, const unsigned myNrBeams, const unsigned myFirstBeam,
  const unsigned groupNumber, const LocationInfo &locationInfo, 
  const std::vector<unsigned> &inputPsets, const std::vector<unsigned> &outputPsets )
:
  itsIsTransposeInput(isTransposeInput),
  itsIsTransposeOutput(isTransposeOutput),
  itsAsyncComm(),
  itsInputPsets(inputPsets),
  itsOutputPsets(outputPsets),
  itsLocationInfo(locationInfo),
  itsCommHandles(itsNrCommunications,myNrBeams),
  itsGroupNumber(groupNumber),
  itsMyNrBeams(myNrBeams),
  itsMyFirstBeam(myFirstBeam)
{
  for (unsigned i = 0; i < inputPsets.size(); i ++) {
    unsigned rank = locationInfo.remapOnTree(inputPsets[i], itsGroupNumber);

    itsRankToPsetIndex[rank] = i;
  }
}

void AsyncTransposeBeams::postReceive(TransposedBeamFormedData *transposedData, unsigned globalBeamNr, unsigned subband, unsigned psetIndex)
{
  unsigned pset = itsInputPsets[psetIndex];
  unsigned rank = itsLocationInfo.remapOnTree(pset, itsGroupNumber); // TODO cache this? maybe in locationInfo itself?
  unsigned localBeamNr = globalBeamNr - itsMyFirstBeam;

  // define what to read
  struct {
    void   *ptr;
    size_t size;
  } toRead[itsNrCommunications] = {
    { transposedData->samples[localBeamNr][subband].origin(), transposedData->samples[localBeamNr][subband].num_elements() * sizeof transposedData->samples[0][0][0][0][0] }
  };

  // read it
  for (unsigned h = 0; h < itsNrCommunications; h ++) {
    LOG_DEBUG_STR( "Posting to receive async beam from pset " << pset << ", rank " << rank << " with tag " << rank + h * MAX_RANK + globalBeamNr * BASE);
    itsCommHandles[h][localBeamNr] = itsAsyncComm.asyncRead(toRead[h].ptr, toRead[h].size, rank, rank + h * MAX_RANK + globalBeamNr * BASE);
  }
}


// returns station number (= pset index)
unsigned AsyncTransposeBeams::waitForAnyReceive()
{
  while (true) {
    void     *buf;
    unsigned size, source;
    int      tag;

    // This read could return any type of message (out of itsCommHandles)
    itsAsyncComm.waitForAnyRead(buf, size, source, tag);

    // source is the real rank, calc pset index
    const unsigned psetIndex = itsRankToPsetIndex[source];

    unsigned rank = itsLocationInfo.remapOnTree(psetIndex, itsGroupNumber); // TODO cache this? maybe in locationInfo itself?
    unsigned globalBeamNr = ((tag % MAX_RANK) - rank) / BASE;
    unsigned localBeamNr = globalBeamNr - itsMyFirstBeam;

    LOG_DEBUG_STR( "Received async beam from pset " << psetIndex << " with tag " << tag << " rank " << rank << ", beam " << globalBeamNr);

    // mark the right communication handle as received
    for (unsigned h = 0; h < itsNrCommunications; h ++) {
      if (static_cast<unsigned>(tag) < (h + 1) * MAX_RANK) {
        itsCommHandles[h][localBeamNr] = -1;
        break;
      }
    }

    // check whether we have received all communications for this psetIndex.
    // This is the case when commHandles are -1.
    bool haveAll = true;

    for (unsigned h = 0; h < itsNrCommunications; h ++) {
      if (itsCommHandles[h][localBeamNr] != -1) {
        haveAll = false;
        break;
      }
    }

    if (haveAll)
      return localBeamNr;
  }
}


void AsyncTransposeBeams::asyncSend(unsigned outputPsetIndex, unsigned beam, const BeamFormedData *inputData)
{
  unsigned pset = itsOutputPsets[outputPsetIndex];
  unsigned rank = itsLocationInfo.remapOnTree(pset, itsGroupNumber);
  int	   tag  = itsLocationInfo.rank();

  // define what to write
  struct {
    const void   *ptr;
    const size_t size;
  } toWrite[itsNrCommunications] = {
    { inputData->samples[beam].origin(), inputData->samples[beam].num_elements() * sizeof inputData->samples[0][0][0][0] }
  };

  // write it
  for (unsigned h = 0; h < itsNrCommunications; h ++) {
    LOG_DEBUG_STR( "Sending async beam from pset " << pset << ", rank " << rank << " with tag " << rank + h * MAX_RANK + beam * BASE);
    itsAsyncComm.asyncWrite(toWrite[h].ptr, toWrite[h].size, rank, tag + h * MAX_RANK + beam * BASE);
  }
}


void AsyncTransposeBeams::waitForAllSends()
{
  // this includes the metadata writes...
  itsAsyncComm.waitForAllWrites();
}


#endif // HAVE_MPI

} // namespace RTCP
} // namespace LOFAR
