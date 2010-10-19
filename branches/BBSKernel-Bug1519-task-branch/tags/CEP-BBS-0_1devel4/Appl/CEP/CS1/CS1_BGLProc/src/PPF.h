#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_PPF_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_PPF_H

#if 0 || !defined HAVE_BGL
#define PPF_C_IMPLEMENTATION
#endif


#include <FIR.h>
#include <TransposedData.h>
#include <FilteredData.h>
#include <CacheAlignedAllocator.h>

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
namespace CS1 {

class PPF
{
  public:
    PPF(unsigned nrStations, unsigned nrSamplesPerIntegration, double channelBandwidth, bool delayCompensation);
    ~PPF();

    void computeFlags(const TransposedData *, FilteredData *);
    void filter(double centerFrequency, const TransposedData *, FilteredData *);

  private:
    void init_fft(), destroy_fft();

#if defined PPF_C_IMPLEMENTATION
    fcomplex phaseShift(unsigned time, unsigned chan, double baseFrequency, const TransposedData::DelayIntervalType &delay) const;
#else
    void     computePhaseShifts(struct phase_shift phaseShifts[/*itsNrSamplesPerIntegration*/], const TransposedData::DelayIntervalType &delay, double baseFrequency) const;
#endif

    unsigned itsNrStations, itsNrSamplesPerIntegration;
    double   itsChannelBandwidth;
    bool     itsDelayCompensation;

#if defined PPF_C_IMPLEMENTATION
    boost::multi_array<FIR, 3> itsFIRs; //[itsNrStations][NR_POLARIZATIONS][NR_SUBBAND_CHANNELS]
    boost::multi_array<fcomplex, 3> itsFFTinData; //[NR_TAPS - 1 + itsNrSamplesPerIntegration][NR_POLARIZATIONS][NR_SUBBAND_CHANNELS]
#else
    boost::multi_array<fcomplex, 2, CacheAlignedAllocator<fcomplex> > itsTmp; //[4][itsNrSamplesPerIntegration]
    boost::multi_array<fcomplex, 3, CacheAlignedAllocator<fcomplex> > itsFFTinData; //[itsNrSamplesPerIntegration][NR_POLARIZATIONS][NR_SUBBAND_CHANNELS + 4]
    boost::multi_array<fcomplex, 3, CacheAlignedAllocator<fcomplex> > itsFFToutData; //[2][NR_POLARIZATIONS][NR_SUBBAND_CHANNELS]
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

} // namespace CS1
} // namespace LOFAR

#endif
