//#  GPI_Controller.h: main class of the Property Interface application
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

#ifndef GPI_CONTROLLER_H
#define GPI_CONTROLLER_H

#include <GPI_Defines.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GPI_PropertyProxy.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM
  {
class GCFEvent;
  }
  namespace PAL
  {
class GPIPMLlightServer;

// This is the main class of the Property Interface class. It has the 
// responsibility to enable PIA’s to connect to the Property Interface and thus 
// virtually to the PA too. On the connect request an instance of one of the 
// specialized GPIPMLlightServer classes will be created, which handles the 
// protocol messages from the PIA or the Property Agent.

class GPIController : public TM::GCFTask
{
	public:
		GPIController ();
		virtual ~GPIController ();
    
    GPIPropertyProxy& getPropertyProxy () {return _propertyProxy;}
    
    void close (GPIPMLlightServer& pls);

	private: // helper methods
    // Don't allow copying of this object.
    // <group>
    GPIController (const GPIController&);
    GPIController& operator= (const GPIController&);
    // </group>
    
	private: // state methods
		TM::GCFEvent::TResult initial   (TM::GCFEvent& e, TM::GCFPortInterface& p);
		TM::GCFEvent::TResult operational (TM::GCFEvent& e, TM::GCFPortInterface& p);

	private: // data members
    typedef list<GPIPMLlightServer*> TPMLlightServers;
    TPMLlightServers      _pmlLightServers;

		TM::GCFTCPPort        _rtcClientPortProvider;
    TM::GCFTCPPort        _cepClientPortProvider;
    GPIPropertyProxy      _propertyProxy;
    
  private: // admin. data members
};
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

#endif
