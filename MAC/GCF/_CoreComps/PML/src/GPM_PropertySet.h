//#  GPM_PropertySet.h: 
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

#ifndef GPM_PROPERTYSET_H
#define GPM_PROPERTYSET_H

#include <SAL/GSA_Service.h>
#include <GCFCommon/GCF_Defines.h>
#include "GPM_Defines.h"
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>

class GPMProperty;
class GPMController;

class GPMPropertySet : public GSAService
{
  public:
    GPMPropertySet(GPMController& controller, TPropertySet& propSet);
    ~GPMPropertySet();
    
  protected:
    void propCreated(const string& /*propName*/) {};
    void propDeleted(const string& /*propName*/) {};
    void propValueGet(const string& /*propName*/, const GCFPValue& /*value*/) {};
    void propSubscribed(const string& propName);
    void propUnsubscribed(const string& propName);
    void propValueChanged(const string& propName, const GCFPValue& value);
  
  private: // methods for GPMController
    friend class GPMController;  
    TPMResult linkProperties(unsigned int seqnr, list<string>& properties);
    TPMResult unlinkProperties(unsigned int seqnr, list<string>& properties);
    TPMResult getValue(const string& propName, GCFPValue** pValue);
    TPMResult setValue(const string& propName, const GCFPValue& value);
    TPMResult getOldValue(const string& propName, GCFPValue** value);
    inline const string& getScope() const {return _scope;}   
  
  private: // helper methods
    TPMResult cutScope(string& propName);
            
  private:
    map<string, GPMProperty*> _properties;
    list<string> _tempLinkList;
    unsigned int _tempSeqnr;
    typedef map<string, GPMProperty*>::iterator TPropertyIter;
    GPMController& _controller;
    string _scope;
    unsigned int _counter;
};
#endif
