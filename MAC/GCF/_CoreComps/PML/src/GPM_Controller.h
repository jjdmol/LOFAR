//#  GPM_Controller.h: 
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

#include <TM/GCF_Task.h>
#include <TM/GCF_Port.h>
#include "GPM_Service.h"
#include "GPM_Defines.h"
#include <Common/lofar_map.h>

class GCFSupervisedTask;
class GPMPropertySet;
class GCFPValue;
class GCFEvent;
class GCFPortInterface;

class GPMController : public GCFTask
{
  public:
    GPMController(GCFSupervisedTask& supervisedTask);
    virtual ~GPMController();

  private: // member functions
    friend class GCFSupervisedTask;
    TPMResult loadAPC(string& apcName, string& scope);
    TPMResult unloadAPC(string& apcName, string& scope);
    TPMResult reloadAPC(string& apcName, string& scope);
    TPMResult loadMyProperties(TPropertySet& newSet);
    TPMResult unloadMyProperties(TPropertySet& newSet);
    TPMResult set(string& propName, GCFPValue& value);
    TPMResult get(string& propName);
    TPMResult getMyOldValue(string& propName, GCFPValue** value);
    void valueChanged(string& propName, GCFPValue& value);
    void valueGet(string& propName, GCFPValue& value);
    void propertiesLinked(list<string>& notLinkedProps);
    void propertiesUnlinked(list<string>& notUnlinkedProps);
  
  private: // state methods
    int initial  (GCFEvent& e, GCFPortInterface& p);
        
  private: // data members
    GCFSupervisedTask& _supervisedTask;
    
    map<string, GPMPropertySet*> _propertySets;
    typedef map<string, GPMPropertySet*>::iterator TPropertySetIter;
    GCFPort _propertyAgent;
    GPMService _scadaService;
    
    typedef struct
    {
      GCFPValue* pValue;
      string* pPropName;
    } TGetData;
};
#endif