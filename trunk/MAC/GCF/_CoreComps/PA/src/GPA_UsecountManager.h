//#  GPA_UsecountManager.h: 
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

#ifndef GPA_USECOUNTMANAGER_H
#define GPA_USECOUNTMANAGER_H

#include <GPA_Defines.h>
#include <SAL/GSA_Service.h>
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>

class GPAController;

class GPAUsecountManager : public GSAService
{
  public:
  	GPAUsecountManager(GPAController& controller);
  	virtual ~GPAUsecountManager();
  	TPAResult incrementUsecount(const list<TAPCProperty>& propList);
  	TPAResult decrementUsecount(const list<TAPCProperty>& propList);
  	TPAResult deletePropertiesByScope(const string& scope, list<string>& subScopes);
    TPAResult setDefaults(const list<TAPCProperty>& propsFromAPC);
    void deleteAllProperties();
    bool waitForAsyncResponses();
			
  protected:
    void propCreated(string& propName);
    void propDeleted(string& propName);
    void propValueChanged(string& propName, GCFPValue& value) {};
    void propValueGet(string& propName, GCFPValue& value) {};
    void propSubscribed(string& propName) {};
    void propUnsubscribed(string& propName) {};

  private: // helper methods
    TPAResult createMACValueObject(
      const string& macType, 
      const string& valueData, 
      GCFPValue** pValue);
    
  private: // data members
    GPAController&	_controller;
    
    map<string /*propName*/, unsigned int /*usecount*/> _propList;
    typedef map<string, unsigned int>::iterator TPropListIter;	
    
  private: // admin. data members
    typedef enum State {DECREMENT, DELETE_BY_SCOPE, DELETE_ALL};
    State _state;
    list<string> _tempPropList;
    unsigned int _counter;
};

#endif
