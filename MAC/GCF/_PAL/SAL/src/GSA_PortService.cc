//#  GSA_PortService.cc:
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
//#  MERCHANTABILITY or FITNESS FOR A PMRTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#define LOFARLOGGER_SUBPACKAGE "SAL"

#include "GSA_PortService.h"
#include <GSA_Defines.h>
#include <GCF/PAL/GCF_PVSSPort.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/GCF_PVBlob.h>
#include <strings.h>
#include <stdio.h>
#include <unistd.h>

GSAPortService::GSAPortService(GCFPVSSPort& port) :
  _port(port),
  _isSubscribed(false)
{
}

GSAPortService::~GSAPortService()
{
  if (_isSubscribed)
  {
    dpeUnsubscribe(_port.getPortAddr());
    dpeUnsubscribe("__gcf_wd.sys");
    dpeUnsubscribe("__gcf_wd.man");
  }
}

void GSAPortService::start ()
{
  assert(!_isSubscribed);
  if (GCFPVSSInfo::propExists(_port.getPortAddr()))
  {
    if (dpeSubscribe(_port.getPortAddr()) != SA_NO_ERROR) 
    {
      _port.serviceStarted(false);
    }
  }
  else
  {
    string portAddr = _port.getPortAddr();
    portAddr.erase(0, GCFPVSSInfo::getLocalSystemName().length() + 1);
    if (dpCreate(portAddr, "GCFDistPort") != SA_NO_ERROR) 
    {
      _port.serviceStarted(false);
    }
  }
}

void GSAPortService::stop ()
{
  dpeUnsubscribe(_port.getPortAddr());
  dpeUnsubscribe("__gcf_wd.sys");
  dpeUnsubscribe("__gcf_wd.man");
  _isSubscribed = false;
}

ssize_t GSAPortService::send (void* buf, size_t count, const string& destDpName)
{
  assert(_isSubscribed);
  GCFPVBlob bv((unsigned char*) buf, count);
  GCFPVBlob convBv;
  
  GCFPVBlob* pBlobMsg = &bv;
  if (destDpName.find("__gcfportUIM") < string::npos)
  {
    assert(_pConverter);
    if (_pConverter->gcfEventToUIMMsg(bv, convBv))
    {
      pBlobMsg = &convBv;
    }
  }
  dpeSet(destDpName, *pBlobMsg);
  return count;
}

ssize_t GSAPortService::recv (void* buf, size_t count)
{
  if (_isSubscribed && _bytesLeft > 0)
  {
    if (count > _bytesLeft) count = _bytesLeft;
    memcpy(buf, _msgBuffer, count);
    _msgBuffer += count;
    _bytesLeft -= count;
    return count;
  }
  else
  {
    return 0;
  }
}

void GSAPortService::dpCreated(const string& dpName)
{
  assert(dpName.find(_port.getPortAddr()) < dpName.length());
  if (dpeSubscribe(_port.getPortAddr()) != SA_NO_ERROR) 
  { 
    _port.serviceStarted(false);
  }
}

void GSAPortService::dpeSubscribed(const string& dpName)
{
  static unsigned char nrOfSubscriptions = 1;
  TSAResult result(SA_NO_ERROR);
  switch (nrOfSubscriptions)
  {
    case 1:
      assert(dpName.find(_port.getPortAddr()) < dpName.length());
      result = dpeSubscribe("__gcf_wd.sys");
      break;
    case 2:
      assert(dpName.find("__gcf_wd.sys") < dpName.length());
      result = dpeSubscribe("__gcf_wd.man");
      break;
    case 3:
      assert(dpName.find("__gcf_wd.man") < dpName.length());
      _isSubscribed = true;
      nrOfSubscriptions = 0;
      _port.serviceStarted(true);
      break;
  }
  if (result != SA_NO_ERROR && !_isSubscribed)
  {
    nrOfSubscriptions = 0;
    _port.serviceStarted(false);
  }
  else
  {
    nrOfSubscriptions++;
  }
}

void GSAPortService::dpeSubscriptionLost(const string& /*dpName*/)
{
  GCFEvent e(F_DISCONNECTED);
  _port.dispatch(e);
}

