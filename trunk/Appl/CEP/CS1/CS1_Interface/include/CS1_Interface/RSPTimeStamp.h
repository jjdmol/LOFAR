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
#include <Common/DataConvert.h>

#define EVEN_SECOND_HAS_MORE_SAMPLES

namespace LOFAR {

  // This is needed to be able to put the TimeStamp in a Blob
  namespace CS1 {
    class TimeStamp;
  }
  void dataConvert(DataFormat fmt, CS1::TimeStamp* inout, uint nrval);

  namespace CS1 {

    class TimeStamp {
    public:
      TimeStamp(int64 time = 0);
      TimeStamp(unsigned seqId, unsigned blockId);

      TimeStamp	    &setStamp(unsigned seqId, unsigned blockId);
      unsigned	    getSeqId() const;
      unsigned	    getBlockId() const;
      static double getMaxBlockId();
      static void   setMaxBlockId(double nMBID);

      template <typename T> TimeStamp &operator += (T increment);
      template <typename T> TimeStamp &operator -= (T decrement);
			    TimeStamp  operator ++ (int); // postfix
			    TimeStamp &operator ++ ();	  // prefix
			    TimeStamp  operator -- (int);
			    TimeStamp &operator -- ();

      template <typename T> TimeStamp  operator +  (T) const;
      template <typename T> TimeStamp  operator -  (T) const;
			    int64      operator -  (const TimeStamp &) const;

			    bool       operator >  (const TimeStamp &) const;
			    bool       operator <  (const TimeStamp &) const;
			    bool       operator >= (const TimeStamp &) const;
			    bool       operator <= (const TimeStamp &) const;
			    bool       operator == (const TimeStamp &) const;
			    bool       operator != (const TimeStamp &) const;

				       operator int64 () const;

      friend ostream &operator << (ostream &os, const TimeStamp &ss);

      // This is needed to be able to put the TimeStamp in a Blob
      friend void ::LOFAR::dataConvert(DataFormat fmt, TimeStamp* inout, uint nrval);

    protected:
      int64	    itsTime;
      static double theirMaxBlockId, theirInvMaxBlockId;
    };

  } // namespace CS1

  // This is needed to be able to put the TimeStamp in a Blob
  inline void dataConvert(DataFormat fmt, CS1::TimeStamp* inout, uint nrval)
    {
      for (uint i=0; i<nrval ;i++) {
 	dataConvert64(fmt, &(inout[i].itsTime));
      }
    }

  namespace CS1 {

    typedef TimeStamp timestamp_t;

    inline TimeStamp::TimeStamp(int64 time)
      {
	itsTime = time;
      }

    inline TimeStamp::TimeStamp(unsigned seqId, unsigned blockId)
      {
#ifdef EVEN_SECOND_HAS_MORE_SAMPLES
	itsTime = (int64) (seqId * theirMaxBlockId + .5) + blockId;
#else
	itsTime = (int64) (seqId * theirMaxBlockId) + blockId;
#endif
      }

    inline TimeStamp &TimeStamp::setStamp(unsigned seqId, unsigned blockId)
      {
#ifdef EVEN_SECOND_HAS_MORE_SAMPLES
	itsTime = (int64) (seqId * theirMaxBlockId + .5) + blockId;
#else
	itsTime = (int64) (seqId * theirMaxBlockId) + blockId;
#endif
	return *this;
      }

    inline unsigned TimeStamp::getSeqId() const
      {
#ifdef EVEN_SECOND_HAS_MORE_SAMPLES
	return (unsigned) (itsTime * theirInvMaxBlockId);
#else
	return (unsigned) ((itsTime - 0.5) * theirInvMaxBlockId);
#endif
      }

    inline unsigned TimeStamp::getBlockId() const
      {
#ifdef EVEN_SECOND_HAS_MORE_SAMPLES
	return itsTime - (int64) (theirMaxBlockId * getSeqId() + .5);
#else
	return itsTime - (int64) (theirMaxBlockId * getSeqId());
#endif
      }

    inline double TimeStamp::getMaxBlockId()
      {
	return theirMaxBlockId;
      }

    inline void TimeStamp::setMaxBlockId(double nMBID)
      {
	theirMaxBlockId    = nMBID;
	theirInvMaxBlockId = 1 / nMBID;
      }

    template <typename T> inline TimeStamp &TimeStamp::operator += (T increment)
      { 
	itsTime += increment;
	return *this;
      }

    template <typename T> inline TimeStamp &TimeStamp::operator -= (T decrement)
      { 
	itsTime -= decrement;
	return *this;
      }

    inline TimeStamp &TimeStamp::operator ++ ()
      { 
	++ itsTime;
	return *this;
      }

    inline TimeStamp TimeStamp::operator ++ (int)
      { 
        TimeStamp tmp = *this;
	++ itsTime;
	return tmp;
      }

    inline TimeStamp &TimeStamp::operator -- ()
      { 
	-- itsTime;
	return *this;
      }

    inline TimeStamp TimeStamp::operator -- (int)
      { 
	TimeStamp tmp = *this;
	-- itsTime;
	return tmp;
      }

    template <typename T> inline TimeStamp TimeStamp::operator + (T increment) const
      { 
	return TimeStamp(itsTime + increment);
      }

    template <typename T> inline TimeStamp TimeStamp::operator - (T decrement) const
      { 
	return TimeStamp(itsTime - decrement);
      }

    inline int64 TimeStamp::operator - (const TimeStamp &other) const
      { 
	return itsTime - other.itsTime;
      }

    inline TimeStamp::operator int64 () const
      {
	return itsTime;
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

