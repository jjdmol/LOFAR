//#  GPM_Converter.cc: 
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

#include "GPM_Converter.h"
#include <GCF/Protocols/PA_Protocol.ph>
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
