//#  GPA_PropSetSession.h: manages the lifetime of a property set with its use count
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

#ifndef GPA_PROPSETSESSION_H
#define GPA_PROPSETSESSION_H

#include <GPA_Defines.h>
#include <GCF/TM/GCF_Task.h>
#include <GPA_PropertySet.h>
#include <GPA_RequestManager.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM
  {
class GCFPortInterface;
  }
  namespace PAL 
  {

/*
   This class manages the property sets with its use count.
*/

class GPAController;

class GPAPropSetSession : public TM::GCFTask
{
  public:
  	GPAPropSetSession(GPAController& controller, TM::GCFPortInterface& psProviderPort);
  	virtual ~GPAPropSetSession();

    typedef struct
    {
      TM::GCFPortInterface* pPSUserPort;
      unsigned short count;
    } TPSUser;

    TM::GCFEvent::TResult dispatch(TM::GCFEvent& e);        
    void doNextRequest();
    
  private:
    TM::GCFEvent::TResult enabling_state(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult enabled_state(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult disabling_state(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult disabled_state(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult linking_state(TM::GCFEvent& e, TM::GCFPortInterface& p);    
    TM::GCFEvent::TResult linked_state(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult unlinking_state(TM::GCFEvent& e, TM::GCFPortInterface& p);

    void configure(TM::GCFEvent& e, TM::GCFPortInterface& p); 
    			
  private: // helper methods
    void link();
    void unlink();
    void unloaded(TM::GCFEvent* pResponse);
    void wrongState(TM::GCFEvent& e, TM::GCFPortInterface& p, const char* state);
    TPSUser* findUser(const TM::GCFPortInterface& p);
    bool mayContinue(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult defaultHandling(TM::GCFEvent& e, TM::GCFPortInterface& p);    
    
  private: // data members
    GPAController&	  _controller;
    unsigned short    _usecount;
    TM::GCFPortInterface& _psProviderPort;
    typedef list<TPSUser> TPSUsers;
    TPSUsers          _psUsers;
    GPAPropertySet    _propSet;
    GPARequestManager       _requestQueue;
    
  private: // admin. data members    
    TPAResult             _savedResult;
    unsigned short        _savedSeqnr;
    // these two "saved" members will be used to "transport" events/ports to next states (state change -> E_ENTRY)
    TM::GCFEvent*         _pSavedEvent;
    TM::GCFPortInterface* _pSavedPort;
    bool                  _isBusy;
};

inline TM::GCFEvent::TResult GPAPropSetSession::dispatch(TM::GCFEvent& e)
{
  return GCFFsm::dispatch(e, _psProviderPort);
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
