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

#include "GSA_PortService.h"
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
}

void GSAPortService::start ()
{
  assert(!_isSubscribed);
  if (GCFPVSSInfo::propExists(_port.getRealName()))
  {
    if (dpeSubscribe(_port.getRealName()) != SA_NO_ERROR) _port.serviceStarted(false);
  }
  else
  {
    if (dpCreate(_port.getRealName(), "LPT_BLOB") != SA_NO_ERROR) _port.serviceStarted(false);
  }
}

void GSAPortService::stop ()
{
  dpeUnsubscribe(_port.getRealName());
}

ssize_t GSAPortService::send (void* buf, size_t count, const string& destDpName)
{
  assert(_isSubscribed);
  GCFPVBlob bv((unsigned char*) buf, count);
  dpeSet(destDpName, bv);
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
  assert(dpName.find(_port.getRealName()) < dpName.length());
  if (dpeSubscribe(_port.getRealName()) != SA_NO_ERROR) _port.serviceStarted(false);  
}

void GSAPortService::dpDeleted(const string& /*dpName*/)
{
  GCFEvent e(F_DISCONNECTED);
  _port.dispatch(e);
}

void GSAPortService::dpeSubscribed(const string& dpName)
{
  assert(dpName.find(_port.getRealName()) < dpName.length());
  _isSubscribed = true;
  _port.serviceStarted(true);
}

void GSAPortService::dpeSubscriptionLost(const string& /*dpName*/)
{
  GCFEvent e(F_DISCONNECTED);
  _port.dispatch(e);
}

void GSAPortService::dpeValueChanged(const string& /*dpName*/, const GCFPValue& value)
{
  GCFPVBlob* pValue = (GCFPVBlob*) &value;  
  _msgBuffer = pValue->getValue();
  _bytesLeft = pValue->getLen();
  unsigned int bytesRead = 1;
  bytesRead += _curPeerID.unpack((char*) _msgBuffer + bytesRead);
  switch (_msgBuffer[0])
  {
    case 'd': // disconnect event from CTRL-script
    {
      GCFEvent e(F_DISCONNECTED);
      switch (_port.getType())
      {
        case GCFPortInterface::MSPP:
        {
          GCFPVSSPort* pClientPort(0);
          for (TClients::iterator iter = _clients.begin();
               iter != _clients.end(); ++iter)
          {
            if (iter->first.find(_curPeerID.getValue()) == 0)
            {
              pClientPort = &(*iter->second);
              pClientPort->dispatch(e);
              break;
            }
          } 
          break;
        }
        case GCFPortInterface::SPP:
        case GCFPortInterface::SAP:
          _port.dispatch(e);
          break;
      }
      break;
    } 
    case 'm': // message
    {
      switch (_port.getType())
      {
        case GCFPortInterface::MSPP:
        {
          GCFPVSSPort* pPort = findClient(_curPeerID.getValue());
          bytesRead += _curPeerAddr.unpack((char *)_msgBuffer + bytesRead);
          if (pPort)
          {
            GCFEvent e(F_ACCEPT_REQ);
            _port.dispatch(e);        
          }
          pPort = findClient(_curPeerID.getValue());
          if (pPort)
          {
            GCFEvent e(F_DATAIN);
            pPort->setDestAddr(_curPeerAddr.getValue());
            _bytesLeft -= bytesRead;
            pPort->dispatch(e);
          }
          break;
        }
        case GCFPortInterface::SPP:
        case GCFPortInterface::SAP:
        {
          bytesRead += _curPeerAddr.unpack((char *)_msgBuffer + bytesRead);
          _bytesLeft -= bytesRead;
          _port.setDestAddr(_curPeerAddr.getValue());
          GCFEvent e(F_DATAIN);
          _port.dispatch(e);
          break;
        }
      }
      break;
    }
    default:
      assert(_msgBuffer[0]);
      break;
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
  GCFPVSSPort* pClientPort = findClient(p.getPortID());
  if (pClientPort) return false;
  
  _clients[p.getPortID()] = &p;
  return true;
}

void GSAPortService::unregisterPort(const string& portID)
{
  _clients.erase(portID);
}
