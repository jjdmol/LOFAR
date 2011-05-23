#ifndef LOFAR_INTERFACE_CORRELATED_DATA_H
#define LOFAR_INTERFACE_CORRELATED_DATA_H

#include <Common/lofar_complex.h>
#include <Common/DataConvert.h>
#include <Interface/Align.h>
#include <Interface/Allocator.h>
#include <Interface/Config.h>
#include <Interface/StreamableData.h>
#include <Interface/MultiDimArray.h>
#include <Stream/Stream.h>


//#define LOFAR_STMAN_V2 

namespace LOFAR {
namespace RTCP {

class CorrelatedData: public StreamableData, public IntegratableData
{
  public:
    CorrelatedData(unsigned nrStations, unsigned nrChannels, Allocator & = heapAllocator);

    virtual IntegratableData &operator += (const IntegratableData &);

    const unsigned              itsAlignment;
    const unsigned              itsNrBaselines;

    MultiDimArray<fcomplex, 4>	visibilities; //[nrBaselines][nrChannels][NR_POLARIZATIONS][NR_POLARIZATIONS]

#if defined LOFAR_STMAN_V2
    typedef unsigned		CountType;
    Vector<CountType>		nrValidSamples; //[nrBaselines]
#else
    typedef unsigned short	CountType;
    Matrix<CountType>		nrValidSamples; //[nrBaselines][nrChannels]
#endif

    Vector<float>		centroids; //[nrBaselines]

  protected:
    virtual void		readData(Stream *);
    virtual void		writeData(Stream *);

  private:
    void			checkEndianness();
};


inline CorrelatedData::CorrelatedData(unsigned nrStations, unsigned nrChannels, Allocator &allocator)
:
#ifdef HAVE_BGP
  itsAlignment(32),
#else 
  itsAlignment(512),
#endif
  itsNrBaselines(nrStations * (nrStations + 1) / 2),
  visibilities(boost::extents[itsNrBaselines][nrChannels][NR_POLARIZATIONS][NR_POLARIZATIONS], itsAlignment, allocator),
  centroids(itsNrBaselines, itsAlignment, allocator)
{
  /// TODO Should this be aligned as well?? 
  
#if defined LOFAR_STMAN_V2
  std::cout << "[[CCC]] V2" << std::endl;
  nrValidSamples.resize(boost::extents[itsNrBaselines], itsAlignment, allocator);
#else
  nrValidSamples.resize(boost::extents[itsNrBaselines][nrChannels], itsAlignment, allocator);
#endif
}


inline void CorrelatedData::readData(Stream *str)
{
  str->read(visibilities.origin(), visibilities.num_elements() * sizeof(fcomplex));
  str->read(nrValidSamples.origin(), nrValidSamples.num_elements() * sizeof(CountType));

  //str->read(&centroids[0], centroids.size() * sizeof(float));
#if 0
  checkEndianness();
#endif  
}


inline void CorrelatedData::writeData(Stream *str) 
{
#if 0 && !defined WORDS_BIGENDIAN && !defined WRITE_BIG_ON_LITTLE_ENDIAN
  THROW(AssertError,"not implemented: think about endianness");
#endif

  str->write(visibilities.origin(), align(visibilities.num_elements() * sizeof(fcomplex), itsAlignment));
  str->write(nrValidSamples.origin(), align(nrValidSamples.num_elements() * sizeof(CountType), itsAlignment));
  //str->write(&centroids[0], centroids.size() * sizeof(float));
}


inline void CorrelatedData::checkEndianness()
{
#if 0 && !defined WORDS_BIGENDIAN && !defined WRITE_BIG_ON_LITTLE_ENDIAN
  dataConvert(LittleEndian, visibilities.origin(), visibilities.num_elements());
  dataConvert(LittleEndian, nrValidSamples.origin(), nrValidSamples.num_elements());
  // dataConvert(LittleEndian, &centroids[0], centroids.size());
#endif
}

inline IntegratableData &CorrelatedData::operator += (const IntegratableData &other_)
{
  const CorrelatedData &other = static_cast<const CorrelatedData &>(other_);

  // add visibilities
  {
    fcomplex	   *dst	 = visibilities.origin();
    const fcomplex *src	 = other.visibilities.origin();
    unsigned	   count = visibilities.num_elements();

    for (unsigned i = 0; i < count; i ++)
      dst[i] += src[i];
  }

  // add nr. valid samples
  {
    CountType       *dst  = nrValidSamples.origin();
    const CountType *src  = other.nrValidSamples.origin();
    unsigned	    count = nrValidSamples.num_elements();

    for (unsigned i = 0; i < count; i ++)
      dst[i] += src[i];
  }

  return *this;
}

} // namespace RTCP
} // namespace LOFAR

#endif
