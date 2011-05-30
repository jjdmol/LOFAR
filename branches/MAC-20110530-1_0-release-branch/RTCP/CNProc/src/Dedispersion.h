#ifndef LOFAR_CNPROC_DEDISPERSION_H
#define LOFAR_CNPROC_DEDISPERSION_H


#include <Common/lofar_complex.h>
#include <Interface/BeamFormedData.h>
#include <Interface/FilteredData.h>
#include <Interface/MultiDimArray.h>
#include <Interface/Parset.h>
#include <Interface/SmartPtr.h>

#include <vector>


#if defined HAVE_FFTW3
#include <fftw3.h>
#elif defined HAVE_FFTW2
#include <fftw.h>
#else
#error Should have FFTW3 or FFTW2 installed
#endif



namespace LOFAR {
namespace RTCP {


class Dedispersion
{
  protected:
	 Dedispersion(const Parset &, const std::vector<unsigned> &subbandIndices);

  public:
	 ~Dedispersion();

  protected:
    void initFFT(fcomplex *data);
    void forwardFFT(fcomplex *data);
    void backwardFFT(fcomplex *data);

    const unsigned itsNrChannels, itsNrSamplesPerIntegration, itsFFTsize;
    const double   itsChannelBandwidth;

    Matrix<fcomplex> itsFFTedBuffer;

#if defined HAVE_FFTW3
    fftwf_plan itsFFTWforwardPlan, itsFFTWbackwardPlan;
#elif defined HAVE_FFTW2
    fftw_plan  itsFFTWforwardPlan, itsFFTWbackwardPlan;
#endif

    void initChirp(const Parset &, const std::vector<unsigned> &subbandIndices);
    void applyChirp(unsigned subbandIndex, unsigned channel);

    std::vector<SmartPtr<Matrix<fcomplex> > > itsChirp; // (*[subbandIndex])[channel][time]
};


class DedispersionBeforeBeamForming : public Dedispersion
{
  public:
    DedispersionBeforeBeamForming(const Parset &, FilteredData *, const std::vector<unsigned> &subbandIndices);

    void dedisperse(FilteredData *, unsigned subbandIndex);

  private:
    const unsigned itsNrStations;
};


class DedispersionAfterBeamForming : public Dedispersion
{
  public:
    DedispersionAfterBeamForming(const Parset &, BeamFormedData *, const std::vector<unsigned> &subbandIndex);

    void dedisperse(BeamFormedData *, unsigned subbandIndex, unsigned beam);

  private:
    const unsigned itsNrBeams;
};




} // namespace RTCP
} // namespace LOFAR

#endif
