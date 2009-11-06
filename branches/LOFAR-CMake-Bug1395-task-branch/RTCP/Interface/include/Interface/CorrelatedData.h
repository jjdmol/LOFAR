#ifndef LOFAR_INTERFACE_CORRELATED_DATA_H
#define LOFAR_INTERFACE_CORRELATED_DATA_H

#include <Common/lofar_complex.h>
#include <Common/DataConvert.h>
#include <Interface/Align.h>
#include <Interface/Allocator.h>
#include <Interface/Config.h>
#include <Interface/Exceptions.h>
#include <Interface/StreamableData.h>
#include <Interface/MultiDimArray.h>
#include <Stream/Stream.h>

#include <boost/multi_array.hpp>
#include <vector>
#include <stdexcept>


namespace LOFAR {
namespace RTCP {

class CorrelatedData: public StreamableData
{
  public:
    CorrelatedData(unsigned nrBaselines, unsigned nrChannels);

    virtual CorrelatedData *clone() const { return new CorrelatedData(*this); }
    virtual size_t requiredSize() const;
    virtual void allocate(Allocator &allocator = heapAllocator);

    virtual StreamableData &operator += (const StreamableData &);

    MultiDimArray<fcomplex, 4>	visibilities; //[nrBaselines][nrChannels][NR_POLARIZATIONS][NR_POLARIZATIONS]
    Matrix<unsigned short>	nrValidSamples; //[nrBaselines][nrChannels]
    Vector<float>		centroids; //[nrBaselines]

  protected:
    virtual void readData(Stream *);
    virtual void writeData(Stream *);

  private:
    const unsigned              itsNrBaselines;
    const unsigned              itsNrChannels;
    unsigned                    itsAlignment;

    void			checkEndianness();

    size_t visibilitiesSize() const;
    size_t nrValidSamplesSize() const;
    size_t centroidSize() const;
};


inline size_t CorrelatedData::visibilitiesSize() const
{
  return align(sizeof(fcomplex) * itsNrBaselines * itsNrChannels * NR_POLARIZATIONS * NR_POLARIZATIONS, itsAlignment);
}


inline size_t CorrelatedData::nrValidSamplesSize() const
{
  return align(sizeof(unsigned short) * itsNrBaselines * itsNrChannels, itsAlignment);
}


inline size_t CorrelatedData::centroidSize() const
{
  return align(sizeof(float) * itsNrBaselines, itsAlignment);
}


inline size_t CorrelatedData::requiredSize() const
{
  return visibilitiesSize() + nrValidSamplesSize() + centroidSize();
}


inline CorrelatedData::CorrelatedData(unsigned nrBaselines, unsigned nrChannels)
:
  StreamableData(true),
  itsNrBaselines(nrBaselines),
  itsNrChannels(nrChannels),
#ifdef HAVE_BGP
  itsAlignment(32)
#else 
  itsAlignment(512)
#endif
{
}

inline void CorrelatedData::allocate(Allocator &allocator)
{
  /// TODO Should this be aligned as well?? 
  visibilities.resize(boost::extents[itsNrBaselines][itsNrChannels][NR_POLARIZATIONS][NR_POLARIZATIONS], itsAlignment, allocator);
  nrValidSamples.resize(boost::extents[itsNrBaselines][itsNrChannels], itsAlignment, allocator);
  centroids.resize(itsNrBaselines, itsAlignment, allocator);
}

inline void CorrelatedData::readData(Stream *str)
{
  str->read(visibilities.origin(), visibilities.num_elements() * sizeof(fcomplex));
  str->read(nrValidSamples.origin(), nrValidSamples.num_elements() * sizeof(unsigned short));
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
  str->write(nrValidSamples.origin(), align(nrValidSamples.num_elements() * sizeof(unsigned short), itsAlignment));
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

inline StreamableData &CorrelatedData::operator += (const StreamableData &other_)
{
  const CorrelatedData &other = dynamic_cast<const CorrelatedData&>(other_);

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
    unsigned short       *dst  = nrValidSamples.origin();
    const unsigned short *src  = other.nrValidSamples.origin();
    unsigned		 count = nrValidSamples.num_elements();

    for (unsigned i = 0; i < count; i ++)
      dst[i] += src[i];
  }

  return *this;
}

} // namespace RTCP
} // namespace LOFAR

#endif
