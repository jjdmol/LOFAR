//#  GPM_Controller.h: singleton class; bridge between controller application 
//#                    and Property Agent
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

#ifndef GPM_CONTROLLER_H
#define GPM_CONTROLLER_H

#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_Handler.h>
#include "GPM_Defines.h"
#include <GPA_Defines.h>
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>

/**
   This singleton class forms the bridge between the PML API classes and the PA. 
   It is a hidden task with its own state machine in each Application, which 
   wants to be part of the MAC subsystem of LOFAR. It will be created at the 
   moment a service of PML will be requested by the Application (like load 
   property set or load APC).
*/

class GCFPValue;
class GCFEvent;
class GCFPortInterface;
class GCFPropertySet;
class GCFMyPropertySet;
class GCFExtPropertySet;
class GPMHandler;

class GPMController : public GCFTask
{
  public:
    ~GPMController ();
    static GPMController* instance(bool temporary = false);
    static void release();

  public: // member functions
    TPMResult loadPropSet (GCFExtPropertySet& propSet);
    TPMResult unloadPropSet (GCFExtPropertySet& propSet);
    TPMResult configurePropSet (GCFPropertySet& propSet, const string& apcName);
    void deletePropSet (const GCFPropertySet& propSet);
    
    TPMResult registerScope (GCFMyPropertySet& propSet);
    TPMResult unregisterScope (GCFMyPropertySet& propSet);
       
    void propertiesLinked (const string& scope, TPAResult result);
    void propertiesUnlinked (const string& scope, TPAResult result);
  
  private:
    friend class GPMHandler;
    GPMController ();

  private: // state methods
    GCFEvent::TResult initial   (GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult connected (GCFEvent& e, GCFPortInterface& p);
        
  private: // helper methods
    unsigned short registerAction (GCFPropertySet& propSet);

  private: // data members        
    GCFPort                       _propertyAgent;
    typedef map<string /* scope */, GCFMyPropertySet*>  TMyPropertySets;
    TMyPropertySets _myPropertySets;
    typedef list<GCFExtPropertySet*>  TExtPropertySets;
    TExtPropertySets _extPropertySets;
    typedef map<unsigned short /*seqnr*/, GCFPropertySet*>  TActionSeqList;
    TActionSeqList _actionSeqList;    

  private: // admin members  

};

class GPMHandler : public GCFHandler
{
  public:
    
    ~GPMHandler() { _pInstance = 0; }
    void workProc() {}
    void stop () {}
    
  private:
    friend class GPMController;
    GPMHandler()
    { GCFTask::registerHandler(*this);}

    static GPMHandler* _pInstance;
    GPMController _controller;
};
#endif
