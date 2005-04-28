//#  WH_SubBand.h: 256 kHz polyphase filter
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

#ifndef LOFAR_TFLOPCORRELATOR_WHSUBBAND_H
#define LOFAR_TFLOPCORRELATOR_WHSUBBAND_H

#include <tinyCEP/WorkHolder.h>

namespace LOFAR
{
  class WH_SubBand: public WorkHolder {
    
  public:
    typedef u16complex FilterType;

    explicit WH_SubBand (const string& name,
			 const short SubBandID); // subBand identification for this filter

    virtual ~WH_SubBand();

    static WorkHolder* construct (const string& name, const short SubBandID); 
    virtual WH_SubBand* make (const string& name);

    virtual void process();
    virtual void dump();
    
  private:
    /// forbid copy constructor
    WH_SubBand(const WH_SubBand&);
    
    /// forbid assignment
    WH_SubBand& operator= (const WH_SubBand&);

    short itsNtaps;
    short itsNcoeff;
    short itsFFTLen;
    short itsSBID; // subBandID
    short itsCpF;
    
    FilterType* delayPtr;
    FilterType* delayLine;

    FilterType* coeffPtr;
    FilterType* inputPtr;

    FilterType acc;

    void adjustDelayPtr();
  };

} // namespace LOFAR

#endif
