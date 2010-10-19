//#  GTM_SBTCPPort.cc: connection to a remote process
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

#include "GTM_SBTCPPort.h"
#include <GTM_Defines.h>
#include "GSB_Defines.h"
#include <PortImpl/GTM_TCPServerSocket.h>
#include <GCF/ParameterSet.h>

namespace LOFAR 
{
 namespace GCF 
 {
  using namespace TM;
  namespace SB 
  {

GTMSBTCPPort::GTMSBTCPPort(GCFTask& task, 
                       string name, 
                       TPortType type, 
                       int protocol, 
                       bool transportRawData) 
  : GCFTCPPort(task, name, type, protocol, transportRawData)
{
}

GTMSBTCPPort::GTMSBTCPPort()
    : GCFTCPPort()
{
}

GTMSBTCPPort::~GTMSBTCPPort()
{
  if (_pSocket)
  {
    delete _pSocket;  
    _pSocket = 0;  
  }
}


bool GTMSBTCPPort::open()
{
  if (isConnected())
  {
    LOG_ERROR(formatString ( 
        "Port %s already open.",
	      _name.c_str()));
    return false;
  }
  else if (!_pSocket)
  {
    if (isSlave())
    {
      LOG_ERROR(formatString ( 
          "Port %s not initialised.",
          _name.c_str()));
      return false;
    }
    else
    {
      if (SPP == getType() || MSPP == getType())
      {
        _pSocket = new GTMTCPServerSocket(*this, (MSPP == getType()));
      }
      else if (SAP == getType())
      {
        _pSocket = new GTMTCPSocket(*this);
      }
    }
  }
  
  unsigned int sbPortNumber(0);
  string sbHost;
  try 
  {    
    sbPortNumber = ParameterSet::instance()->getInt(PARAM_SB_SERVER_PORT);    
    sbHost = ParameterSet::instance()->getString(PARAM_SB_SERVER_HOST);    
  }
  catch (...)
  {
    LOG_FATAL("Could not get special portnumber param or host param for the Service Broker");
    GCFTask::stop();
    return false;
  }
  
  if (_pSocket->open(sbPortNumber))
  { 
    if (SAP == getType())
    {   
      if (_pSocket->connect(sbPortNumber, sbHost))
      {
        setState(S_CONNECTING);        
        schedule_connected();
      }
      else
      {
        setState(S_DISCONNECTING);
        schedule_disconnected();
      }
    } 
    else if (MSPP == getType())
    {
      setState(S_CONNECTING);        
      schedule_connected();
    }
  }
  else
  {
    setState(S_DISCONNECTING);
    if (SAP == getType())
    {
      schedule_disconnected();
    }
    else
    {
      return false;
    }
  }
  return true;
}
  } // namespace SB
 } // namespace GCF
} // namespace LOFAR
