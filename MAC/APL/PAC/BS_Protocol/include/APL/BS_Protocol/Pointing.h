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

#include "Timestamp.h"

namespace LOFAR {
  namespace BS {

    /**
     * Class with 
     */
    class Pointing
      {
      public:
	/**
	 * A direction can have one of these three types.
	 */
	enum Type {
	  J2000 = 1,
	  AZEL = 2,

	  /**
	   * The LOFAR station local coordinate system.
	   * For the definition of this coordinate system see [ref].
	   */
	  LOFAR_LMN = 3,
	};

	//@{
	/**
	 * Constructors and destructors for a pointing.
	 */
	Pointing();
	Pointing(double angle1, double angle2, RTC::Timestamp time, Type type);
	virtual ~Pointing();
	//@}

	//@{
	/**
	 * 'set' methods to set the time and
	 * direction of a pointing.
	 */
	void setDirection(double angle1, double angle2);
	void setTime(RTC::Timestamp time);
	void setType(Type type);
	//@}

	//@{
	/**
	 * Accessor methods. Get the time and
	 * direction of a pointing.
	 */
	double         angle1()    const;
	double         angle2()    const;
	RTC::Timestamp time()      const;
	bool           isTimeSet() const;
	//@}

	/**
	 * Get the type of pointing.
	 */
	Type      getType() const;

	/**
	 * Compare the time of two pointings.
	 * Needed for priority queue of pointings.
	 */
	bool operator<(Pointing const & right) const;

      private:
	double         m_angle1;
	double         m_angle2;
	RTC::Timestamp m_time;
	Type           m_type;
      };

    inline void   Pointing::setTime(RTC::Timestamp time) { m_time = time; }
    inline void   Pointing::setType(Type type) { m_type = type; }
    inline void   Pointing::setDirection(double angle1, double angle2) { m_angle1 = angle1; m_angle2 = angle2; }
    inline RTC::Timestamp Pointing::time() const      { return m_time;  }
    inline bool   Pointing::isTimeSet() const { return !(0 == m_time.sec() && 0 == m_time.usec()); }
    inline Pointing::Type Pointing::getType() const   { return m_type; }
    inline double Pointing::angle1() const    { return m_angle1; }
    inline double Pointing::angle2() const    { return m_angle2; }
    inline bool   Pointing::operator<(Pointing const & right) const
      {
        // inverse priority, earlier times are at the front of the queue
        return (m_time > right.m_time);
      }
  };
};

#endif /* POINTING_H_ */
