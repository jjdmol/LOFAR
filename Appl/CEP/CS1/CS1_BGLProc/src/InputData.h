#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_INPUT_DATA_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_INPUT_DATA_H

#include <Common/lofar_complex.h>
#include <Common/DataConvert.h>
#include <CS1_Interface/AlignedStdAllocator.h>
#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/SubbandMetaData.h>
#include <Stream/Stream.h>

#include <CS1_Interface/Allocator.h>

#include <boost/multi_array.hpp>

#include <vector>


namespace LOFAR {
namespace CS1 {

class InputData
{
  public:
    InputData(const Arena &, unsigned nrSubbands, unsigned nrSamplesToBGLProc);
    ~InputData();

    void read(Stream *);

    static size_t requiredSize(unsigned nrSubbands, unsigned nrSamplesToBGLProc);

    typedef INPUT_SAMPLE_TYPE		  SampleType;

  private:
    SparseSetAllocator			  allocator;

  public:
    boost::multi_array_ref<SampleType, 3> samples; //[outputPsets.size()][itsCS1PS->nrSamplesToBGLProc()][NR_POLARIZATIONS]

    std::vector<SubbandMetaData, AlignedStdAllocator<SubbandMetaData, 16> > metaData; //[outputPsets.size()]
};


inline size_t InputData::requiredSize(unsigned nrSubbands, unsigned nrSamplesToBGLProc)
{
  return sizeof(SampleType) * nrSubbands * nrSamplesToBGLProc * NR_POLARIZATIONS;
}


inline InputData::InputData(const Arena &arena, unsigned nrSubbands, unsigned nrSamplesToBGLProc)
:
  allocator(arena),
  samples(static_cast<SampleType *>(allocator.allocate(requiredSize(nrSubbands, nrSamplesToBGLProc), 32)), boost::extents[nrSubbands][nrSamplesToBGLProc][NR_POLARIZATIONS]),
  metaData(nrSubbands)
{
}


inline InputData::~InputData()
{
  allocator.deallocate(samples.origin());
}


inline void InputData::read(Stream *str)
{
  // read all metadata
  str->read(&metaData[0], metaData.size() * sizeof(SubbandMetaData));

  // now read all subbands using one recvBlocking call, even though the ION
  // sends all subbands one at a time
  str->read(samples.origin(), samples.num_elements() * sizeof(SampleType));

#if defined C_IMPLEMENTATION && defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, samples.origin(), samples.num_elements());
#endif
}


} // namespace CS1
} // namespace LOFAR

#endif
