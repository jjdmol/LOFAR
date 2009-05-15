//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include "AsyncTranspose.h"

#include <Interface/CN_Mapping.h>
#include <Interface/PrintVector.h>

#include <cassert>


namespace LOFAR {
namespace RTCP {

#if defined HAVE_MPI

#define MAX_TAG 100000 // The maximum tag we use to represent a data message. 
                       // Higher tags are metadata.

template <typename SAMPLE_TYPE> AsyncTranspose<SAMPLE_TYPE>::AsyncTranspose(bool isTransposeInput, bool isTransposeOutput, unsigned nrCoresPerPset,
									    const LocationInfo &locationInfo, 
									    const std::vector<unsigned> &inputPsets, 
									    const std::vector<unsigned> &outputPsets, unsigned nrSamplesToCNProc)
:
  itsIsTransposeInput(isTransposeInput),
  itsIsTransposeOutput(isTransposeOutput),
  itsInputPsets(inputPsets),
  itsOutputPsets(outputPsets),
  itsLocationInfo(locationInfo)
{
  itsGroupNumber = -1;
  for(unsigned core = 0; core < nrCoresPerPset; core++) {
    unsigned rank = locationInfo.remapOnTree(locationInfo.psetNumber(), core);
    if(rank == locationInfo.rank()) {
      itsGroupNumber = core;
      break;
    }
  }

  for(unsigned i=0; i<inputPsets.size(); i++) {
    unsigned rank = locationInfo.remapOnTree(inputPsets[i], itsGroupNumber);
    itsRankToPsetIndex[rank] = i;
  }

  itsMessageSize = InputData<SAMPLE_TYPE>::requiredSize(1, nrSamplesToCNProc);
  dataHandles.resize(inputPsets.size());
  metaDataHandles.resize(inputPsets.size());
  itsAsyncComm = new AsyncCommunication();
}


template <typename SAMPLE_TYPE> AsyncTranspose<SAMPLE_TYPE>::~AsyncTranspose()
{
    delete itsAsyncComm;
}


template <typename SAMPLE_TYPE> void AsyncTranspose<SAMPLE_TYPE>::postAllReceives(TransposedData<SAMPLE_TYPE> *transposedData)
{
    for(unsigned i=0; i<itsInputPsets.size(); i++) {
      void* buf = (void*) transposedData->samples[i].origin();
      unsigned pset = itsInputPsets[i];
      unsigned rank = itsLocationInfo.remapOnTree(pset, itsGroupNumber); // TODO cache this? maybe in locationInfo itself?
      dataHandles[i] = itsAsyncComm->asyncRead(buf, itsMessageSize, rank, rank);
      metaDataHandles[i] = itsAsyncComm->asyncRead(&transposedData->metaData[i], sizeof(SubbandMetaData), rank, rank + MAX_TAG);
    }
}


// returns station number (= pset index)
template <typename SAMPLE_TYPE> unsigned AsyncTranspose<SAMPLE_TYPE>::waitForAnyReceive()
{
  void* buf;
  unsigned size, source;
  int tag;

  while(true) {
    // This read could return either a data message, or a meta data message.
    itsAsyncComm->waitForAnyRead(buf, size, source, tag);

    // source is the real rank, calc pset index
    unsigned psetIndex = itsRankToPsetIndex[source];

    if(tag < MAX_TAG) { // real data message
      dataHandles[psetIndex] = -1; // record that we have received the data
      if(metaDataHandles[psetIndex] == -1) { // We already have the metadata
	return psetIndex;
      }
    } else { // metadata message
      metaDataHandles[psetIndex] = -1; // record that we have received the metadata
      if(dataHandles[psetIndex] == -1) {
	return psetIndex; // We already have the data
      }
    }
  }
}


template <typename SAMPLE_TYPE> void AsyncTranspose<SAMPLE_TYPE>::asyncSend(unsigned outputPsetNr, const InputData<SAMPLE_TYPE> *inputData)
{
  unsigned pset = itsOutputPsets[outputPsetNr];
  unsigned rank = itsLocationInfo.remapOnTree(pset, itsGroupNumber);
  int tag = itsLocationInfo.rank();

  itsAsyncComm->asyncWrite(inputData->samples[outputPsetNr].origin(), itsMessageSize, rank, tag);
  itsAsyncComm->asyncWrite(&inputData->metaData[outputPsetNr], sizeof(SubbandMetaData), rank, tag + MAX_TAG);
}


template <typename SAMPLE_TYPE> void AsyncTranspose<SAMPLE_TYPE>::waitForAllSends()
{
  // this includes the metadata writes...
  itsAsyncComm->waitForAllWrites();
}

template class AsyncTranspose<i4complex>;
template class AsyncTranspose<i8complex>;
template class AsyncTranspose<i16complex>;

#endif // MPI

} // namespace RTCP
} // namespace LOFAR
