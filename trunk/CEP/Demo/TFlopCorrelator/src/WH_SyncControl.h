//#  WH_SyncControl.h: 
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//#  $Id$
//#
/////////////////////////////////////////////////////////////////////

#ifndef TFLOPCORRELATOR_WH_SYNCCONTROL_H
#define TFLOPCORRELATOR_WH_SYNCCONTROL_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file filename.h
// one line description.

//# Includes
#include <tinyCEP/WorkHolder.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
  class WH_SyncControl: public WorkHolder
  {
  public:

    explicit WH_SyncControl(const string& name);
    virtual ~WH_SyncControl();
    static WorkHolder* construct(const string& name);
    virtual WH_SyncControl* make(const string& name);

    virtual void process();
  private:
    /// forbid copy constructor
    WH_SyncControl (const WH_SyncControl&);
    /// forbid assignment
    WH_SyncControl& operator= (const WH_SyncControl&);
   

  };

  // @}
} // namespace LOFAR

#endif
