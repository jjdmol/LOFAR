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

#ifndef ABSPOINTING_H_
#define ABSPOINTING_H_

#include "ABSDirection.h"
#include <sys/time.h>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace ABS
{

  /**
   * Class with 
   */
  class Pointing
      {
      public:
	  //@{
	  /**
	   * Constructors and destructors for a pointing.
	   */
	  Pointing();
	  Pointing(const Direction direction, boost::posix_time::ptime);
	  virtual ~Pointing();
	  //@}

	  //@{
	  /**
	   * 'set' methods to set the time and
	   * direction of a pointing.
	   */
	  void setDirection(const Direction direction);
	  void setTime(boost::posix_time::ptime time);
          //@}

	  //@{
	  /**
	   * Accessor methods. Get the time and
	   * direction of a pointing.
	   */
	  Direction direction()  const;
	  boost::posix_time::ptime time()  const;
	  bool isTimeSet() const;
          //@}

	  /**
	   * Compare the time of two pointings.
	   */
	  bool operator<(Pointing const & right) const;

      private:
	  Direction                m_direction;
	  boost::posix_time::ptime m_time;
      };

  inline void                     Pointing::setTime(boost::posix_time::ptime time)      { m_time      = time; }
  inline void                     Pointing::setDirection(const Direction direction) { m_direction = direction; }
  inline boost::posix_time::ptime Pointing::time() const                      { return m_time;  }
  inline bool                     Pointing::isTimeSet() const                 { return m_time != boost::posix_time::ptime(boost::gregorian::date(1970,1,1)); }
  inline Direction                Pointing::direction() const                 { return m_direction;  }
  inline bool Pointing::operator<(Pointing const & right) const
      {
	// inverse priority, earlier times are at the front of the queue
	return (m_time > right.m_time);
      }
};

#endif /* ABSPOINTING_H_ */
