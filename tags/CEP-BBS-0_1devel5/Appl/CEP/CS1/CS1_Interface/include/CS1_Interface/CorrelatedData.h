#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_CORRELATED_DATA_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_CORRELATED_DATA_H

#include <Common/lofar_complex.h>
#include <Common/DataConvert.h>
#include <CS1_Interface/Align.h>
#include <CS1_Interface/Allocator.h>
#include <CS1_Interface/CS1_Config.h>
#include <Stream/Stream.h>

#include <boost/multi_array.hpp>
#include <stdexcept>


namespace LOFAR {
namespace CS1 {

class CorrelatedData
{
  public:
    CorrelatedData(const Arena &, unsigned nrBaselines);
    ~CorrelatedData();

    static size_t requiredSize(unsigned nrBaselines);
    void	  read(Stream *);
    void	  write(Stream *) const;

    CorrelatedData &operator += (const CorrelatedData &);

  private:
    SparseSetAllocator	  allocator;
    unsigned	  itsNrBaselines;

  public:
    boost::multi_array_ref<fcomplex, 4>       visibilities; //[itsNrBaselines][NR_SUBBAND_CHANNELS][NR_POLARIZATIONS][NR_POLARIZATIONS]
    boost::multi_array_ref<unsigned short, 2> nrValidSamples; //[itsNrBaselines][NR_SUBBAND_CHANNELS]
    float				      *centroids; //[itsNrBaselines]

  private:
    void	  checkEndianness();

    static size_t visibilitiesSize(unsigned nrBaselines);
    static size_t nrValidSamplesSize(unsigned nrBaselines);
    static size_t centroidSize(unsigned nrBaselines);
};


inline size_t CorrelatedData::visibilitiesSize(unsigned nrBaselines)
{
  return sizeof(fcomplex) * nrBaselines * NR_SUBBAND_CHANNELS * NR_POLARIZATIONS * NR_POLARIZATIONS;
}


inline size_t CorrelatedData::nrValidSamplesSize(unsigned nrBaselines)
{
  return sizeof(unsigned short) * nrBaselines * NR_SUBBAND_CHANNELS;
}


inline size_t CorrelatedData::centroidSize(unsigned nrBaselines)
{
  return align(sizeof(float) * nrBaselines, 32);
}


inline size_t CorrelatedData::requiredSize(unsigned nrBaselines)
{
  return visibilitiesSize(nrBaselines) + nrValidSamplesSize(nrBaselines) + centroidSize(nrBaselines);
}


inline CorrelatedData::CorrelatedData(const Arena &arena, unsigned nrBaselines)
:
  allocator(arena),
  itsNrBaselines(nrBaselines),
  visibilities(static_cast<fcomplex *>(allocator.allocate(visibilitiesSize(nrBaselines), 32)), boost::extents[nrBaselines][NR_SUBBAND_CHANNELS][NR_POLARIZATIONS][NR_POLARIZATIONS]),
  nrValidSamples(static_cast<unsigned short *>(allocator.allocate(nrValidSamplesSize(nrBaselines), 32)), boost::extents[nrBaselines][NR_SUBBAND_CHANNELS]),
  centroids(static_cast<float *>(allocator.allocate(centroidSize(nrBaselines), 32)))
{
}


inline CorrelatedData::~CorrelatedData()
{
  allocator.deallocate(visibilities.origin());
  allocator.deallocate(nrValidSamples.origin());
  allocator.deallocate(centroids);
}


inline void CorrelatedData::read(Stream *str)
{
  str->read(visibilities.origin(), visibilities.num_elements() * sizeof(fcomplex));
  str->read(nrValidSamples.origin(), nrValidSamples.num_elements() * sizeof(unsigned short));
  //str->read(centroids, itsNrBaselines * sizeof(float));

  checkEndianness();
}


inline void CorrelatedData::write(Stream *str) const
{
#if !defined WORDS_BIGENDIAN
  throw std::logic_error("not implemented: think about endianness");
#endif

  str->write(visibilities.origin(), visibilities.num_elements() * sizeof(fcomplex));
  str->write(nrValidSamples.origin(), nrValidSamples.num_elements() * sizeof(unsigned short));
  //str->write(centroids, itsNrBaselines * sizeof(float));
}


inline void CorrelatedData::checkEndianness()
{
#if !defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, visibilities.origin(), visibilities.num_elements());
  dataConvert(LittleEndian, nrValidSamples.origin(), nrValidSamples.num_elements());
  // dataConvert(LittleEndian, centroids, itsNrBaselines);
#endif
}


inline CorrelatedData &CorrelatedData::operator += (const CorrelatedData &other)
{
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

} // namespace CS1
} // namespace LOFAR

#endif
