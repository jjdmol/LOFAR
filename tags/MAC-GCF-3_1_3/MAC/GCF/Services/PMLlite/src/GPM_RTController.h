//#  GPM_RTController.h: singleton class; bridge between controller application 
//#                    and Supervisory Server
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
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

#ifndef GPM_RTCONTROLLER_H
#define GPM_RTCONTROLLER_H

#include <GCF/GCF_Task.h>
#include <GCF/GCF_Port.h>
#include "GPM_RTDefines.h"
#include <GPI_Defines.h>
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>

/**
   This singleton class forms the bridge between the PMLlite API classes and the SS. 
   It is a hidden task with its own state machine in each Application, which 
   wants to be part of the MAC subsystem of LOFAR. It will be created at the 
   moment a service of PMLlite will be requested by the Application (like load 
   property set or set property).
*/

class GCFPValue;
class GCFEvent;
class GCFPortInterface;
class GCFRTMyPropertySet;

class GPMRTController : public GCFTask
{
  public:
    ~GPMRTController ();
    static GPMRTController* instance();

  public: // member functions
    TPMResult registerScope (GCFRTMyPropertySet& propSet);
    TPMResult unregisterScope (GCFRTMyPropertySet& propSet, 
                               bool permanent = false);
       
    void propertiesLinked (const string& scope, 
                           list<string>& propsToSubscribe, 
                           TPIResult result);
    void propertiesUnlinked (const string& scope, 
                           list<string>& propsToSubscribe, 
                           TPIResult result);
    void valueSet(const string& propName, const GCFPValue& value);
  
  private:
    GPMRTController ();
    static GPMRTController* _pInstance;

  private: // state methods
    GCFEvent::TResult initial   (GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult connected (GCFEvent& e, GCFPortInterface& p);
        
  private: // helper methods
    void sendMsgToPI (GCFEvent& e);

  private: // data members        
    GCFPort                       _supervisoryServer;
    bool                          _isBusy;
    bool                          _preparing;
    unsigned int                  _counter;
    typedef map<string, GCFRTMyPropertySet*>  TPropertySets;
    TPropertySets                 _propertySets;
};
#endif
