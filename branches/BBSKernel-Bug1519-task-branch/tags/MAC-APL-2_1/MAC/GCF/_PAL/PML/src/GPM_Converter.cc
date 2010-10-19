// Ctrl extension for TCP communication

#include "GPM_Converter.h"
#include <PA_Protocol.ph>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVBlob.h>
#include <GCF/Utils.h>

#include <stdio.h>

bool GPMConverter::uimMsgToGCFEvent(unsigned char* pMsgBuf, unsigned int length, GCFPVBlob& gcfEvent)
{
  string msg((char*) pMsgBuf, length);
  
  list<string> msgItems;
  Utils::convStringToList(msgItems, msg);
  if (msgItems.size() < 4) return false;
  list<string>::iterator curMsgItem = msgItems.begin();

  GCFPVString portId(*curMsgItem);
  curMsgItem++;
  GCFPVString portAddr(*curMsgItem);
  curMsgItem++;
  GCFEvent* pEvent(0);
  if (*curMsgItem == "gone")
  {
    PAPropSetGoneEvent* pIndication = new PAPropSetGoneEvent();

    pIndication->scope = *(++curMsgItem);
    pEvent = pIndication;
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

bool GPMConverter::gcfEventToUIMMsg(GCFPVBlob& /*gcfEvent*/, GCFPVBlob& /*uimMsg*/)
{
  assert(0);
  return false;
}
