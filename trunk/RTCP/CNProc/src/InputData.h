#ifndef LOFAR_CNPROC_INPUT_DATA_H
#define LOFAR_CNPROC_INPUT_DATA_H

#include <Common/lofar_complex.h>
#include <Common/DataConvert.h>
#include <Interface/Align.h>
#include <Interface/MultiDimArray.h>
#include <Interface/Config.h>
#include <Interface/SubbandMetaData.h>
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

    InputData(const unsigned nrSubbands, const unsigned nrSamplesToCNProc, const unsigned nrPencilBeams);

    virtual void allocate( Allocator &allocator = heapAllocator );

    // used for asynchronous transpose
    void readMetaData(Stream *str);
    void readOne(Stream *str, unsigned subbandPosition);

    // used for synchronous transfer
    void readAll(Stream *str);

  private:
    const unsigned	    itsNrSubbands;
    const unsigned	    itsNrPencilBeams;

  public:
    Vector<SubbandMetaData> metaData; //[outputPsets.size()]
};


template <typename SAMPLE_TYPE> inline InputData<SAMPLE_TYPE>::InputData(const unsigned nrSubbands, const unsigned nrSamplesToCNProc, const unsigned nrPencilBeams)
:
  SuperType( false, boost::extents[nrSubbands][nrSamplesToCNProc][NR_POLARIZATIONS], 0 ),
  itsNrSubbands(nrSubbands),
  itsNrPencilBeams(nrPencilBeams)
{
}

template <typename SAMPLE_TYPE> inline void InputData<SAMPLE_TYPE>::allocate( Allocator &allocator )
{
  SuperType::allocate( allocator );
  metaData.resize( itsNrSubbands, 32 );

  for( unsigned i = 0; i < itsNrSubbands; i++ ) {
    metaData[i] = SubbandMetaData( itsNrPencilBeams );
  }
}

// used for asynchronous transpose
template <typename SAMPLE_TYPE> inline void InputData<SAMPLE_TYPE>::readMetaData(Stream *str)
{
  // read all metadata
  for( unsigned subband = 0; subband < itsNrSubbands; subband++ ) {
    metaData[subband].read( str );
  }
}

// used for asynchronous transpose
template <typename SAMPLE_TYPE> inline void InputData<SAMPLE_TYPE>::readOne(Stream *str, unsigned subbandPosition)
{
  str->read(SuperType::samples[subbandPosition].origin(), SuperType::samples[subbandPosition].num_elements() * sizeof(SAMPLE_TYPE));

#if defined C_IMPLEMENTATION && defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, SuperType::samples[subbandPosition].origin(), SuperType::samples[subbandPosition].num_elements());
#endif
}

template <typename SAMPLE_TYPE> inline void InputData<SAMPLE_TYPE>::readAll(Stream *str)
{
  // read all metadata
  readMetaData( str );

  // now read all subbands using one recvBlocking call, even though the ION
  // sends all subbands one at a time
  str->read(SuperType::samples.origin(), SuperType::samples.num_elements() * sizeof(SAMPLE_TYPE));

#if defined C_IMPLEMENTATION && defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, SuperType::samples.origin(), SuperType::samples.num_elements());
#endif
}

} // namespace RTCP
} // namespace LOFAR

#endif
