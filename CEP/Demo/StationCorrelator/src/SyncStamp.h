//#  SyncStamp.h: Small class to hold the stamps that come from the station
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

#ifndef SYNCSTAMP_H
#define SYNCSTAMP_H

#define MAX_BLOCK_ID 1000

namespace LOFAR
{
  class SyncStamp {
  public:
    SyncStamp(const int blockId = 0, const int seqId = 0);

    /// set the Stamp
    void setStamp(const int seqId, const int blockId);

    /// the blockId restarts at zero at some point. Check if we are there yet
    void checkOverflow();
    /// increase the value of the stamp
    void increment(const int value);

    void operator+= (SyncStamp& other);
    void operator++ (int);
    bool operator> (SyncStamp& other);
    bool operator< (SyncStamp& other);
    bool operator== (SyncStamp& other);

  private:
    int itsSeqId;
    int itsBlockId;
  };

  inline void SyncStamp::setStamp(const int seqId, const int blockId)
    { itsSeqId = seqId; itsBlockId = blockId; checkOverflow(); };
  inline void SyncStamp::increment(const int value)
    { itsBlockId += value; checkOverflow(); };
  inline void SyncStamp::checkOverflow()
    { if (itsBlockId > MAX_BLOCK_ID) {itsSeqId++; itsBlockId = 0;};}

  inline void SyncStamp::operator += (SyncStamp& other)
    { 
      itsBlockId += other.itsBlockId;
      checkOverflow();
      itsSeqId += other.itsSeqId;
    }
  inline void SyncStamp::operator ++ (int dummy)
    { 
      itsBlockId ++;
      checkOverflow();
    }
  inline bool SyncStamp::operator > (SyncStamp& other)
    { 
      if (itsSeqId > other.itsSeqId) return true;
      if (itsSeqId < other.itsSeqId) return false;
      if (itsBlockId > other.itsBlockId) return true;
      return false;
    }
  inline bool SyncStamp::operator < (SyncStamp& other)
    { 
      if (itsSeqId < other.itsSeqId) return true;
      if (itsSeqId > other.itsSeqId) return false;
      if (itsBlockId < other.itsBlockId) return true;
      return false;
    }
  inline bool SyncStamp::operator == (SyncStamp& other)
    { 
      return ((itsSeqId == other.itsSeqId) && (itsBlockId == other.itsBlockId));
    }
}

#endif
    
