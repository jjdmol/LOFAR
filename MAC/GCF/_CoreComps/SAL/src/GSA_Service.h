//#  GSA_Service.h: 
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

#ifndef  GSA_SERVICE_H
#define  GSA_SERVICE_H

#include <Common/lofar_string.h>
#include <SAL/GSA_Defines.h>

// GCF forwards
class GCFPValue;

// GCF/SAL forwards
class GSAWaitForAnswer;

// PVSS forwards
class DpMsgAnswer;
class DpHLGroup;
class Variable;
class CharString;
class DpIdentifier;

class GSAService
{
  public:
    GSAService();
    virtual ~GSAService();

  protected:
    virtual TSAResult createProp(const string& macType, const string& propName);
    virtual TSAResult deleteProp(const string& propName);
    virtual TSAResult subscribe(const string& propName);
    virtual TSAResult unsubscribe(const string& propName);
    virtual TSAResult get(const string& propName);
    virtual TSAResult set(const string& propName, const GCFPValue& value);
    virtual bool exists(const string& propName);
    
    virtual void propCreated(string& propName) = 0;
    virtual void propDeleted(string& propName) = 0;
    virtual void propSubscribed(string& propName) = 0;
    virtual void propUnsubscribed(string& propName) = 0;
    virtual void propValueGet(string& propName, GCFPValue& value) = 0;
    virtual void propValueChanged(string& propName, GCFPValue& value) = 0;
        
  private: // methods
    // interface for GSAWaitForAnswer
    void handleHotLink(const DpMsgAnswer& answer);
    void handleHotLink(const DpHLGroup& group);
    friend class GSAWaitForAnswer;
    
    // helper methods to convert PVSS dpTypes to MAC types and visa versa
    TSAResult convertPVSSToMAC(const Variable& variable, 
                          const CharString& typeName, 
                          GCFPValue** pMacValue) const;
                          
    TSAResult convertMACToPVSS(const GCFPValue& macValue, Variable** pVar) const;

    // helper methods
    TSAResult getDpId(const string& dpName, DpIdentifier& dpId) const;

  private: // data members    
    GSAWaitForAnswer* _pWFA;
    int _error;
};                                 

#endif
