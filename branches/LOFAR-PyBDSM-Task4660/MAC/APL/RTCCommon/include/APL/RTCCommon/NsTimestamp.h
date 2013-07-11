//#  -*- mode: c++ -*-
//#
//#  NsTimestamp.h: timestamp information up to nano-seconds
//#
//#  Copyright (C) 2011
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
//#  $Id: NsTimestamp.h 16487 2010-10-06 15:06:18Z overeem $

#ifndef NSTIMESTAMP_H_
#define NSTIMESTAMP_H_

#include <Common/LofarTypes.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <iostream>
#include <limits.h>
#include <cmath>

namespace LOFAR {
  namespace RTC
{
class NsTimestamp
{
public:
	// Constructors for a NsTimestamp object.
	// Currently the tv_usec part is always set to 0 irrespective
	// of the value passed in.
	NsTimestamp() { itsSec = 0; itsNsec = 0; }
	explicit NsTimestamp(struct timeval tv) : itsSec(tv.tv_sec), itsNsec(1000 * tv.tv_usec) {};
	NsTimestamp(long seconds, long nseconds) : itsSec(seconds), itsNsec(nseconds%1000000000) {};
	explicit NsTimestamp(double UTCtime) : itsSec((int64)trunc(UTCtime)), itsNsec((int64)((UTCtime-trunc(UTCtime))*1e9)) {};

	// Copy constructor.
	NsTimestamp(const NsTimestamp& copy) { itsSec = copy.itsSec; itsNsec = copy.itsNsec; }

	// Destructor for NsTimestamp.
	~NsTimestamp() {}

	/*@{*/
	// Set the timestamp value to the current time (now)
	// plus an optional delay in seconds.
	void setNow(double delay = 0.0);
	/*@}*/

	// Get the timestamp value. If the pointer is 0
	// this method does nothing.
	void get(struct timeval* tv) const;

	// Get instance of timestamp with current time.
	static NsTimestamp now(double delay = 0.0) { NsTimestamp t; t.setNow(delay); return t; }

	/*@{*/
	// Overloaded operators.
	NsTimestamp& operator=(const NsTimestamp& rhs);
	NsTimestamp  operator-(double delay) const;
	NsTimestamp  operator+(double delay) const;
	NsTimestamp& operator+=(double delay);
	NsTimestamp& operator-=(double delay);
	bool       operator>(const NsTimestamp& rhs) const;
	bool       operator<(const NsTimestamp& rhs) const;
	bool       operator<=(const NsTimestamp& rhs) const;
	bool       operator>=(const NsTimestamp& rhs) const;
	bool       operator==(const NsTimestamp& rhs) const;
	bool       operator!=(const NsTimestamp& rhs) const;
	/*@}*/

	/*@{*/
	// Conversion operators.
	operator double() const;
	void set(double newTime);
	/*@}*/

	/*@{*/
	// Return parts.
	long sec()  const;
	long nsec() const;
	static NsTimestamp maxTime() { return (NsTimestamp(INT_MAX, 999999999)); };
	/*@}*/

	/*@{*/
	// marshalling methods
	size_t getSize();
	size_t pack  (char* buffer) const;
	size_t unpack(const char *buffer);
	/*@}*/

private:
	// The current precision of a timestamp is 1 second.
	// @note By using a 'struct timeval' there is the option to
	// go to a precision of microseconds.
	int64	itsSec;
	int64	itsNsec;
};

/*@{*/
// Inline methods.
inline void NsTimestamp::get(struct timeval *tv) const
{ 
	if (tv) { 
		tv->tv_sec = itsSec;  
		tv->tv_usec = (long)((itsNsec+500)/1000);	// round
	} 
}

inline void NsTimestamp::set(double UTCtime)
{
	itsSec = (int64)trunc(UTCtime);
	itsNsec= (int64)((UTCtime-trunc(UTCtime))*1e9);
}

inline NsTimestamp& NsTimestamp::operator=(const NsTimestamp& rhs)
{
	itsSec = rhs.itsSec;
	itsNsec = rhs.itsNsec;
	return (*this);
}

inline NsTimestamp NsTimestamp::operator-(double delay) const
{
	return (NsTimestamp((double)(*this) - delay));
}

inline NsTimestamp NsTimestamp::operator+(double delay) const
{
	return (NsTimestamp((double)(*this) + delay));
}

inline NsTimestamp& NsTimestamp::operator+=(double delay)
{
	set((double)(*this) + delay);
	return (*this);
}

inline NsTimestamp& NsTimestamp::operator-=(double delay)
{
	set((double)(*this) - delay);
	return (*this);
}

inline NsTimestamp::operator double() const
{
	return (static_cast<double>(itsSec)+(static_cast<double>(itsNsec)/1e9));
}

# define NSTIMERCMP(a, b, CMP) \
  (((a).itsSec == (b).itsSec) ? ((a).itsNsec CMP (b).itsNsec) :  ((a).itsSec CMP (b).itsSec))

inline bool NsTimestamp::operator>(const NsTimestamp& rhs) const
{
	return(NSTIMERCMP(*this, rhs, >));
}

inline bool NsTimestamp::operator<(const NsTimestamp& rhs) const
{
	return(NSTIMERCMP(*this, rhs, <));
}

inline bool NsTimestamp::operator<=(const NsTimestamp& rhs) const
{
	return(NSTIMERCMP(*this, rhs, ==) || NSTIMERCMP(*this, rhs, <));
}

inline bool NsTimestamp::operator>=(const NsTimestamp& rhs) const
{
	return(NSTIMERCMP(*this, rhs, ==) || NSTIMERCMP(*this, rhs, >));
}

inline bool NsTimestamp::operator==(const NsTimestamp& rhs) const
{
	return(NSTIMERCMP(*this, rhs, ==));
}

inline bool NsTimestamp::operator!=(const NsTimestamp& rhs) const
{
	return(NSTIMERCMP(*this, rhs, !=));
}
inline long NsTimestamp::sec()   const { return itsSec;  }
inline long NsTimestamp::nsec() const { return itsNsec; }

inline size_t NsTimestamp::getSize()
{
	return (2 * sizeof(int64));
}   

inline size_t NsTimestamp::pack  (char* __buffer) const
{   
	size_t	__valSize(sizeof(int64));
	memcpy(__buffer, &itsSec, __valSize);
	memcpy((char*)__buffer+__valSize, &itsNsec, __valSize);
	return (2*__valSize);
}   
      
inline size_t NsTimestamp::unpack(const char *__buffer)
{   
	size_t	__valSize(sizeof(int64));
	memcpy(&itsSec, __buffer, __valSize);
	memcpy(&itsNsec, __buffer + __valSize, __valSize);
	return (2*__valSize);
}   


/*@}*/

std::ostream& operator<< (std::ostream& os, const LOFAR::RTC::NsTimestamp& ts);

  }; // namespace RTC
}; // namespace LOFAR

#endif /* NSTIMESTAMP_H_ */
