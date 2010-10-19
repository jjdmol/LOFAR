//#  WH_BGL_Processing.h: polyphase filter and correlator
//#
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_WH_BGL_PROCESSING_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_WH_BGL_PROCESSING_H

#if 0 || !defined HAVE_BGL
#define C_IMPLEMENTATION
#endif

#if defined HAVE_FFTW3
#include <fftw3.h>
#elif defined HAVE_FFTW2
#include <fftw.h>
#else
#error Should have FFTW3 or FFTW2 installed
#endif

#include <tinyCEP/WorkHolder.h>
#include <CS1_Interface/bitset.h>
#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/DH_Subband.h>
//#include <CS1_Interface/DH_RFI_Mitigation.h>
#include <CS1_Interface/DH_Visibilities.h>
#include <CS1_Interface/CS1_Parset.h>

#include <boost/multi_array.hpp>
#include <malloc.h>


#if defined HAVE_BGL
#define CACHE_LINE_SIZE	32
#define CACHE_ALIGNED	__attribute__ ((aligned(CACHE_LINE_SIZE)))
#else
#define CACHE_LINE_SIZE	16
#define CACHE_ALIGNED
#endif


namespace LOFAR {
namespace CS1 {

template <typename T> class CacheAlignedAllocator : public std::allocator<T>
{
  public:
    typedef typename std::allocator<T>::size_type size_type;
    typedef typename std::allocator<T>::pointer pointer;
    typedef typename std::allocator<T>::const_pointer const_pointer;

    pointer allocate(size_type size, const_pointer /*hint*/ = 0)
    {
      return static_cast<pointer>(memalign(CACHE_LINE_SIZE, size * sizeof(T)));
    }

    void deallocate(pointer ptr, size_type /*size*/)
    {
      free(ptr);
    }
};


class FIR {
  public:
#if defined C_IMPLEMENTATION
    FIR();

    fcomplex processNextSample(fcomplex sample, const float weights[NR_TAPS]);

    fcomplex itsDelayLine[NR_TAPS];
#endif

    static const float weights[NR_SUBBAND_CHANNELS][NR_TAPS];
};

class WH_BGL_Processing: public WorkHolder {
  public:
    enum inDataHolders {
      SUBBAND_CHANNEL,
//    RFI_MITIGATION_CHANNEL,
      NR_IN_CHANNELS
    };

    enum outDataHolders {
      VISIBILITIES_CHANNEL,
      NR_OUT_CHANNELS
    };

    explicit WH_BGL_Processing(const string &name, unsigned coreNumber, CS1_Parset *ps);
    virtual ~WH_BGL_Processing();

    static WorkHolder *construct(const string &name, unsigned coreNumber, CS1_Parset *ps);
    virtual WH_BGL_Processing *make(const string &name);

    virtual void preprocess();
    virtual void process();
    virtual void dump() const;
    virtual void postprocess();

    DH_Subband *get_DH_Subband() {
      return dynamic_cast<DH_Subband *>(getDataManager().getInHolder(SUBBAND_CHANNEL));
    }

#if 0
    DH_RFI_Mitigation *get_DH_RFI_Mitigation() {
      return dynamic_cast<DH_RFI_Mitigation *>(getDataManager().getInHolder(RFI_MITIGATION_CHANNEL));
    }
#endif

    DH_Visibilities *get_DH_Visibilities() {
      return dynamic_cast<DH_Visibilities *>(getDataManager().getOutHolder(VISIBILITIES_CHANNEL));
    }

  private:
    /// forbid copy constructor
    WH_BGL_Processing(const WH_BGL_Processing&);

    /// forbid assignment
    WH_BGL_Processing &operator = (const WH_BGL_Processing&);

    void doPPF(double baseFrequency), bypassPPF();
    void computeFlags();
    void doCorrelate();

#if defined C_IMPLEMENTATION
    fcomplex phaseShift(unsigned time, unsigned chan, double baseFrequency, const DH_Subband::DelayIntervalType &delay) const;
#else
    void computePhaseShifts(struct phase_shift phaseShifts[/*itsNrSamplesPerIntegration*/], const DH_Subband::DelayIntervalType &delay, double baseFrequency) const;
#endif

    /// FIR Filter variables
#if defined HAVE_FFTW3
    fftwf_plan	    itsFFTWPlan;
#elif defined HAVE_FFTW2
    fftw_plan	    itsFFTWPlan;
#endif

    vector<double>  itsCenterFrequencies;
    double	    itsChannelBandwidth;

    unsigned	    itsNrStations, itsNrBaselines;
    unsigned	    itsNrSamplesPerIntegration;

    CS1_Parset     *itsCS1PS;
    const unsigned  itsCoreNumber;
    unsigned        itsFirstSubband, itsCurrentSubband, itsLastSubband, itsSubbandIncrement;
    bool	    itsInputConnected;
    bool	    itsDelayCompensation;

#if defined C_IMPLEMENTATION
    typedef boost::multi_array<FIR, 3> itsFIRsType;
    itsFIRsType     *itsFIRs;
    //[itsNrStations][NR_POLARIZATIONS][NR_SUBBAND_CHANNELS]

    typedef boost::multi_array<fcomplex, 3> itsFFTdataType;
    itsFFTdataType  *itsFFTinData;
    //[NR_TAPS - 1 + itsNrSamplesPerIntegration][NR_POLARIZATIONS][NR_SUBBAND_CHANNELS]
#else
    typedef boost::multi_array<fcomplex, 2, CacheAlignedAllocator<fcomplex> > itsTmpType;
    itsTmpType	    *itsTmp; //[4][itsNrSamplesPerIntegration]

    typedef boost::multi_array<fcomplex, 3, CacheAlignedAllocator<fcomplex> > itsFFTdataType;
    itsFFTdataType  *itsFFTinData;
    //[itsNrSamplesPerIntegration][NR_POLARIZATIONS][NR_SUBBAND_CHANNELS + 4]

    itsFFTdataType  *itsFFToutData;
    //[2][NR_POLARIZATIONS][NR_SUBBAND_CHANNELS]
#endif

    // The "| 2" significantly improves transpose speeds for particular numbers
    // of stations due to cache conflict effects.  The extra memory is not used.
    typedef boost::multi_array<fcomplex, 4, CacheAlignedAllocator<fcomplex> > itsSamplesType;
    itsSamplesType  *itsSamples;
    //[NR_SUBBAND_CHANNELS][itsNrStations][itsNrSamplesPerIntegration | 2][NR_POLARIZATIONS] CACHE_ALIGNED

    SparseSet<unsigned> *itsFlags; //[itsNrStations]
    unsigned	    *itsNrValidSamples; //[itsNrBaselines]
    float	    *itsCorrelationWeights; //[itsNrSamplesPerIntegration + 1]

    bitset<NR_SUBBAND_CHANNELS> *itsRFIflags; //[itsNrStations]
};

} // namespace CS1
} // namespace LOFAR

#endif
