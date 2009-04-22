//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <AsyncTranspose.h>

#include <Interface/CN_Mapping.h>
#include <Interface/PrintVector.h>

#include <cassert>


namespace LOFAR {
namespace RTCP {

#if defined HAVE_MPI

#define MAX_TAG 100000 // The maximum tag we use to represent a data message. 
                       // Higher tags are metadata.

template <typename SAMPLE_TYPE> AsyncTranspose<SAMPLE_TYPE>::AsyncTranspose(
  const bool isTransposeInput, const bool isTransposeOutput, 
  const unsigned groupNumber, const LocationInfo &locationInfo, 
  const std::vector<unsigned> &inputPsets, const std::vector<unsigned> &outputPsets, 
  const unsigned nrSamplesToCNProc, const unsigned nrSubbands, const unsigned nrSubbandsPerPset)
:
  itsIsTransposeInput(isTransposeInput),
  itsIsTransposeOutput(isTransposeOutput),
  itsNrSubbands(nrSubbands),
  itsNrSubbandsPerPset(nrSubbandsPerPset),
  itsInputPsets(inputPsets),
  itsOutputPsets(outputPsets),
  itsLocationInfo(locationInfo),
  itsGroupNumber(groupNumber)
{
  InputData<SAMPLE_TYPE> oneSample( 1, nrSamplesToCNProc );

  for(unsigned i=0; i<inputPsets.size(); i++) {
    const unsigned rank = locationInfo.remapOnTree(inputPsets[i], itsGroupNumber);
    itsRankToPsetIndex[rank] = i;
  }

  itsMessageSize = oneSample.requiredSize();
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
  while(true) {
    void* buf;
    unsigned size, source;
    int tag;

    // This read could return either a data message, or a meta data message.
    itsAsyncComm->waitForAnyRead(buf, size, source, tag);

    // source is the real rank, calc pset index
    const unsigned psetIndex = itsRankToPsetIndex[source];

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


template <typename SAMPLE_TYPE> void AsyncTranspose<SAMPLE_TYPE>::asyncSend(const unsigned outputPsetNr, 
									    const InputData<SAMPLE_TYPE> *inputData)
{
  const unsigned pset = itsOutputPsets[outputPsetNr];
  const unsigned rank = itsLocationInfo.remapOnTree(pset, itsGroupNumber);
  const int tag = itsLocationInfo.rank();

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

#endif // HAVE_MPI

} // namespace RTCP
} // namespace LOFAR
