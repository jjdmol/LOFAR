//#  SyncStamp.cc: Small class to hold the stamps that come from the station
//#
//#  Copyright (C) 2002-2005
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

#include <SyncStamp.h>

namespace LOFAR {
  SyncStamp::SyncStamp(const int seqId, const int blockId) 
    :itsSeqId(seqId),
     itsBlockId(blockId)
  {
    checkOverflow();
  }

  void SyncStamp::checkOverflow() {
    if (itsBlockId > MAX_BLOCK_ID) {
      int newBlockId = itsBlockId % MAX_BLOCK_ID;
      itsSeqId += itsBlockId / MAX_BLOCK_ID; 
      itsBlockId = newBlockId; 
    };
  }


  ostream& operator<<(ostream& os, const SyncStamp& ss){
    os<<ss.itsSeqId<<" s: "<<ss.itsBlockId;
    return os;
  }
}
