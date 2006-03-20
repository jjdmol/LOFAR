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

#define DELAY_COMPENSATION

#include <fftw.h>
#include <bitset>

#include <tinyCEP/WorkHolder.h>
#include <CS1_Interface/bitset.h>
#include <CS1_Interface/CS1_Config.h>

#include <CS1_Interface/DH_Subband.h>
#include <CS1_Interface/DH_RFI_Mitigation.h>
#include <CS1_Interface/DH_Visibilities.h>


#if defined HAVE_BGL
#define CACHE_LINE_SIZE	32
#define CACHE_ALIGNED	__attribute__ ((aligned(CACHE_LINE_SIZE)))
#else
#define CACHE_ALIGNED
#endif


namespace LOFAR
{

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
      RFI_MITIGATION_CHANNEL,
      NR_IN_CHANNELS
    };

    enum outDataHolders {
      VISIBILITIES_CHANNEL,
      NR_OUT_CHANNELS
    };

    explicit WH_BGL_Processing(const string &name, double baseFrequency, const ACC::APS::ParameterSet &ps);
    virtual ~WH_BGL_Processing();

    static WorkHolder *construct(const string &name, double baseFrequency, const ACC::APS::ParameterSet &);
    virtual WH_BGL_Processing *make(const string &name);

    virtual void preprocess();
    virtual void process();
    virtual void dump() const;
    virtual void postprocess();

    DH_Subband *get_DH_Subband() {
      return dynamic_cast<DH_Subband *>(getDataManager().getInHolder(SUBBAND_CHANNEL));
    }

    DH_RFI_Mitigation *get_DH_RFI_Mitigation() {
      return dynamic_cast<DH_RFI_Mitigation *>(getDataManager().getInHolder(RFI_MITIGATION_CHANNEL));
    }

    DH_Visibilities *get_DH_Visibilities() {
      return dynamic_cast<DH_Visibilities *>(getDataManager().getOutHolder(VISIBILITIES_CHANNEL));
    }

  private:
    /// forbid copy constructor
    WH_BGL_Processing(const WH_BGL_Processing&);
    
    /// forbid assignment
    WH_BGL_Processing& operator= (const WH_BGL_Processing&);

    void doPPF(), bypassPPF();
    void computeFlags();
    void doCorrelate();

#if defined DELAY_COMPENSATION
    fcomplex phaseShift(int time, int chan, const DH_Subband::DelayIntervalType &delay) const;
    void computePhaseShifts(struct phase_shift phaseShifts[NR_SAMPLES_PER_INTEGRATION], const DH_Subband::DelayIntervalType &delay) const;
#endif

    /// FIR Filter variables
    fftw_plan	    itsFFTWPlan;
    double	    itsBaseFrequency;
    const ACC::APS::ParameterSet &itsPS;
    static FIR	    itsFIRs[NR_STATIONS][NR_POLARIZATIONS][NR_SUBBAND_CHANNELS] CACHE_ALIGNED;

    static fcomplex samples[NR_SUBBAND_CHANNELS][NR_STATIONS][NR_SAMPLES_PER_INTEGRATION][NR_POLARIZATIONS] CACHE_ALIGNED;
    static LOFAR::bitset<NR_SAMPLES_PER_INTEGRATION> flags[NR_STATIONS] CACHE_ALIGNED;
    static unsigned itsNrValidSamples[NR_BASELINES] CACHE_ALIGNED;
    static float    correlationWeights[NR_SAMPLES_PER_INTEGRATION + 1] CACHE_ALIGNED;
    static float    thresholds[NR_BASELINES][NR_SUBBAND_CHANNELS];
};

} // namespace LOFAR

#endif
