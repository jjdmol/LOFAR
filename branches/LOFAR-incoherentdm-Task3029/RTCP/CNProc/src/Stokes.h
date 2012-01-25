#ifndef LOFAR_CNPROC_STOKES_H
#define LOFAR_CNPROC_STOKES_H

#include <Interface/FilteredData.h>
#include <Interface/StreamableData.h>
#include <Interface/BeamFormedData.h>
#include <Interface/MultiDimArray.h>
#include <Interface/Parset.h>
#include <Dedispersion.h>

#if 0 || !defined HAVE_BGP
#define STOKES_C_IMPLEMENTATION
#endif

namespace LOFAR {
namespace RTCP {


class Stokes
{
  public:
    static const float MAX_FLAGGED_PERCENTAGE = 0.9f;

    Stokes(unsigned nrChannels, unsigned nrSamples);

    template <bool ALLSTOKES> void calculateCoherent(const SampleData<> *sampleData, PreTransposeBeamFormedData *stokesData, unsigned inbeam, const StreamInfo &info);
    template <bool ALLSTOKES> void calculateIncoherent(const FilteredData *sampleData, PreTransposeBeamFormedData *stokesData, const std::vector<unsigned> &stationMapping, const StreamInfo &info, DedispersionBeforeBeamForming *dedispersion, unsigned subband, double dm, Allocator &allocator);

  private:
    const unsigned          itsNrChannels;
    const unsigned          itsNrSamples;
};

} // namespace RTCP
} // namespace LOFAR

#endif
