//#  GPA_Controller.h: main class of the Property Agent
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

#ifndef GPA_CONTROLLER_H
#define GPA_CONTROLLER_H

#include <GPA_Defines.h>
#include <GPA_RequestManager.h>
#include <GPA_Converter.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/PAL/GCF_PVSSPort.h>

/**
   This is the main class of the Property Agent. It uses a number of helper 
   classes to manage PML requests, registered scopes and use counts of created 
   properties. The assigned port provider supports the possibility to accept 
   more than one connect request from different clients (PML).
*/

namespace LOFAR 
{
 namespace GCF 
 {
  namespace PAL 
  {

class GPAPropertySet;

class GPAController : public TM::GCFTask
{
	public:
		GPAController();
		virtual ~GPAController();
  
	private: // state methods
		TM::GCFEvent::TResult initial(TM::GCFEvent& e, TM::GCFPortInterface& p);
		TM::GCFEvent::TResult operational(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult linking(TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult unlinking(TM::GCFEvent& e, TM::GCFPortInterface& p);

  private: // helper methods
    friend class GPAPropertySet;
    bool mayContinue(TM::GCFEvent& e, TM::GCFPortInterface& p);
    void sendAndNext(TM::GCFEvent& e);
    void doNextRequest();    
    GPAPropertySet* findPropSet(const string& scope) const;
    void acceptConnectRequest(TM::GCFPortInterface& p);
    void clientPortGone(TM::GCFPortInterface& p);
    void propSetClientGone(TM::GCFPortInterface& p);
    void deletePort(TM::GCFPortInterface& p);
    void emptyGarbage();
    //GCFPVSSPort& getDistPmlPort() { return _distPmlPortProvider;}
    
	private: // data members
    typedef map<string /*scope*/, GPAPropertySet*> TPropertySets;
    TPropertySets         _propertySets;
    list<GPAPropertySet*> _propertySetGarbage;
    
		GPARequestManager       _requestManager;

    list<TM::GCFPortInterface*> _pmlPorts;		
    list<TM::GCFPortInterface*> _pmlPortGarbage;
		TM::GCFTCPPort              _pmlPortProvider;
    PAL::GCFPVSSPort            _distPmlPortProvider;
    
  private: // admin. data members
    bool              _isBusy;
    bool              _isRegistered;
    unsigned long     _garbageTimerId;
    unsigned int      _counter;  
    GPAPropertySet*   _pCurPropSet;
    GPAConverter      _converter;   
    bool              _synchronous;
};
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

#endif
