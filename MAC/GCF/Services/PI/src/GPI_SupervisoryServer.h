//#  GPI_SupervisoryServer.h: 
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

#ifndef GPI_SUPERVISORYSERVER_H
#define GPI_SUPERVISORYSERVER_H

#include <GPI_Defines.h>
#include <GPI_PropertyProxy.h>
#include <TM/GCF_Task.h>
#include <TM/Socket/GCF_TCPPort.h>

class GCFEvent;
class GPIController;
class GCFPValue;

class GPISupervisoryServer : public GCFTask
{
	public:
		GPISupervisoryServer(GPIController& controller);
		virtual ~GPISupervisoryServer();

  public: // call back methods for the GPIPropertyProxy
    void propSubscribed(string& propName);
    void propUnsubscribed(string& propName);
    void propValueChanged(string& propName, GCFPValue& value);
    
	private: // helper methods
    void registerScope(const string& scope);
    void subscribe(char* data, bool onOff);
    void unpackPropertyList(char* pListData, list<string>& propertyList);
    TPIResult unLinkProperties(list<string>& properties, bool onOff);
    
	private: // state methods
		int initial(GCFEvent& e, GCFPortInterface& p);
		int connected(GCFEvent& e, GCFPortInterface& p);
    int operational(GCFEvent& e, GCFPortInterface& p);
    int closing(GCFEvent& e, GCFPortInterface& p);

	private: // data members
		GCFTCPPort        _ssPort;
    GCFTCPPort        _propertyAgent;
    GPIController     _controller;
    GPIPropertyProxy  _propProxy;
    string            _name;
    
  private: // admin. data members
    bool _isBusy;
    unsigned int _counter;
};

#endif
