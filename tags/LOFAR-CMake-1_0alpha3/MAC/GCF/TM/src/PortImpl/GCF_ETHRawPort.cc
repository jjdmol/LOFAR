//# GCF_ETHRawPort.cc: connection to a remote process
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <GCF/TM/GCF_ETHRawPort.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_RawPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Protocols.h>
#include "GTM_ETHSocket.h"
#include <GTM_Defines.h>
#include <errno.h>
#include <Common/ParameterSet.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {

GCFETHRawPort::GCFETHRawPort(GCFTask& 		task,
                          	 const string&	name,
                          	 TPortType 		type, 
                             bool 			transportRawData) : 
   GCFRawPort(task, name, type, 0, transportRawData), 
   _pSocket(0), _ethertype(0x0000)
{
  ASSERT(MSPP != getType());

  _pSocket = new GTMETHSocket(*this);
}

GCFETHRawPort::GCFETHRawPort() : 
  GCFRawPort(),       
  _pSocket(0), _ethertype(0x0000)
{
  ASSERT(MSPP != getType());
}

GCFETHRawPort::~GCFETHRawPort()
{
  if (_pSocket) delete _pSocket;  
}

bool GCFETHRawPort::close()
{
  setState(S_CLOSING);
  schedule_close();

  return true;
}

bool GCFETHRawPort::open()
{
  if (isConnected())
  {
    LOG_WARN("already connected");
   
    return false;
  }
  else if (MSPP == getType())
  {
    LOG_ERROR(formatString ( 
        "ETH raw ports can not act as a MSPP (%s).",
        getRealName().c_str()));
    return false;
  }
  // check for ifname
  if (_ifname == "")
  {    
    try 
    {
      string ifNameParam = formatString(
          PARAM_ETH_IFNAME,
          getTask()->getName().c_str(),
          getRealName().c_str());
      _ifname += globalParameterSet()->getString(ifNameParam);      
    }
    catch (...)
    {
      LOG_ERROR("no interface name specified");
      return false;
    }
  }

  if (_destMacStr == "")
  {    
    try 
    {
      string destMacParam = formatString(
          PARAM_ETH_MACADDR,
          getTask()->getName().c_str(),
          getRealName().c_str());
      _destMacStr += globalParameterSet()->getString(destMacParam);      
    }
    catch (...)
    {
      LOG_ERROR("no destination mac adres is specified");
      return false;
    }
  }

  if (_ethertype == 0x0000)
  {
    try 
    {
      string ethertypeParam = formatString(
          PARAM_ETH_ETHERTYPE,
          getTask()->getName().c_str(),
          getRealName().c_str());
      _ethertype += globalParameterSet()->getInt32(ethertypeParam);      
    }
    catch (...)
    {
      // is optional so no problem.
    }
  }
  if (!_pSocket)
  {
    if (isSlave())
    {
      LOG_ERROR(formatString (
  			  "Port %s is not initialised.",
  			  getRealName().c_str()));
      return false;
    }
    else    
    {
      _pSocket = new GTMETHSocket(*this);
    }
  } 
   
  if (_pSocket->open(_ifname.c_str(), _destMacStr.c_str(), _ethertype) < 0)
  {    
    if (SAP == getType())
    {
      setState(S_DISCONNECTING);
      schedule_disconnected();
    }
    else
    {
      return false;
    }
  }
  else
  { 
    setState(S_CONNECTING);
    schedule_connected();
  }
  return true;
}

ssize_t GCFETHRawPort::send(GCFEvent& e)
{
  size_t written = 0;

  ASSERT(_pSocket);

  if (!isConnected()) 
  {
    LOG_ERROR(formatString (
        "Port '%s' on task '%s' not connected! Event not sent!",
        getRealName().c_str(),
        getTask()->getName().c_str()));
    return 0;
  }

  unsigned int packsize;
  void* buf = e.pack(packsize);

  LOG_DEBUG(formatString (
      "Sending event '%s' for task '%s' on port '%s'",
//      getTask()->eventName(e).c_str(),
      eventName(e).c_str(),
      getTask()->getName().c_str(), 
      getRealName().c_str()));

  if ((written = _pSocket->send(buf, packsize)) != packsize)
  {
    LOG_DEBUG(formatString (
        "truncated send: %s",
        strerror(errno)));
      
    setState(S_DISCONNECTING);  
    schedule_disconnected();
    
    written = 0;
  }

  return written;
}

ssize_t GCFETHRawPort::recv(void* buf, size_t count)
{
  if (!isConnected()) return 0;
  ASSERT(_pSocket);
  return _pSocket->recv(buf, count);
}

void GCFETHRawPort::setAddr(const char* ifname,
			    const char* destMac)
{
  // store for use in open
  _ifname     = ifname;
  _destMacStr = destMac;
}

void GCFETHRawPort::setEtherType(unsigned short type)
{
  _ethertype = type;
}
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

