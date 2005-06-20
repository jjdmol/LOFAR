//#  WH_SubBand.h: 256 kHz polyphase filter
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
#include <TFC_Interface/DH_SubBand.h>

namespace LOFAR
{
  // define a structure to contain all relevant data for one filter
  typedef struct {
    short filter_id;
    fcomplex*   delayLine;
    float*      filterTaps;
  } filterBox;


  class WH_SubBand: public WorkHolder {
    
  public:

    // Note that we may need to change FilterType to dcomplex 
    // because of the FFT. The current cast to fftw_complex might
    // not be correct and may even cause a segfault.
    typedef fcomplex FilterType;

    explicit WH_SubBand (const string& name,
			 const short SubBandID); // subBand identification for this filter

    virtual ~WH_SubBand();

    static WorkHolder* construct (const string& name, const short SubBandID); 
    virtual WH_SubBand* make (const string& name);

    virtual void preprocess();
    virtual void process();
    virtual void dump();
    virtual void postprocess();

  private:
    /// forbid copy constructor
    WH_SubBand(const WH_SubBand&);
    
    /// forbid assignment
    WH_SubBand& operator= (const WH_SubBand&);

    /// FIR Filter variables
    short itsNFilters;
    short itsNtaps;
    short itsNStations;
    short itsNTimes;
    short itsNPol;
    short itsSBID; // subBandID
    short itsCpF;
    short itsFFTs;
    
    DH_SubBand::BufferType* delayPtr;
    DH_SubBand::BufferType* delayLine;

    filterBox* filterData;

    float**      coeffPtr;
    FilterType*  inputPtr;

    FilterType*  fft_in;
    FilterType*  fft_out;

    void adjustDelayPtr(FilterType* dLine);
  };

} // namespace LOFAR

#endif
