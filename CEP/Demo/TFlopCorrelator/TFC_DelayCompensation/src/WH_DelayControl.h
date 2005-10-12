//#  WH_DelayControl.h: WorkHolder to distribute delays to all 
//#                     RSPInputs 
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//#  $Id$
//#
/////////////////////////////////////////////////////////////////////

#ifndef TFC_DELAYCOMPENSATION_WH_DELAYCONTROL_H
#define TFC_DELAYCOMPENSATION_WH_DELAYCONTROL_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file WH_DelayControl.h
// WorkHolder to distribute delays to all RSPInputs

//# Includes
#include <tinyCEP/WorkHolder.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
  class WH_DelayControl: public WorkHolder
  {
  public:

    explicit WH_DelayControl(const string& name,
			     const int nRSPInputs,
			     const int nrChannels,
			     const int delay);
    virtual ~WH_DelayControl();
    static WorkHolder* construct(const string& name, const int nRSPInputs,
				 const int nrChannels, const int delay);
    virtual WH_DelayControl* make(const string& name);

    virtual void process();

    void setDelay(const int delay);
    int getDelay();
    
  private:
    /// forbid copy constructor
    WH_DelayControl (const WH_DelayControl&);
    /// forbid assignment
    WH_DelayControl& operator= (const WH_DelayControl&);
   
    int  itsNrRSPInputs;
    int  itsNrChannels;
    int  itsAbsDelay;    // Absolute delay value
    int  itsDelayDiff;   // Change in the delay for a station
  };

inline int WH_DelayControl::getDelay()
  {  return itsAbsDelay; }

  // @}
} // namespace LOFAR

#endif
