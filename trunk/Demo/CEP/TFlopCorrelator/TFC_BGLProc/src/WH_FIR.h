//#  WH_FIR.h: 256 kHz polyphase filter
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

#ifndef LOFAR_TFLOPCORRELATOR_WHSUBBAND_H
#define LOFAR_TFLOPCORRELATOR_WHSUBBAND_H

#include <fftw.h>

#include <tinyCEP/WorkHolder.h>
#include <TFC_Interface/DH_FIR.h>

namespace LOFAR
{
  // define a structure to contain all relevant data for one filter
  typedef struct {
    short filter_id;
    fcomplex*   delayLine;
    float*      filterTaps;
  } filterBox;


  class WH_FIR: public WorkHolder {
    
  public:

    // Note that we may need to change FilterType to dcomplex 
    // because of the FFT. The current cast to fftw_complex might
    // not be correct and may even cause a segfault.
    typedef fcomplex FilterType;

    explicit WH_FIR (const string& name,
			 const short FIRID); // subBand identification for this filter

    virtual ~WH_FIR();

    static WorkHolder* construct (const string& name, const short FIRID); 
    virtual WH_FIR* make (const string& name);

    virtual void preprocess();
    virtual void process();
    virtual void dump() const;
    virtual void postprocess();

  private:
    /// forbid copy constructor
    WH_FIR(const WH_FIR&);
    
    /// forbid assignment
    WH_FIR& operator= (const WH_FIR&);

    /// FIR Filter variables
    short itsSBID; // subBandID
    short itsNFilters;
    short itsNtaps;
    short itsNStations;
    short itsNTimes;
    short itsNPol;
    short itsFFTs;
    
    DH_FIR::BufferType* delayPtr;
    DH_FIR::BufferType* delayLine;

    filterBox* filterData;

    float**      coeffPtr;
    FilterType*  inputPtr;

    FilterType*  fft_in;
    FilterType*  fft_out;

    void adjustDelayPtr(FilterType* dLine);
  };

} // namespace LOFAR

#endif
