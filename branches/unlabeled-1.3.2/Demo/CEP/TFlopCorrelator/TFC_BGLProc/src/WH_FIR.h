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
    DH_FIR::BufferType WH_FIR::FIR_basic(DH_FIR::BufferType &sample,
				 short station,
				 short bank); 
    DH_FIR::BufferType WH_FIR::FIR_circular(DH_FIR::BufferType &sample,
				    short station,
				    short bank); 
    
    /// forbid copy constructor
    WH_FIR(const WH_FIR&);
    
    /// forbid assignment
    WH_FIR& operator= (const WH_FIR&);

    /// FIR Filter variables
    short itsSBID; // subBandID
    short itsNFilterBanks; // =FFT length
    short itsNtaps;
    short itsNStations;
    short itsNTimes;
    short itsNPol;
    short itsFFTs;
    short itsOutTime; // number of output time slices (= #FFT calls) per input 
    
    // attributes for the implementation of the FIR filters
    DH_FIR::BufferType ***itsDelayLine; // will be addresed as [station][bank][tab]
    float               **itsCoeff;     // will be addresed as [bank][tab]
    short               **itsBankState  // will be addresed as [station][bank];

    //OLD    DH_FIR::BufferType* delayPtr;
    //OLD    DH_FIR::BufferType* delayLine;
    //OLD    filterBox* filterData;
    //OLD    float**      coeffPtr;
    //OLD    FilterType*  inputPtr;
    //OLD    FilterType*  fft_in;
    //OLD    FilterType*  fft_out;

      //OLD    void adjustDelayPtr(FilterType* dLine);

  };

} // namespace LOFAR

#endif
