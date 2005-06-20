//#  WH_FFT.h: 256 kHz polyphase filter
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

#ifndef LOFAR_TFLOPCORRELATOR_WHFFT_H
#define LOFAR_TFLOPCORRELATOR_WHFFT_H

#include <fftw.h>

#include <tinyCEP/WorkHolder.h>
#include <TFC_Interface/DH_PPF.h>
#include <TFC_Interface/DH_SubBand.h>

namespace LOFAR
{
  class WH_FFT: public WorkHolder {
    
  public:

    typedef fcomplex FilterType;

    explicit WH_FFT (const string& name); // subBand identification for this filter

    virtual ~WH_FFT();

    static WorkHolder* construct (const string& name); 
    virtual WH_FFT* make (const string& name);

    virtual void preprocess();
    virtual void process();
    virtual void dump();
    virtual void postprocess();

  private:
    /// forbid copy constructor
    WH_FFT(const WH_FFT&);
    
    /// forbid assignment
    WH_FFT& operator= (const WH_FFT&);

    // FFT variables
    short itsNtaps;       // #points of the FFT
    short itsNSamples;    // #samples per input (normally itsNtaps/itsInputs)
    short itsCpF;         // #correlators per Filter
    short itsInputs;      
    short itsSBID;

    FilterType*  fft_in;
    FilterType*  fft_out;

    /// FFTW variables
    fftw_direction itsFFTDirection;
    fftw_plan      itsFFTPlan;

  };

} // namespace LOFAR

#endif
