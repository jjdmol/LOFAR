//#  GPA_UsecountManager.h: manages the properties with its use count
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
#include <GSA_Service.h>
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>

/**
   This class manages the properties with its use count, which are created 
   (resp. deleted) by means of the base class GSAService.
*/

class GPAController;

class GPAUsecountManager : public GSAService
{
  public:
  	GPAUsecountManager(GPAController& controller);
  	virtual ~GPAUsecountManager();
  	void incrementUsecount(const list<TAPCProperty>& propList);
  	void decrementUsecount(const list<TAPCProperty>& propList);
    TPAResult setDefaults(const list<TAPCProperty>& propsFromAPC);
    void deletePropertiesByScope(const string& scope, list<string>& subScopes);
    void deleteAllProperties();
    bool waitForAsyncResponses();
			
  protected:
    void propCreated(const string& propName);
    void propDeleted(const string& propName);
    inline void propValueGet(const string& /*propName*/, const GCFPValue& /*value*/) {}; 
    inline void propValueChanged(const string& /*propName*/, const GCFPValue& /*value*/) {};
    inline void propSubscribed(const string& /*propName*/) {};
    inline void propUnsubscribed(const string& /*propName*/) {};

  private: // helper methods
    
  private: // data members
    GPAController&	_controller;
    
    map<string /*propName*/, unsigned int /*usecount*/> _propList;
    typedef map<string, unsigned int>::iterator TPropListIter;	
    
  private: // admin. data members
    typedef enum State {NO_ACTION, DECREMENT, DELETE_BY_SCOPE, DELETE_ALL};
    State _state;
    list<string> _tempPropList;
    unsigned int _counter;
};

#endif
