//#  GPA_ScopeManager.h: manages a list of all (registered) scopes
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

#ifndef GPA_SCOPEMANAGER_H
#define GPA_SCOPEMANAGER_H

#include <GPA_Defines.h>
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>

/**
   This class manages a list of all (registered) scope entries for the purpose to 
   (un)link a number of properties at once.
*/

class GCFEvent;
class GCFPortInterface; 
class GPAController;

class GPAScopeManager
{
  public:
    GPAScopeManager (GPAController& controller);
    virtual ~GPAScopeManager ();
  
    TPAResult linkProperties (list<string>& propList);
    TPAResult unlinkProperties (list<string>& propList);
    TPAResult registerScope (const string& scope, 
                             GCFPortInterface& port);
    void unregisterScope (const string& scope);
    void propertiesLinked (const string& scope);
    void propertiesUnlinked (const string& scope);
    void deleteScopesByPort (const GCFPortInterface& requestPort, 
                             list<string>& deleteScopes);
    void getSubScopes (const string& scope, 
                       list<string>& subscopes);
    void deleteAllScopes ();
    bool waitForAsyncResponses ();
          
  private: // helper methods
    void resetScopeList ();
    TPAResult fillScopeLists (list<string>& propList);
    void sendUnLinkEvents (GCFEvent& e);
    
  private: // data members
    typedef struct
    {
    	GCFPortInterface* pPort;
    	list<string> propList;
      bool respond;
    } TScopeData;
    
    GPAController&          _controller;
    map<string, TScopeData> _scopeList;
    typedef map<string, TScopeData>::iterator TScopeListIter;
  
  private: // admin. data members
    unsigned int _counter;
};

#endif
