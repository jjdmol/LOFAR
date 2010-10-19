#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_INPUT_DATA_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_INPUT_DATA_H

#include <Common/lofar_complex.h>
#include <Common/DataConvert.h>
#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/ION_to_CN.h>
#include <Transport/TH_Null.h>

#include <Allocator.h>
#include <TH_ZoidClient.h>

#include <boost/multi_array.hpp>


namespace LOFAR {
namespace CS1 {

class InputData
{
  public:
    InputData(const Heap &, unsigned nrSubbands, unsigned nrSamplesToBGLProc);
    ~InputData();

    void read(TransportHolder *th);

    static size_t requiredSize(unsigned nrSubbands, unsigned nrSamplesToBGLProc);

    typedef INPUT_SAMPLE_TYPE		  SampleType;

  private:
    Overlay				  overlay;

  public:
    boost::multi_array_ref<SampleType, 3> samples; //[outputPsets.size()][itsCS1PS->nrSamplesToBGLProc()][NR_POLARIZATIONS]

    ION_to_CN				  metaData;
};


inline size_t InputData::requiredSize(unsigned nrSubbands, unsigned nrSamplesToBGLProc)
{
  return sizeof(SampleType) * nrSubbands * nrSamplesToBGLProc * NR_POLARIZATIONS;
}


inline InputData::InputData(const Heap &heap, unsigned nrSubbands, unsigned nrSamplesToBGLProc)
:
  overlay(heap),
  samples(static_cast<SampleType *>(overlay.allocate(requiredSize(nrSubbands, nrSamplesToBGLProc), 32)), boost::extents[nrSubbands][nrSamplesToBGLProc][NR_POLARIZATIONS])
{
}


inline InputData::~InputData()
{
  overlay.deallocate(samples.origin());
}


inline void InputData::read(TransportHolder *th)
{
  metaData.read(th);

  // now read all subbands using one recvBlocking call, even though the ION
  // sends all subbands one at a time
  th->recvBlocking(samples.origin(), samples.num_elements() * sizeof(SampleType), 0, 0, 0);

#if defined C_IMPLEMENTATION && defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, samples.origin(), samples.num_elements());
#endif
}


} // namespace CS1
} // namespace LOFAR

#endif
