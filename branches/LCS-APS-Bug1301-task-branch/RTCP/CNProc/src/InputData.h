#ifndef LOFAR_CNPROC_INPUT_DATA_H
#define LOFAR_CNPROC_INPUT_DATA_H

#include <Common/lofar_complex.h>
#include <Common/DataConvert.h>
#include <Interface/Align.h>
#include <Interface/MultiDimArray.h>
#include <Interface/Config.h>
#include <Interface/SubbandMetaData.h>
#include <Stream/Stream.h>

#include <Interface/Allocator.h>

#include <vector>


namespace LOFAR {
namespace RTCP {

template <typename SAMPLE_TYPE> class InputData
{
  public:
    InputData(unsigned nrSubbands, unsigned nrSamplesToCNProc, Allocator &allocator = heapAllocator);

    void read(Stream *);

    // used for asynchronous transpose
    void readMetaData(Stream *str);
    void readOne(Stream *str);

    static size_t requiredSize(unsigned nrSubbands, unsigned nrSamplesToCNProc);

  private:
    unsigned		    itsNrSubbands;
    unsigned		    itsSubbandIndex;

  public:
    Cube<SAMPLE_TYPE>	    samples; //[outputPsets.size()][itsPS->nrSamplesToCNProc()][NR_POLARIZATIONS]
    Vector<SubbandMetaData> metaData; //[outputPsets.size()]
};


template <typename SAMPLE_TYPE> inline size_t InputData<SAMPLE_TYPE>::requiredSize(unsigned nrSubbands, unsigned nrSamplesToCNProc)
{
  return align(sizeof(SAMPLE_TYPE) * nrSubbands * nrSamplesToCNProc * NR_POLARIZATIONS, 32);
}


template <typename SAMPLE_TYPE> inline InputData<SAMPLE_TYPE>::InputData(unsigned nrSubbands, unsigned nrSamplesToCNProc, Allocator &allocator)
:
  itsNrSubbands(nrSubbands),
  itsSubbandIndex(0),
  samples(boost::extents[nrSubbands][nrSamplesToCNProc][NR_POLARIZATIONS], 32, allocator),
  metaData(nrSubbands)
{
}


// used for asynchronous transpose
template <typename SAMPLE_TYPE> inline void InputData<SAMPLE_TYPE>::readMetaData(Stream *str)
{
  // read all metadata
  str->read(&metaData[0], metaData.size() * sizeof(SubbandMetaData));
}

// used for asynchronous transpose
template <typename SAMPLE_TYPE> inline void InputData<SAMPLE_TYPE>::readOne(Stream *str)
{
  str->read(samples[itsSubbandIndex].origin(), samples[itsSubbandIndex].num_elements() * sizeof(SAMPLE_TYPE));

#if defined C_IMPLEMENTATION && defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, samples[itsSubbandIndex].origin(), samples[itsSubbandIndex].num_elements());
#endif

  if (++ itsSubbandIndex == itsNrSubbands) // we have read all data
    itsSubbandIndex = 0;
}

template <typename SAMPLE_TYPE> inline void InputData<SAMPLE_TYPE>::read(Stream *str)
{
  // read all metadata
  str->read(&metaData[0], metaData.size() * sizeof(SubbandMetaData));

  // now read all subbands using one recvBlocking call, even though the ION
  // sends all subbands one at a time
  str->read(samples.origin(), samples.num_elements() * sizeof(SAMPLE_TYPE));

#if defined C_IMPLEMENTATION && defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, samples[itsSubbandIndex].origin(), samples[itsSubbandIndex].num_elements());
#endif
}

} // namespace RTCP
} // namespace LOFAR

#endif
