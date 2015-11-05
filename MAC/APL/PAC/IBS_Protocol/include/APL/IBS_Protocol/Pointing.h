//#  ABSPointing.h: interface of the Pointing class
//#
//#  Copyright (C) 2002-2004
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

#ifndef POINTING_H_
#define POINTING_H_

#include <APL/RTCCommon/Timestamp.h>

namespace LOFAR {
  namespace IBS_Protocol {

class Pointing
{
public:
	//@{
	// Constructors and destructors for a pointing.
	Pointing();
	Pointing(double angle0, double angle1, const string& type, RTC::Timestamp time, uint duration = 0);
	~Pointing();
	//@}

	//@{
	// 'set' methods to set the time and
	// direction of a pointing.
	void setDirection(double angle0, double angle1);
	void setTime	 (RTC::Timestamp time);
	void setDuration (uint duration);
	void setType	 (const string& type);
	//@}

	//@{
	// Accessor methods. Get the time and
	// direction of a pointing.
	double         			angle0()    const;
	double         			angle1()    const;
	RTC::Timestamp 			time()      const;
	RTC::Timestamp			endTime()	const;
	uint					duration()	const;
	bool           			isTimeSet() const;
	string					getType()	const;
	//@}

	// Compare the time of two pointings.
	// Needed for priority queue of pointings.
	bool operator<(Pointing const & right) const;

	// Check if the pointing overlap each other.
	bool overlap(const Pointing&	that) const;

	// Output function
	ostream& print (ostream& os) const;

	/*@{*/
	// marshalling methods
	unsigned int getSize();
	unsigned int pack  (void* buffer);
	unsigned int unpack(void *buffer);
	/*@}*/

private:
	double      	itsAngle2Pi;
	double      	itsAnglePi;
	RTC::Timestamp	itsTime;
	uint			itsDuration;
	string      	itsType;
};

inline void			  Pointing::setTime(RTC::Timestamp time) { itsTime = time; }
inline void			  Pointing::setType(const string& type)	 { itsType = type; }
inline void			  Pointing::setDirection(double angle0, double angle1) { itsAngle2Pi = angle0; itsAnglePi = angle1; }
inline void			  Pointing::setDuration(uint duration) { itsDuration = duration; }
inline RTC::Timestamp Pointing::time() const      { return (itsTime);  }
inline RTC::Timestamp Pointing::endTime() const   { return (itsDuration ? itsTime+(long)itsDuration : RTC::Timestamp::maxTime());  }
inline bool			  Pointing::isTimeSet() const { return (!(0 == itsTime.sec() && 0 == itsTime.usec())); }
inline string		  Pointing::getType() const   { return (itsType); }
inline double		  Pointing::angle0() const    { return (itsAngle2Pi); }
inline double		  Pointing::angle1() const    { return (itsAnglePi); }
inline uint			  Pointing::duration() const  { return (itsDuration); }

//
// operator<
//
inline bool   Pointing::operator<(Pointing const & right) const
{
	// inverse priority, earlier times are at the front of the queue
	return (itsTime < right.itsTime);
}

//
// operator<<
//
inline ostream& operator<< (ostream& os, const Pointing& pt) 
{
	return (pt.print(os));
}

  }; // namespace IBS_Protocol
}; // namespace LOFAR

#endif /* POINTING_H_ */
