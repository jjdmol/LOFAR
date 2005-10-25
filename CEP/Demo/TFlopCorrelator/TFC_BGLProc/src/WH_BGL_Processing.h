//#  WH_BGL_Processing.h: 256 kHz polyphase filter and correlator
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#

#ifndef LOFAR_TFLOPCORRELATOR_WH_BGL_PROCESSING_H
#define LOFAR_TFLOPCORRELATOR_WH_BGL_PROCESSING_H

#include <fftw.h>

#include <tinyCEP/WorkHolder.h>
#include <TFC_Interface/TFC_Config.h>


namespace LOFAR
{
  class FIR {
  public:
    FIR();

    inline fcomplex processNextSample(fcomplex sample, const float weights[NR_TAPS])
    {
      fcomplex sum = sample * weights[0];
      itsDelayLine[0] = sample;

      for (int tap = NR_TAPS; -- tap > 0;) {
	sum += weights[tap] * itsDelayLine[tap];
	itsDelayLine[tap] = itsDelayLine[tap - 1];
      }

      return sum;
    }

  //private:
    fcomplex itsDelayLine[NR_TAPS] __attribute__ ((aligned(16)));
    static const float weights[NR_SUB_CHANNELS][NR_TAPS];
  };


  class WH_BGL_Processing: public WorkHolder {
    
  public:
    explicit WH_BGL_Processing (const string& name, const short subBandID);
    virtual ~WH_BGL_Processing();

    static WorkHolder* construct (const string& name, const short subBandID); 
    virtual WH_BGL_Processing* make (const string& name);

    virtual void preprocess();
    virtual void process();
    virtual void dump() const;
    virtual void postprocess();

  private:
    /// forbid copy constructor
    WH_BGL_Processing(const WH_BGL_Processing&);
    
    /// forbid assignment
    WH_BGL_Processing& operator= (const WH_BGL_Processing&);

    /// the two phases: first PPF, then correlation
    void doPPF();
    void doCorrelate();

    /// FIR Filter variables
    short	 itsSubBandID;
    fftw_plan	 itsFFTWPlan;
    FIR		 (*itsFIRs)[NR_STATIONS][NR_POLARIZATIONS][NR_SUB_CHANNELS];

    /// CorrCube
    static fcomplex	 itsCorrCube[NR_SUB_CHANNELS][NR_STATIONS][NR_SAMPLES_PER_INTEGRATION][NR_POLARIZATIONS] __attribute__ ((aligned(32)));
  };

} // namespace LOFAR

#endif
