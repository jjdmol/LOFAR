//#  GPI_PMLlightServer.cc: 
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

#include <GPI_CEPServer.h>
#include <Common/BlobIStream.h>
#include <Common/BlobOStream.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GPI_TH_Port.h>

using LOFAR::BlobIStream;
using LOFAR::BlobOStream;
using LOFAR::TYPES::uint16;
using LOFAR::GPITH_Port;

GPICEPServer::GPICEPServer(GPIController& controller) : 
  GPIPMLlightServer(controller, PI_CEPPLS_TASK_NAME, true),
  _dhServer("Server"),
  _valueBuf(0),
  _upperboundValueBuf(0)
{
  _dhServer.setID(2);
  // placeholder for the remote port (TH_Socket) in the PIClient (CEP application)
  DH_PIProtocol dhPIClient("dummy"); 
  dhPIClient.setID(1);

  GPITH_Port   thFrom(getClientPort());
  GPITH_Port   thTo(getClientPort());
  
  dhPIClient.connectBidirectional(_dhServer, thFrom, thTo, false);
  _dhServer.init();
}

GPICEPServer::~GPICEPServer()
{
  if (_valueBuf) delete [] _valueBuf;
}

GCFEvent::TResult GPICEPServer::operational(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {      
    case F_ENTRY:
    case F_CONNECTED:
    case F_DISCONNECTED:
    case F_CLOSED:
    case F_TIMER:
    case PA_SCOPE_REGISTERED:
    case PA_SCOPE_UNREGISTERED:
    case PA_LINK_PROP_SET:
    case PA_UNLINK_PROP_SET:
      // these message can be still done by the baseclass
      GPIPMLlightServer::operational(e, p);
      break;
     
    case F_DATAIN:
    {
      // this indicates that there is data received from the CEP-PMLlight port
      // so read the data from the "port" via dataholder and transportholder
      try
      {
        _dhServer.read();
        BlobIStream& blob = _dhServer.getExtraBlob();
        uint16 result;
        // The conversion from Blob to the GCFEvent concept!!!
        switch (_dhServer.getEventID())
        {
          case DH_PIProtocol::REGISTER_SCOPE:
          {
            PIRegisterScopeEvent requestIn;
            requestIn.seqnr = _dhServer.getSeqNr();
            blob >> requestIn.scope;
            blob >> requestIn.type;
            char category;
            blob >> category;
            requestIn.category = (TPSCategory) category;
            registerPropSet(requestIn);
            break;
          }
          case DH_PIProtocol::UNREGISTER_SCOPE:
          {
            PIUnregisterScopeEvent requestIn;
            requestIn.seqnr = _dhServer.getSeqNr();
            blob >> requestIn.scope;
            unregisterPropSet(requestIn);
            break;
          }
          case DH_PIProtocol::PROPSET_LINKED:
          {
            PIPropSetLinkedEvent responseIn;
            blob >> responseIn.scope;
            blob >> result;
            responseIn.result = (TPIResult) result;
            propSetLinked(responseIn);
            break;
          }
          case DH_PIProtocol::PROPSET_UNLINKED:
          {
            PIPropSetUnlinkedEvent responseIn;
            blob >> responseIn.scope;
            blob >> result;
            responseIn.result = (TPIResult) result;
            propSetUnlinked(responseIn);
            break;
          }
          case DH_PIProtocol::VALUE_SET:
          {
            PIValueSetEvent indication;
            blob >> indication.name;
            uint16 valueSize;
            blob >> valueSize;
            if (_upperboundValueBuf < valueSize)
            {
              // enlarge the value buffer
              _upperboundValueBuf = valueSize;
              if (_valueBuf) delete [] _valueBuf;
              _valueBuf = new char[valueSize];
            }
            // reuses the valueBuffer for storing value data
            blob.get(_valueBuf, valueSize);
            if (indication.value.unpack(_valueBuf) != valueSize)
            {
              assert(0);
            }
            valueSet(indication);
            break;
          }
          default:
            assert(0);
            break;
        }
      }
      catch (std::exception& x) 
      {
        LOG_ERROR(formatString(
            "Exception in CEP server: %s",
            x.what()));    
        status = GCFEvent::NOT_HANDLED;
      }
      break;    
    }
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void GPICEPServer::sendMsgToClient(GCFEvent& msg)
{
  // The conversion of the GCFEvent concept to Blobs for the dataholder
  bool converted(true);
  try
  {
    BlobOStream& blob = _dhServer.createExtraBlob();
    string localScope;
    switch (msg.signal)
    {
      case PI_SCOPE_REGISTERED:
      {
        PIScopeRegisteredEvent* pResponseOut = (PIScopeRegisteredEvent*) &msg;
        _dhServer.setEventID(DH_PIProtocol::SCOPE_REGISTERED);
        _dhServer.setSeqNr(pResponseOut->seqnr);
        blob << (uint16) pResponseOut->result;
        break;
      }
      case PI_SCOPE_UNREGISTERED:
      {
        PIScopeUnregisteredEvent* pResponseOut = (PIScopeUnregisteredEvent*) &msg;
        _dhServer.setEventID(DH_PIProtocol::SCOPE_UNREGISTERED);
        _dhServer.setSeqNr(pResponseOut->seqnr);
        blob << (uint16) pResponseOut->result;
        break;
      }
      case PI_LINK_PROP_SET:
      {
        PILinkPropSetEvent* pRequestOut = (PILinkPropSetEvent*) &msg;
        _dhServer.setEventID(DH_PIProtocol::LINK_PROPSET);
        _dhServer.setSeqNr(0);
        blob << pRequestOut->scope;
        break;
      }
      case PI_UNLINK_PROP_SET:
      {
        PIUnlinkPropSetEvent* pRequestOut = (PIUnlinkPropSetEvent*) &msg;
        _dhServer.setEventID(DH_PIProtocol::UNLINK_PROPSET);
        _dhServer.setSeqNr(0);
        blob << pRequestOut->scope;
        break;
      }
      default:
        // no other messages should be expected here to forward them to CEP-PMLlight
        converted = false;
        break;
    }
    if (converted) _dhServer.write();
  }
  catch (std::exception& x) 
  {
    LOG_ERROR(formatString(
        "Exception in CEP server: %s",
        x.what()));    
  }
}
