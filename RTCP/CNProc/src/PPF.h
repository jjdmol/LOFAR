#ifndef LOFAR_CNPROC_PPF_H
#define LOFAR_CNPROC_PPF_H

#if 0 || !(defined HAVE_BGL || defined HAVE_BGP)
#define PPF_C_IMPLEMENTATION
#endif


#include <BandPass.h>
#include <FIR.h>
#include <TransposedData.h>
#include <Interface/FilteredData.h>
#include <Interface/AlignedStdAllocator.h>

#include <boost/multi_array.hpp>
#include <boost/noncopyable.hpp>

#if defined HAVE_BGL
#include <rts.h>
#endif

#if defined HAVE_FFTW3
#include <fftw3.h>
#elif defined HAVE_FFTW2
#include <fftw.h>
#else
#error Should have FFTW3 or FFTW2 installed
#endif


namespace LOFAR {
namespace RTCP {

template <typename SAMPLE_TYPE> class PPF: boost::noncopyable
{
  public:
    PPF(const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const double channelBandwidth, const bool delayCompensation, const bool verbose);
    ~PPF();

    void computeFlags(const unsigned stat, const TransposedData<SAMPLE_TYPE> *, FilteredData *);
    void filter(const unsigned stat, const double centerFrequency, const TransposedData<SAMPLE_TYPE> *, FilteredData *);

  private:
    void init_fft(), destroy_fft();

#if !defined PPF_C_IMPLEMENTATION
    void initConstantTable();
#endif

#if defined PPF_C_IMPLEMENTATION
    fcomplex phaseShift(const unsigned time, const unsigned chan, const double baseFrequency, const double delayAtBegin, const double delayAfterEnd) const;
#else
    void     computePhaseShifts(struct phase_shift phaseShifts[/*itsNrSamplesPerIntegration*/], const double delayAtBegin, const double delayAfterEnd, const double baseFrequency) const;
#endif

    const unsigned itsNrStations, itsNrSamplesPerIntegration;
    const unsigned itsNrChannels;
    unsigned       itsLogNrChannels;
    const double   itsChannelBandwidth;
    const bool     itsDelayCompensation;
    BandPass itsBandPass;

#if defined PPF_C_IMPLEMENTATION
    boost::multi_array<FIR, 3> itsFIRs; //[itsNrStations][NR_POLARIZATIONS][itsNrChannels]
    boost::multi_array<fcomplex, 3> itsFFTinData; //[NR_TAPS - 1 + itsNrSamplesPerIntegration][NR_POLARIZATIONS][itsNrChannels]
#else
    boost::multi_array<fcomplex, 2, AlignedStdAllocator<fcomplex, 32> > itsTmp; //[4][itsNrSamplesPerIntegration]
    boost::multi_array<fcomplex, 3, AlignedStdAllocator<fcomplex, 32> > itsFFTinData; //[itsNrSamplesPerIntegration][NR_POLARIZATIONS][itsNrChannels + 4]
    boost::multi_array<fcomplex, 3, AlignedStdAllocator<fcomplex, 32> > itsFFToutData; //[2][NR_POLARIZATIONS][itsNrChannels]
#endif

#if defined HAVE_FFTW3
    fftwf_plan itsFFTWPlan;
#elif defined HAVE_FFTW2
    fftw_plan  itsFFTWPlan;
#endif

#if defined HAVE_BGL && !defined PPF_C_IMPLEMENTATION
    BGL_Mutex  *mutex;
#endif
};

} // namespace RTCP
} // namespace LOFAR

#endif
