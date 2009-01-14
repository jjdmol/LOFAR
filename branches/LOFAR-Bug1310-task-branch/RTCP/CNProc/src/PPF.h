#ifndef LOFAR_BGLPROC_PPF_H
#define LOFAR_BGLPROC_PPF_H

#if 0 || !(defined HAVE_BGL || defined HAVE_BGP)
#define PPF_C_IMPLEMENTATION
#endif


#include <FIR.h>
#include <TransposedData.h>
#include <Interface/FilteredData.h>
#include <Interface/AlignedStdAllocator.h>

#include <boost/multi_array.hpp>

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

template <typename SAMPLE_TYPE> class PPF
{
  public:
    PPF(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration, double channelBandwidth, bool delayCompensation);
    ~PPF();

    void computeFlags(unsigned stat, const TransposedData<SAMPLE_TYPE> *, FilteredData *);
    void filter(unsigned stat, double centerFrequency, const TransposedData<SAMPLE_TYPE> *, FilteredData *);

  private:
    void init_fft(), destroy_fft();

#if !defined PPF_C_IMPLEMENTATION
    void initConstantTable();
#endif

#if defined PPF_C_IMPLEMENTATION
    fcomplex phaseShift(unsigned time, unsigned chan, double baseFrequency, double delayAtBegin, double delayAfterEnd) const;
#else
    void     computePhaseShifts(struct phase_shift phaseShifts[/*itsNrSamplesPerIntegration*/], double delayAtBegin, double delayAfterEnd, double baseFrequency) const;
#endif

    unsigned itsNrStations, itsNrSamplesPerIntegration;
    unsigned itsNrChannels, itsLogNrChannels;
    double   itsChannelBandwidth;
    bool     itsDelayCompensation;

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
