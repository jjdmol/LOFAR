//#  -*- mode: c++ -*-
//#
//#  Timestamp.h: timestamp information for the RSP driver
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <iostream>
#include <limits.h>
#include <cmath>

namespace LOFAR {
namespace RTC
{
  class Timestamp
      {
      public:
	  /**
	   * Constructors for a Timestamp object.
	   * Currently the tv_usec part is always set to 0 irrespective
	   * of the value passed in.
	   */
	  Timestamp() { m_tv.tv_sec = 0; m_tv.tv_usec = 0; }
	  explicit Timestamp(struct timeval tv) : m_tv(tv) { m_tv.tv_usec = 0; }
	  Timestamp(long seconds, long useconds) { m_tv.tv_sec = seconds; m_tv.tv_usec = (useconds%1000000); }
	  explicit Timestamp(double UTCtime) { set(UTCtime); }

	  /**
	   * Copy constructor.
	   */
	  Timestamp(const Timestamp& copy) { m_tv = copy.m_tv; } // m_tv.tv_usec = 0; }

	  /* Destructor for Timestamp. */
	  virtual ~Timestamp() {}

	  /**
	   * Set the timestamp value. The tv_usec part
	   * is currently initialized to 0. If and when
	   * the precision needs to be increased this will
	   * be modifed.
	   */
	  void set(const struct timeval& tv);
	  void set(const double dbl);
	  
	  /*@{*/
	  /**
	   * Set the timestamp value to the current time (now)
	   * plus an optional delay in seconds.
	   */
	  void setNow(double delay = 0.0);
	  /*@}*/

	  /**
	   * Get the timestamp value. If the pointer is 0
	   * this method does nothing.
	   */
	  void get(struct timeval* tv) const;

	  /**
	   * Convert to modified Julian Date for use in AMC::TimeCoord constructor.
	   */
	  void convertToMJD(double& mjd, double& fraction);

	  /**
	   * Get instance of timestamp with current time.
	   */
	  static Timestamp now(double delay = 0.0) { Timestamp t; t.setNow(delay); return t; }

	  /*@{*/
	  /**
	   * Overloaded operators.
	   */
	  Timestamp& operator=(const Timestamp& rhs);
	  Timestamp  operator-(long delay) const;
	  Timestamp  operator+(long delay) const;
	  Timestamp& operator-=(long delay);
	  Timestamp& operator+=(long delay);
	  Timestamp  operator-(double delay) const;
	  Timestamp  operator+(double delay) const;
	  Timestamp& operator-=(double delay);
	  Timestamp& operator+=(double delay);
	  bool       operator>(const Timestamp& rhs) const;
	  bool       operator<(const Timestamp& rhs) const;
	  bool       operator<=(const Timestamp& rhs) const;
	  bool       operator>=(const Timestamp& rhs) const;
	  bool       operator==(const Timestamp& rhs) const;
	  bool       operator!=(const Timestamp& rhs) const;
	  /*@}*/
    
    /*@{*/
    /**
     * Conversion operators.
     */
               operator double() const;
    /*@}*/

	  /*@{*/
	  /**
	   * Return parts.
	   */
	  long sec()  const;
	  long usec() const;
	  static Timestamp maxTime() { return (Timestamp(INT_MAX, 999999)); };
	  /*@}*/

public:
	  /*@{*/
	  /**
	   * marshalling methods
	   */
	  size_t getSize() const;
	  size_t pack  (char* buffer) const;
	  size_t unpack(const char *buffer);
	  /*@}*/

private:
	  /**
	   * The current precision of a timestamp is 1 second.
	   * @note By using a 'struct timeval' there is the option to
	   * go to a precision of microseconds.
	   */
	  struct timeval m_tv;
      };

  /*@{*/
  /**
   * Inline methods.
   */
  inline void Timestamp::set(const struct timeval& tv)  { m_tv = tv; }
  inline void Timestamp::set(const double UTCtime) {
	m_tv.tv_sec = (long)trunc(UTCtime);
	m_tv.tv_usec= (long)((UTCtime-trunc(UTCtime))*1e6);
  }
  inline void Timestamp::get(struct timeval *tv) const { if (tv) *tv = m_tv; }

  inline size_t Timestamp::getSize() const
  {
    return sizeof(struct timeval);
  }

  inline size_t Timestamp::pack  (char* buffer) const
  {
    memcpy(buffer, &m_tv, sizeof(struct timeval));
    return sizeof(struct timeval);
  }

  inline size_t Timestamp::unpack(const char *buffer)
  {
    memcpy(&m_tv, buffer, sizeof(struct timeval));
    return sizeof(struct timeval);
  }

  inline Timestamp& Timestamp::operator=(const Timestamp& rhs)
  {
    m_tv = rhs.m_tv;
//    m_tv.tv_usec = 0;
    return *this;
  }

  inline Timestamp Timestamp::operator-(long delay) const
  {
    Timestamp ts(m_tv);
    ts.m_tv.tv_sec -= delay;
    return ts;
  }

  inline Timestamp Timestamp::operator+(long delay) const
  {
    Timestamp ts(m_tv);
    ts.m_tv.tv_sec += delay;
    return ts;
  }

  inline Timestamp& Timestamp::operator-=(long delay)
  {
	m_tv.tv_sec -= delay;
	return (*this);
  }

  inline Timestamp& Timestamp::operator+=(long delay)
  {
	m_tv.tv_sec += delay;
	return (*this);
  }

inline Timestamp Timestamp::operator-(double delay) const
{
	return (Timestamp((double)(*this) - delay));
}

inline Timestamp Timestamp::operator+(double delay) const
{
	return (Timestamp((double)(*this) + delay));
}

inline Timestamp& Timestamp::operator+=(double delay)
{
	set((double)(*this) + delay);
	return (*this);
}

inline Timestamp& Timestamp::operator-=(double delay)
{
	set((double)(*this) - delay);
	return (*this);
}

  inline bool Timestamp::operator>(const Timestamp& rhs) const
  {
    return timercmp(&m_tv, &rhs.m_tv, >);
  }

  inline bool Timestamp::operator<(const Timestamp& rhs) const
  {
    return timercmp(&m_tv, &rhs.m_tv, <);
  }

  inline bool Timestamp::operator<=(const Timestamp& rhs) const
  {
    return timercmp(&m_tv, &rhs.m_tv, ==) || timercmp(&m_tv, &rhs.m_tv, <);
  }

  inline bool Timestamp::operator>=(const Timestamp& rhs) const
  {
    return timercmp(&m_tv, &rhs.m_tv, ==) || timercmp(&m_tv, &rhs.m_tv, >);
  }

  inline bool Timestamp::operator==(const Timestamp& rhs) const
  {
    return timercmp(&m_tv, &rhs.m_tv, ==);
  }

  inline bool Timestamp::operator!=(const Timestamp& rhs) const
  {
    return timercmp(&m_tv, &rhs.m_tv, !=);
  }
  
  inline Timestamp::operator double() const
  {
    return (static_cast<double>(m_tv.tv_sec)+static_cast<double>(m_tv.tv_usec/1e6));
  }

  inline long Timestamp::sec()  const { return m_tv.tv_sec;  }
  inline long Timestamp::usec() const { return m_tv.tv_usec; }
  /*@}*/

  std::ostream& operator<< (std::ostream& os, const LOFAR::RTC::Timestamp& ts);

}; // namespace RTC

}; // namespace LOFAR

#define SECONDS(s)  Timestamp((s),0)
#define USECONDS(u) Timestamp(0,(u))
     
#endif /* TIMESTAMP_H_ */
