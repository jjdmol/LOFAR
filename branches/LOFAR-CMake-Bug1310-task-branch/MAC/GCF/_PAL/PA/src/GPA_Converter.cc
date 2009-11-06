// Ctrl extension for TCP communication

#include <lofar_config.h>

#include "GPA_Converter.h"
#include <GCF/Protocols/PA_Protocol.ph>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVBlob.h>
#include <GCF/Utils.h>

#include <stdio.h>

namespace LOFAR 
{
 namespace GCF
 {
using namespace TM;
using namespace Common;
  namespace PAL 
  {

bool GPAConverter::uimMsgToGCFEvent(unsigned char* pMsgBuf, unsigned int length, GCFPVBlob& gcfEvent)
{
  string msg((char*) pMsgBuf, length);
  
  list<string> msgItems;
  Common::convStringToList(msgItems, msg);
  if (msgItems.size() < 5) return false;
  list<string>::iterator curMsgItem = msgItems.begin();

  GCFPVString portId(*curMsgItem);
  curMsgItem++;
  GCFPVString portAddr(*curMsgItem);
  curMsgItem++;
  GCFEvent* pEvent(0);
  if (*curMsgItem == "l")
  {
    PALoadPropSetEvent* pRequest = new PALoadPropSetEvent();

    pRequest->seqnr = atoi((++curMsgItem)->c_str());
    pRequest->scope = *(++curMsgItem);
    pEvent = pRequest;
  }
  else if (*curMsgItem == "ul")
  {
    PAUnloadPropSetEvent* pRequest = new PAUnloadPropSetEvent();
       
    pRequest->seqnr = atoi((++curMsgItem)->c_str());
    pRequest->scope = *(++curMsgItem);
    pEvent = pRequest;
  }
  else if (*curMsgItem == "conf")
  {
    PAConfPropSetEvent* pRequest = new PAConfPropSetEvent();
       
    pRequest->seqnr = atoi((++curMsgItem)->c_str());
    pRequest->scope = *(++curMsgItem);
    if (msgItems.size() < 6) return false;
    pRequest->apcName = *(++curMsgItem);
    pEvent = pRequest;
  }
  else 
  {
    return false;
  }

  unsigned int packsize;
  void* buf = pEvent->pack(packsize);
  
  unsigned int newBufSize = 1 + portId.getSize() + portAddr.getSize() + packsize;
  unsigned char* newBuf = new unsigned char[newBufSize];
  newBuf[0] = 'm'; // just a message
  unsigned int bytesPacked = 1;
  bytesPacked += portId.pack((char*) newBuf + bytesPacked);
  bytesPacked += portAddr.pack((char*) newBuf + bytesPacked);
  memcpy(newBuf + bytesPacked, buf, packsize);

  gcfEvent.setValue(newBuf, newBufSize, true);
  delete [] newBuf;
  delete pEvent;
  
  return true;
}

bool GPAConverter::gcfEventToUIMMsg(GCFPVBlob& gcfEvent, GCFPVBlob& uimMsg)
{
  list<string> uimMsgItems;
  _msgBuffer = gcfEvent.getValue();
  _bytesLeft = gcfEvent.getLen();
  unsigned int bytesRead = 1;
  GCFPVString portId;
  bytesRead += portId.unpack((char*) _msgBuffer + bytesRead);
  switch (_msgBuffer[0])
  {
    case 'd': // disconnect event
    {
      uimMsgItems.push_back("d");
      uimMsgItems.push_back(portId.getValue());
      break;
    } 
    case 'm': // message
    {
      uimMsgItems.push_back("m");

      uimMsgItems.push_back(portId.getValue());

      GCFPVString portAddr;
      bytesRead += portAddr.unpack((char *)_msgBuffer + bytesRead);
      uimMsgItems.push_back(portAddr.getValue());
      
      _msgBuffer += bytesRead; // move to start of real event
      _bytesLeft -= bytesRead;
      
      GCFEvent e;
      // expects and reads signal
      if (recv(&e.signal, sizeof(e.signal)) != sizeof(e.signal)) 
      {
        // don't continue with receiving
      }
      // expects and reads length
      else if (recv(&e.length, sizeof(e.length)) != sizeof(e.length))
      {
        // don't continue with receiving
      }  
      // reads payload if specified
      else if (e.length > 0)
      {
        GCFEvent* full_event = 0;
        char* event_buf = new char[sizeof(e) + e.length];
        full_event = (GCFEvent*)event_buf;
        memcpy(event_buf, &e, sizeof(e));
    
        // read the payload right behind the just memcopied basic event structure
        if (recv(event_buf + sizeof(e), e.length) > 0)
        {          
          // dispatchs an event with just received params
          encodeEvent(*full_event, uimMsgItems);
        }    
        delete [] event_buf;
      }
      // dispatchs an event without params
      else
      {
        encodeEvent(e, uimMsgItems);
      }
            
      break;
    }
    default:
      return false;
      break;
  }
  string uimMsgString;
  Common::convListToString(uimMsgString, uimMsgItems);
  uimMsg.setValue(uimMsgString);
  return true;
}

ssize_t GPAConverter::recv (void* buf, size_t count)
{
  if (_bytesLeft > 0)
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

void GPAConverter::encodeEvent(GCFEvent& e, list<string>& uimMsgItems)
{
  switch (e.signal)
  {
    case PA_PROP_SET_LOADED:
    {
      PAPropSetLoadedEvent response(e);
      uimMsgItems.push_back("loaded");
      uimMsgItems.push_back(formatString("%d", response.seqnr));
      uimMsgItems.push_back((response.result == PA_NO_ERROR ? "OK" : "failed"));
      break;
    }
    case PA_PROP_SET_UNLOADED:
    {
      PAPropSetUnloadedEvent response(e);
      uimMsgItems.push_back("unloaded");
      uimMsgItems.push_back(formatString("%d", response.seqnr));
      uimMsgItems.push_back((response.result == PA_NO_ERROR ? "OK" : "failed"));
      break;
    }
    case PA_PROP_SET_CONF:
    {
      PAPropSetConfEvent response(e);
      uimMsgItems.push_back("configured");
      uimMsgItems.push_back(formatString("%d", response.seqnr));
      uimMsgItems.push_back((response.result == PA_NO_ERROR ? "OK" : "failed"));
      uimMsgItems.push_back(response.apcName);
      break;
    }
    case PA_PROP_SET_GONE:
    {
      PAPropSetGoneEvent indication(e);
      uimMsgItems.push_back("gone");
      uimMsgItems.push_back(indication.scope);
      break;
    }    
  }
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
