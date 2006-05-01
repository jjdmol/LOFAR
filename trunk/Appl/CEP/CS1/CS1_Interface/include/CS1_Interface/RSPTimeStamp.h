//#  RSPTimeStamp.h: Small class to hold the timestamps from RSP
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_CS1_INTERFACE_RSPTIMESTAMP_H
#define LOFAR_CS1_INTERFACE_RSPTIMESTAMP_H

#include <Common/lofar_iosfwd.h>

namespace LOFAR {
namespace CS1 {

class TimeStamp {
  public:
		    TimeStamp(long long time = 0);
		    TimeStamp(unsigned seqId, unsigned blockId);

    TimeStamp	    &setStamp(unsigned seqId, unsigned blockId);
    unsigned	    getSeqId() const;
    unsigned	    getBlockId() const;
    static unsigned getMaxBlockId();
    static void     setMaxBlockId(unsigned nMBID);

    TimeStamp	    &operator += (const TimeStamp &other);
    TimeStamp	    &operator += (long long increment);
    TimeStamp	    &operator -= (const TimeStamp &other);
    TimeStamp	    &operator -= (long long decrement);
    TimeStamp	    &operator ++ (int);
    TimeStamp	    &operator ++ ();
    TimeStamp	    &operator -- (int);
    TimeStamp	    &operator -- ();

    TimeStamp	    operator + (long long other) const;
    long long	    operator + (const TimeStamp &other) const;
    TimeStamp	    operator - (long long other) const;
    long long	    operator - (const TimeStamp &other) const;

    bool	    operator >  (const TimeStamp &other) const;
    bool	    operator <  (const TimeStamp &other) const;
    bool	    operator >= (const TimeStamp &other) const;
    bool	    operator <= (const TimeStamp &other) const;
    bool	    operator == (const TimeStamp &other) const;
    bool	    operator != (const TimeStamp &other) const;

    friend ostream  &operator << (ostream &os, const TimeStamp &ss);

  private:
      long long		itsTime;
      static unsigned	theirMaxBlockId;
};

typedef TimeStamp timestamp_t;

inline TimeStamp::TimeStamp(long long time)
{
  itsTime = time;
}

inline TimeStamp::TimeStamp(unsigned seqId, unsigned blockId)
{
  itsTime = (long long) seqId * theirMaxBlockId + blockId;
}

inline TimeStamp &TimeStamp::setStamp(unsigned seqId, unsigned blockId)
{
  itsTime = (long long) seqId * theirMaxBlockId + blockId;
  return *this;
}

inline unsigned TimeStamp::getSeqId() const
{
  return itsTime / theirMaxBlockId;
}

inline unsigned TimeStamp::getBlockId() const
{
  return itsTime % theirMaxBlockId;
}

unsigned TimeStamp::getMaxBlockId()
{
  return theirMaxBlockId;
}

void TimeStamp::setMaxBlockId(unsigned nMBID)
{
  theirMaxBlockId = nMBID;
}

inline TimeStamp &TimeStamp::operator += (const TimeStamp &other)
{ 
  itsTime += other.itsTime;
  return *this;
}

inline TimeStamp &TimeStamp::operator += (long long increment)
{ 
  itsTime += increment;
  return *this;
}

inline TimeStamp &TimeStamp::operator -= (const TimeStamp &other)
{ 
  itsTime -= other.itsTime;
  return *this;
}

inline TimeStamp &TimeStamp::operator -= (long long decrement)
{ 
  itsTime -= decrement;
  return *this;
}

inline TimeStamp &TimeStamp::operator ++ ()
{ 
  ++ itsTime;
  return *this;
}

inline TimeStamp &TimeStamp::operator ++ (int)
{ 
  ++ itsTime;
  return *this;
}

inline TimeStamp &TimeStamp::operator -- ()
{ 
  -- itsTime;
  return *this;
}

inline TimeStamp &TimeStamp::operator -- (int)
{ 
  -- itsTime;
  return *this;
}

inline TimeStamp TimeStamp::operator + (long long increment) const
{ 
  return TimeStamp(itsTime + increment);
}

inline long long TimeStamp::operator + (const TimeStamp &other) const
{ 
  return itsTime + other.itsTime;
}

inline TimeStamp TimeStamp::operator - (long long decrement) const
{ 
  return TimeStamp(itsTime - decrement);
}

inline long long TimeStamp::operator - (const TimeStamp &other) const
{ 
  return itsTime - other.itsTime;
}

inline bool TimeStamp::operator > (const TimeStamp &other) const
{ 
  return itsTime > other.itsTime;
}

inline bool TimeStamp::operator >= (const TimeStamp &other) const
{
  return itsTime >= other.itsTime;
}

inline bool TimeStamp::operator < (const TimeStamp &other) const
{ 
  return itsTime < other.itsTime;
}

inline bool TimeStamp::operator <= (const TimeStamp &other) const
{
  return itsTime <= other.itsTime;
}

inline bool TimeStamp::operator == (const TimeStamp &other) const
{ 
  return itsTime == other.itsTime;
}

inline bool TimeStamp::operator != (const TimeStamp &other) const
{ 
  return itsTime != other.itsTime;
}

} // namespace CS1
} // namespace LOFAR

#endif

