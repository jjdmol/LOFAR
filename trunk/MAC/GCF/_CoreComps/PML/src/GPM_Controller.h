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
#include <TM/PortInterface/GCF_Port.h>
#include "GPM_Defines.h"
#include "GPM_Service.h"
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>

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
    friend class GPMService;
    friend class GPMPropertySet;
    
    TPMResult loadAPC(const string& apcName, const string& scope);
    TPMResult unloadAPC(const string& apcName, const string& scope);
    TPMResult reloadAPC(const string& apcName, const string& scope);
    TPMResult loadMyProperties(TPropertySet& newSet);
    TPMResult unloadMyProperties(const string& scope);
    TPMResult set(const string& propName, const GCFPValue& value);
    TPMResult get(const string& propName);
    TPMResult getMyOldValue(const string& propName, GCFPValue** value);
    void valueChanged(const string& propName, const GCFPValue& value);
    void valueGet(const string& propName, const GCFPValue& value);
    void propertiesLinked(const string& scope, list<string>& notLinkedProps);
    void propertiesUnlinked(const string& scope, list<string>& notUnlinkedProps);
  
  private: // state methods
    int initial  (GCFEvent& e, GCFPortInterface& p);
    int connected(GCFEvent& e, GCFPortInterface& p);
        
  private: // helper methods
    GPMPropertySet* findPropertySet(const string& propName);
    void registerScope(const string& scope);
    void unpackPropertyList(char* pListData, list<string>& propertyList);
    
  private: // data members
    GCFSupervisedTask& _supervisedTask;
    
    map<string, GPMPropertySet*> _propertySets;
    typedef map<string, GPMPropertySet*>::iterator TPropertySetIter;
    GCFPort _propertyAgent;
    GPMService _scadaService;
    bool _isBusy;
    bool _preparing;
    unsigned int _counter;
    
    typedef struct
    {
      GCFPValue* pValue;
      string* pPropName;
    } TGetData;
};
#endif
