//#  RSPTimeStamp.h: Small class to hold the timestamps from RSP
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

#ifndef TFLOPCORRELATOR_TFC_INTERFACE_RSPTIMESTAMP_H
#define TFLOPCORRELATOR_TFC_INTERFACE_RSPTIMESTAMP_H

#include <Common/lofar_iostream.h>
//#include <math.h>


namespace LOFAR
{
  class TimeStamp {
  public:
    TimeStamp(const int seqId = 0, const int blockId = 0);

    // set the Stamp
    void setStamp(const int seqId, const int blockId);
    // get sequence ID
    const int getSeqId () const;
    // get block ID
    const int getBlockId () const;
    static int getMaxBlockId() {return theirMaxBlockId; };

    // the blockId restarts at zero at some point. Check if we are there yet
    void checkOverflow();

    // increase the value of the stamp
    void operator+= (const TimeStamp& other);
    void operator+= (int increment);
    void operator++ (int);

    TimeStamp operator+ (int other) const;
    int operator- (const TimeStamp& other) const;
    TimeStamp operator- (int other) const;
    bool operator>  (const TimeStamp& other) const;
    bool operator<  (const TimeStamp& other) const;
    bool operator>= (const TimeStamp& other) const;
    bool operator<= (const TimeStamp& other) const;
    bool operator== (const TimeStamp& other) const;

    friend ostream& operator<<(ostream& os, const TimeStamp& ss);

  private:
    int itsSeqId;
    int itsBlockId;

    static int theirMaxBlockId;
  };

  typedef TimeStamp timestamp_t;

  inline void TimeStamp::setStamp(const int seqId, const int blockId)
    { itsSeqId = seqId; itsBlockId = blockId; checkOverflow(); };
  
  inline const int TimeStamp::getSeqId () const
    { return itsSeqId; }
  
  inline const int TimeStamp::getBlockId () const
    { return itsBlockId; }

  inline void TimeStamp::operator += (const TimeStamp& other)
    { 
      itsBlockId += other.itsBlockId;
      checkOverflow();
      itsSeqId += other.itsSeqId;
    }
  inline void TimeStamp::operator += (int increment)
    { 
      itsBlockId += increment;
      checkOverflow();
    }

  inline void TimeStamp::operator ++ (int dummy)
    { 
      itsBlockId ++;
      checkOverflow();
    }

  inline TimeStamp TimeStamp::operator+ (int increment) const
    { 
      // check overflow is done in the constructor
      return TimeStamp(itsSeqId, itsBlockId + increment);
    }
  inline TimeStamp TimeStamp::operator- (int decrement) const
    { 
      // check overflow is done in the constructor
      return TimeStamp(itsSeqId, itsBlockId - decrement);
    }

   inline int TimeStamp::operator- (const TimeStamp& other) const
    { 
      int seqdecr = itsSeqId - other.itsSeqId;
      int blockdecr = itsBlockId - other.itsBlockId;
      
      return  (seqdecr*theirMaxBlockId) + blockdecr;
    }

  inline bool TimeStamp::operator > (const TimeStamp& other) const
    { 
      if (itsSeqId > other.itsSeqId) return true;
      if (itsSeqId < other.itsSeqId) return false;
      if (itsBlockId > other.itsBlockId) return true;
      return false;
    }
  inline bool TimeStamp::operator >= (const TimeStamp& other) const
    { return !operator<(other); }

  inline bool TimeStamp::operator < (const TimeStamp& other) const
    { 
      if (itsSeqId < other.itsSeqId) return true;
      if (itsSeqId > other.itsSeqId) return false;
      if (itsBlockId < other.itsBlockId) return true;
      return false;
    }
  inline bool TimeStamp::operator <= (const TimeStamp& other) const
    { return !operator>(other); }

  inline bool TimeStamp::operator == (const TimeStamp& other) const
    { 
      return ((itsSeqId == other.itsSeqId) && (itsBlockId == other.itsBlockId));
    }

}

#endif
    
