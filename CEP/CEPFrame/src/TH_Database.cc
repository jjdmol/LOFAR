//  TH_Database.cc: Database TransportHolder Implementation
//
//  Copyright (C) 2000, 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//////////////////////////////////////////////////////////////////////////


#include <TH_Database.h>
#include <BaseTransport.h>
#include <Transportable.h>

TH_Database TH_Database::proto;

TH_Database::TH_Database() {}

TH_Database::~TH_Database() {}

TH_Database* TH_Database::make() const
  { return new TH_Database(); }

string TH_Database::getType() const
  { return "TH_Database"; }

bool TH_Database::connectionPossible(int srcRank, int dstRank) const
  { return srcRank == dstRank; }

bool TH_Database::recv(void* buf, int nbytes, int, int tag)
{ 
  if (getBaseTransport () -> getSourceTransportable () != 0) {
    (getBaseTransport () -> getSourceTransportable ())
      -> RetrieveFromDatabase (0, tag, (char *) buf, nbytes);
  }

  return true;
}


bool TH_Database::send(void* buf, int nbytes, int, int tag)
{
  if (getBaseTransport () -> getTargetTransportable () != 0) {
    (getBaseTransport () -> getTargetTransportable ())
      -> StoreInDatabase (0, tag, (char *) buf, nbytes);
  }

  return true;
}


void TH_Database::waitForBroadCast () {}
void TH_Database::waitForBroadCast (unsigned long&) {}
void TH_Database::sendBroadCast (unsigned long) {}
int  TH_Database::getCurrentRank () { return -1; }
int  TH_Database::getNumberOfNodes () { return 1; }
void TH_Database::init (int, const char * []) {}
void TH_Database::finalize () {}
void TH_Database::synchroniseAllProcesses () {}



