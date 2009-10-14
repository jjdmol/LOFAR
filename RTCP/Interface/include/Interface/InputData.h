#ifndef LOFAR_CNPROC_INPUT_DATA_H
#define LOFAR_CNPROC_INPUT_DATA_H

#include <Common/lofar_complex.h>
#include <Common/DataConvert.h>
#include <Interface/Align.h>
#include <Interface/MultiDimArray.h>
#include <Interface/Config.h>
#include <Interface/StreamableData.h>
#include <Stream/Stream.h>

#include <Interface/Allocator.h>

#include <vector>


namespace LOFAR {
namespace RTCP {

template <typename SAMPLE_TYPE> class InputData: public SampleData<SAMPLE_TYPE,3>
{
  public:
    typedef SampleData<SAMPLE_TYPE,3> SuperType;

    InputData(const unsigned nrSubbands, const unsigned nrSamplesToCNProc);

    virtual InputData *clone() const { return new InputData(*this); }

    // used for asynchronous transpose
    void readOne(Stream *str, unsigned subbandPosition);

  protected:
    virtual void checkEndianness();

  private:
    const unsigned	    itsNrSubbands;
};


template <typename SAMPLE_TYPE> inline InputData<SAMPLE_TYPE>::InputData(const unsigned nrSubbands, const unsigned nrSamplesToCNProc)
:
  SuperType( false, boost::extents[nrSubbands][nrSamplesToCNProc][NR_POLARIZATIONS], 0 ),
  itsNrSubbands(nrSubbands)
{
}

// used for asynchronous transpose
template <typename SAMPLE_TYPE> inline void InputData<SAMPLE_TYPE>::readOne(Stream *str, unsigned subbandPosition)
{
  str->read(SuperType::samples[subbandPosition].origin(), SuperType::samples[subbandPosition].num_elements() * sizeof(SAMPLE_TYPE));

#if defined C_IMPLEMENTATION && defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, SuperType::samples[subbandPosition].origin(), SuperType::samples[subbandPosition].num_elements());
#endif
}

template <typename SAMPLE_TYPE> inline void InputData<SAMPLE_TYPE>::checkEndianness()
{
#if defined C_IMPLEMENTATION && defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, SuperType::samples.origin(), SuperType::samples.num_elements());
#endif
}

} // namespace RTCP
} // namespace LOFAR

#endif
