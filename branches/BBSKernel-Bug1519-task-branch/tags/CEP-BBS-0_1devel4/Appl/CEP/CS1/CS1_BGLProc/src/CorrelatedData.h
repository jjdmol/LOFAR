#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_CORRELATED_DATA_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_CORRELATED_DATA_H

#include <Common/lofar_complex.h>
#include <CS1_Interface/CS1_Config.h>
#include <Transport/TH_Null.h>

#include <Allocator.h>
#include <TH_ZoidClient.h>

#include <boost/multi_array.hpp>


namespace LOFAR {
namespace CS1 {

class CorrelatedData
{
  public:
    CorrelatedData(const Heap &heap, unsigned nrBaselines);
    ~CorrelatedData();

    static size_t requiredSize(unsigned nrBaselines);
    void	  write(TransportHolder *) /*const*/;

  private:
    Overlay	  overlay;
    unsigned	  itsNrBaselines;

  public:
    boost::multi_array_ref<fcomplex, 4>       visibilities; //[itsNrBaselines][NR_SUBBAND_CHANNELS][NR_POLARIZATIONS][NR_POLARIZATIONS]
    boost::multi_array_ref<unsigned short, 2> nrValidSamples; //[itsNrBaselines][NR_SUBBAND_CHANNELS]
    float				      *centroids; //[itsNrBaselines]

  private:
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
  size_t unalignedSize = sizeof(float) * nrBaselines;
  return (unalignedSize + 31) & ~31;
}


inline size_t CorrelatedData::requiredSize(unsigned nrBaselines)
{
  return visibilitiesSize(nrBaselines) + nrValidSamplesSize(nrBaselines) + centroidSize(nrBaselines);
}


inline CorrelatedData::CorrelatedData(const Heap &heap, unsigned nrBaselines)
:
  overlay(heap),
  itsNrBaselines(nrBaselines),
  visibilities(static_cast<fcomplex *>(overlay.allocate(visibilitiesSize(nrBaselines), 32)), boost::extents[nrBaselines][NR_SUBBAND_CHANNELS][NR_POLARIZATIONS][NR_POLARIZATIONS]),
  nrValidSamples(static_cast<unsigned short *>(overlay.allocate(nrValidSamplesSize(nrBaselines), 32)), boost::extents[nrBaselines][NR_SUBBAND_CHANNELS]),
  centroids(static_cast<float *>(overlay.allocate(centroidSize(nrBaselines), 32)))
{
}


inline CorrelatedData::~CorrelatedData()
{
  overlay.deallocate(visibilities.origin());
  overlay.deallocate(nrValidSamples.origin());
  overlay.deallocate(centroids);
}


inline void CorrelatedData::write(TransportHolder *th) /*const*/
{
  th->sendBlocking(visibilities.origin(), visibilities.num_elements() * sizeof(fcomplex), 0, 0);
  th->sendBlocking(nrValidSamples.origin(), nrValidSamples.num_elements() * sizeof(unsigned short), 0, 0);
  //th->sendBlocking(centroids, itsNrBaselines * sizeof(float), 0, 0);
}



} // namespace CS1
} // namespace LOFAR

#endif
