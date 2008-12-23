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
    CorrelatedData(unsigned nrBaselines, unsigned nrChannels, Allocator &allocator = heapAllocator);

    static size_t requiredSize(unsigned nrBaselines, unsigned nrChannels);

    virtual StreamableData &operator += (const StreamableData &);

    MultiDimArray<fcomplex, 4>	visibilities; //[nrBaselines][nrChannels][NR_POLARIZATIONS][NR_POLARIZATIONS]
    Matrix<unsigned short>	nrValidSamples; //[nrBaselines][nrChannels]
    Vector<float>		centroids; //[nrBaselines]

  protected:
    virtual void readData(Stream *);
    virtual void writeData(Stream *) const;

  private:
    void			checkEndianness();

    static size_t visibilitiesSize(unsigned nrBaselines, unsigned nrChannels);
    static size_t nrValidSamplesSize(unsigned nrBaselines, unsigned nrChannels);
    static size_t centroidSize(unsigned nrBaselines);
};


inline size_t CorrelatedData::visibilitiesSize(unsigned nrBaselines, unsigned nrChannels)
{
  return align(sizeof(fcomplex) * nrBaselines * nrChannels * NR_POLARIZATIONS * NR_POLARIZATIONS, 32);
}


inline size_t CorrelatedData::nrValidSamplesSize(unsigned nrBaselines, unsigned nrChannels)
{
  return align(sizeof(unsigned short) * nrBaselines * nrChannels, 32);
}


inline size_t CorrelatedData::centroidSize(unsigned nrBaselines)
{
  return align(sizeof(float) * nrBaselines, 32);
}


inline size_t CorrelatedData::requiredSize(unsigned nrBaselines, unsigned nrChannels)
{
  return visibilitiesSize(nrBaselines, nrChannels) + nrValidSamplesSize(nrBaselines, nrChannels) + centroidSize(nrBaselines);
}


inline CorrelatedData::CorrelatedData(unsigned nrBaselines, unsigned nrChannels, Allocator &allocator)
:
  StreamableData(true),
  visibilities(boost::extents[nrBaselines][nrChannels][NR_POLARIZATIONS][NR_POLARIZATIONS], 32, allocator),
  nrValidSamples(boost::extents[nrBaselines][nrChannels], 32, allocator),
  centroids(nrBaselines, 32, allocator)
{
}


inline void CorrelatedData::readData(Stream *str)
{
  str->read(visibilities.origin(), visibilities.num_elements() * sizeof(fcomplex));
  str->read(nrValidSamples.origin(), nrValidSamples.num_elements() * sizeof(unsigned short));
  //str->read(&centroids[0], centroids.size() * sizeof(float));

  checkEndianness();
}


inline void CorrelatedData::writeData(Stream *str) const
{
#if !defined WORDS_BIGENDIAN
  THROW(AssertError, "not implemented: think about endianness");
#endif

  str->write(visibilities.origin(), visibilities.num_elements() * sizeof(fcomplex));
  str->write(nrValidSamples.origin(), nrValidSamples.num_elements() * sizeof(unsigned short));
  //str->write(&centroids[0], centroids.size() * sizeof(float));
}


inline void CorrelatedData::checkEndianness()
{
#if !defined WORDS_BIGENDIAN
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