void GSAPortService::dpeValueChanged(const string& dpName, const GCFPValue& value)
{
  if (dpName.find("__gcf_wd") < dpName.length())
  {
    GCFPVString* pValue = (GCFPVString*) &value;
    if (pValue->getValue()[0] == 'd')
    {
      string sysID;
      sysID.assign(pValue->getValue(), 1, string::npos);
      disconnectedEvent(sysID);
    }
  }
  else
  {
    GCFPVBlob* pValue = (GCFPVBlob*) &value;  
    _msgBuffer = pValue->getValue();
    _bytesLeft = pValue->getLen();
    unsigned int bytesRead = 1;  
    switch (_msgBuffer[0])
    {
      case 'd': // disconnect event from CTRL-script
      {
        string curPeerID;
        curPeerID.assign((char*) _msgBuffer + bytesRead, _bytesLeft - 1);
        disconnectedEvent(curPeerID);
        break;
      } 
      case 'm': // message
      {
        bytesRead += _curPeerID.unpack((char*) _msgBuffer + bytesRead);
        switch (_port.getType())
        {
          case GCFPortInterface::MSPP:
          {
            GCFPVSSPort* pPort = findClient(_curPeerID.getValue());
            bytesRead += _curPeerAddr.unpack((char *)_msgBuffer + bytesRead);
            
            if (!pPort)
            {
              GCFEvent e(F_ACCEPT_REQ);
              _port.dispatch(e);        
            }
            pPort = findClient(_curPeerID.getValue());
            if (pPort)
            {
              if (pPort->getState() == GCFPortInterface::S_CONNECTING)
              {
                GCFEvent e(F_CONNECTED);
                pPort->dispatch(e);
              }
              GCFEvent e(F_DATAIN);
              pPort->setDestAddr(_curPeerAddr.getValue());
              _bytesLeft -= bytesRead;
              _msgBuffer += bytesRead;
              pPort->dispatch(e);
            }
            break;
          }
          case GCFPortInterface::SPP:
          case GCFPortInterface::SAP:
          {
            bytesRead += _curPeerAddr.unpack((char *)_msgBuffer + bytesRead);
            _bytesLeft -= bytesRead;
            _msgBuffer += bytesRead;
            _port.setDestAddr(_curPeerAddr.getValue());
            GCFEvent e(F_DATAIN);
            _port.dispatch(e);
            break;
          }
        }
        break;
      }
      case 'u': // uim message
      {
        GCFPVBlob gcfEvent;
        assert(_pConverter);
        if (_pConverter->uimMsgToGCFEvent(_msgBuffer + 1, _bytesLeft - 1, gcfEvent))
        {
          dpeValueChanged(dpName, gcfEvent);
        }
        break;
      }
      default:
        assert(_msgBuffer[0]);
        break;
    }
  }
}

GCFPVSSPort* GSAPortService::findClient(const string& c) 
{
  GCFPVSSPort* pClientPort(0);
  for (TClients::iterator iter = _clients.begin();
       iter != _clients.end(); ++iter)
  {
    if (iter->first == c)
    {
      pClientPort = iter->second;
      break;
    }
  } 
  return pClientPort;
}

bool GSAPortService::registerPort(GCFPVSSPort& p)
{
  GCFPVSSPort* pClientPort = findClient(_curPeerID.getValue());
  if (pClientPort) return false;
  
  _clients[_curPeerID.getValue()] = &p;
  return true;
}

void GSAPortService::unregisterPort(const string& portID)
{
  _clients.erase(portID);
}

void GSAPortService::disconnectedEvent(const string& curPeerID)
{
  _curPeerID.setValue(curPeerID);
  GCFEvent e(F_DISCONNECTED);
  switch (_port.getType())
  {
    case GCFPortInterface::MSPP:
    {
      GCFPVSSPort* pClientPort(0);
      list<GCFPVSSPort*> portsToInform;
      for (TClients::iterator iter = _clients.begin();
           iter != _clients.end(); ++iter)
      {        
        if (iter->first.find(_curPeerID.getValue()) == 0)
        {
          portsToInform.push_back(&(*iter->second));
        }
      } 
      for (list<GCFPVSSPort*>::iterator iter = portsToInform.begin();
           iter != portsToInform.end(); ++iter)
      {
        pClientPort = *iter;
        pClientPort->dispatch(e);                    
      }
      break;
    }
    case GCFPortInterface::SPP:
      _port.dispatch(e);
      break;
    case GCFPortInterface::SAP:
      // no real disconnect is occured here, because it is maybe still possible 
      // to communicate with other managers/systems
      // A disconnect only can be send after a subscription lost indication 
      // for this type ports.
      break;
  }
}
