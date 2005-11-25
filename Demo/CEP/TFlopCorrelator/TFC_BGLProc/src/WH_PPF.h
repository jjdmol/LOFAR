//#  WH_PPF.h: 256 kHz polyphase filter
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#

#ifndef LOFAR_TFLOPCORRELATOR_WH_PPF_H
#define LOFAR_TFLOPCORRELATOR_WH_PPF_H

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


  class WH_PPF: public WorkHolder {
    
  public:
    explicit WH_PPF (const string& name, const short subBandID, const short max_element);
    virtual ~WH_PPF();

    static WorkHolder* construct (const string& name, const short subBandID, const short max_element); 
    virtual WH_PPF* make (const string& name);

    virtual void preprocess();
    virtual void process();
    virtual void dump() const;
    virtual void postprocess();

  private:
    /// forbid copy constructor
    WH_PPF(const WH_PPF&);
    
    /// forbid assignment
    WH_PPF& operator= (const WH_PPF&);

    void doPPF(), bypassPPF();

    /// FIR Filter variables
    short	 itsSubBandID;
    short        itsMaxElement;
    fftw_plan	 itsFFTWPlan;
    FIR		 (*itsFIRs)[NR_STATIONS][NR_SUB_CHANNELS][NR_POLARIZATIONS];
  };

} // namespace LOFAR

#endif
