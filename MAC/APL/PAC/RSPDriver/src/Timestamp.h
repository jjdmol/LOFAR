//#  Timestamp.h: timestamp information for the RSP driver
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

#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <sys/time.h>
#include <string.h>

namespace RSP_Protocol
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
	  Timestamp(long seconds, long useconds) { m_tv.tv_sec = seconds; m_tv.tv_usec = useconds; }

	  /**
	   * Copy constructor.
	   */
	  Timestamp(const Timestamp& copy) { m_tv = copy.m_tv; m_tv.tv_usec = 0; }

	  /* Destructor for Timestamp. */
	  virtual ~Timestamp() {}

	  /**
	   * Set the timestamp value. The tv_usec part
	   * is currently initialized to 0. If and when
	   * the precision needs to be increased this will
	   * be modifed.
	   */
	  void set(const struct timeval& tv);
	  
	  /*@{*/
	  /**
	   * Set the timestamp value to the current time (now)
	   * plus an optional delay.
	   */
	  void setNow(double delay = 0.0);
	  /*@}*/

	  /**
	   * Get the timestamp value. If the pointer is 0
	   * this method does nothing.
	   */
	  void get(struct timeval* tv);

	  /*@{*/
	  /**
	   * Overloaded operators.
	   */
	  Timestamp& operator=(const Timestamp& rhs);
	  Timestamp  operator+(long delay) const;
	  bool       operator>(const Timestamp& rhs) const;
	  bool       operator==(const Timestamp& rhs) const;
	  /*@}*/

	  /*@{*/
	  /**
	   * Return parts.
	   */
	  long sec();
	  long usec();
	  /*@}*/

      public:
	  /*@{*/
	  /**
	   * marshalling methods
	   */
	  unsigned int getSize();
	  unsigned int pack  (void* buffer);
	  unsigned int unpack(void *buffer);
	  /*@}*/

      private:
	  /**
	   * The current precision of a timestamp is 1 second.
	   * @note By using a 'struct timeval' there is the option to
	   * go to a precision of microseconds.
	   */
	  struct timeval m_tv;
      };

  inline void Timestamp::set(const struct timeval& tv)  { m_tv = tv; m_tv.tv_usec = 0; }
  inline void Timestamp::get(struct timeval *tv) { if (tv) *tv = m_tv;          }
  inline unsigned int Timestamp::getSize()            { return sizeof(struct timeval); }
  inline unsigned int Timestamp::pack  (void* buffer) { memcpy(buffer, &m_tv, sizeof(struct timeval)); return sizeof(struct timeval); }
  inline unsigned int Timestamp::unpack(void *buffer) { memcpy(&m_tv, buffer, sizeof(struct timeval)); return sizeof(struct timeval); }
  inline Timestamp& Timestamp::operator=(const Timestamp& rhs) { m_tv = rhs.m_tv; m_tv.tv_usec = 0; return *this; }
  inline Timestamp Timestamp::operator+(long delay) const { Timestamp ts(m_tv); ts.m_tv.tv_sec += delay; return ts; }
  inline bool Timestamp::operator>(const Timestamp& rhs) const { return (m_tv.tv_sec == rhs.m_tv.tv_sec ? m_tv.tv_usec > rhs.m_tv.tv_usec : (m_tv.tv_usec > rhs.m_tv.tv_usec ? true : false)); }
  inline bool Timestamp::operator==(const Timestamp& rhs) const { return (m_tv.tv_sec == rhs.m_tv.tv_sec && m_tv.tv_usec == rhs.m_tv.tv_usec); }
  inline long Timestamp::sec()  { return m_tv.tv_sec;  }
  inline long Timestamp::usec() { return m_tv.tv_usec; }
};

#define SECONDS(s)  Timestamp((s),0)
#define USECONDS(u) Timestamp(0,(u))
     
#endif /* TIMESTAMP_H_ */
