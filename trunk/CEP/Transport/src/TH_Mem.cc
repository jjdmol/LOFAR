//# TH_Mem.cc: In-memory transport mechanism
//#
//# Copyright (C) 2000, 2001
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


#include <Transport/TH_Mem.h>
#include <Common/Debug.h>

namespace LOFAR
{

/**
 * Declare messages map which keeps track of all sent messages
 * until they have been received. This map is indexed with
 * the tag which is a unique for each connection created in
 * the Transport.
 */
map<int, TH_Mem::Msg> TH_Mem::messages;


TH_Mem::TH_Mem() :
  itsFirstSendCall(true),
  itsFirstRecvCall(true),
  itsDataSource(0)
{
}

TH_Mem::~TH_Mem()
{
}

TH_Mem* TH_Mem::make() const
{
    return new TH_Mem();
}

string TH_Mem::getType() const
{
    return "TH_Mem";
}

bool TH_Mem::connectionPossible(int srcRank, int dstRank) const
{
  cdebug(3) << "TH_Mem::connectionPossible between "
            << srcRank << " and "
            << dstRank << "?" << endl;

  return srcRank == dstRank;
}

bool TH_Mem::recvNonBlocking(void* buf, int nbytes, int tag)
{ 
  if (itsFirstRecvCall) 
  {

    Msg m;

    AssertStr(messages.end() != messages.find(tag), "no matching send for recv"); 
    m = messages[tag];

    if ( (m.isAvailable())
           && (nbytes == m.getNBytes()))
    {
      itsDataSource = m.getBuf();

      /// do the memcpy
      memcpy(buf, itsDataSource, m.getNBytes());
	
      // erase the record
      messages.erase(tag);
    }
    else
    {
      // erase the record
      messages.erase(tag);
	
      Throw("No matching send for recv: ");
    }

    itsFirstRecvCall = false;

  }
  else
  {
    AssertStr(itsDataSource != 0, "No matching send for recv");

    memcpy(buf, itsDataSource, nbytes);
  }

  return true;

}

/**
   The send function must now add the buffer to the map
   containing messages.
 */
bool TH_Mem::sendNonBlocking(void* buf, int nbytes, int tag)
{

  if (itsFirstSendCall)
  { 

    Msg      m(buf, nbytes, tag);

    messages[tag] = m;

    itsFirstSendCall = false;
	
  }

  return true;
}

void TH_Mem::waitForBroadCast()
{}

void TH_Mem::waitForBroadCast(unsigned long&)
{}


void TH_Mem::sendBroadCast(unsigned long)
{}

int TH_Mem::getCurrentRank()
{
    return -1;
}

int TH_Mem::getNumberOfNodes()
{
    return 1;
}

void TH_Mem::init(int, const char* [])
{}

void TH_Mem::finalize()
{}

void TH_Mem::synchroniseAllProcesses()
{}

TH_Mem::Msg::Msg() : itsIsAvailable(false)
{
}

TH_Mem::Msg::Msg(void* buf, int nbytes, int tag) :
    itsBuf(buf), itsNBytes(nbytes), itsTag(tag),
    itsIsAvailable(true)
{}

}
