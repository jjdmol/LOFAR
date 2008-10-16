#ifndef LOFAR_CNPROC_INPUT_DATA_H
#define LOFAR_CNPROC_INPUT_DATA_H

#include <Common/lofar_complex.h>
#include <Common/DataConvert.h>
#include <Interface/AlignedStdAllocator.h>
#include <Interface/Config.h>
#include <Interface/SubbandMetaData.h>
#include <Stream/Stream.h>

#include <Interface/Allocator.h>

#include <boost/multi_array.hpp>

#include <vector>


namespace LOFAR {
namespace RTCP {

template <typename SAMPLE_TYPE> class InputData
{
  public:
    InputData(const Arena &, unsigned nrSubbands, unsigned nrSamplesToCNProc);
    ~InputData();

    void read(Stream *);

    // used for asynchronous transpose
    void readMetaData(Stream *str);
    void readOne(Stream *str);

    static size_t requiredSize(unsigned nrSubbands, unsigned nrSamplesToCNProc);

  private:
    SparseSetAllocator			   allocator;
    unsigned                              itsNrSubbands;
    unsigned                              itsSubbandIndex;

  public:
    boost::multi_array_ref<SAMPLE_TYPE, 3> samples; //[outputPsets.size()][itsPS->nrSamplesToCNProc()][NR_POLARIZATIONS]

    std::vector<SubbandMetaData, AlignedStdAllocator<SubbandMetaData, 16> > metaData; //[outputPsets.size()]
};


template <typename SAMPLE_TYPE> inline size_t InputData<SAMPLE_TYPE>::requiredSize(unsigned nrSubbands, unsigned nrSamplesToCNProc)
{
  return sizeof(SAMPLE_TYPE) * nrSubbands * nrSamplesToCNProc * NR_POLARIZATIONS;
}


template <typename SAMPLE_TYPE> inline InputData<SAMPLE_TYPE>::InputData(const Arena &arena, unsigned nrSubbands, unsigned nrSamplesToCNProc)
:
  allocator(arena),
  itsNrSubbands(nrSubbands),
  itsSubbandIndex(0),
  samples(static_cast<SAMPLE_TYPE *>(allocator.allocate(requiredSize(nrSubbands, nrSamplesToCNProc), 32)), boost::extents[nrSubbands][nrSamplesToCNProc][NR_POLARIZATIONS]),
  metaData(nrSubbands)
{
}


template <typename SAMPLE_TYPE> inline InputData<SAMPLE_TYPE>::~InputData()
{
  allocator.deallocate(samples.origin());
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

    itsSubbandIndex++;
    if(itsSubbandIndex == itsNrSubbands) { // we have read all data
	itsSubbandIndex = 0;
    }
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
