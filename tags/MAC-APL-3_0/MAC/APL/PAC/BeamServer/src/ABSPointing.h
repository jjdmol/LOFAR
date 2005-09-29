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
#include <time.h>
#include <sys/time.h>

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
	  Pointing(const Direction direction, time_t time);
	  virtual ~Pointing();
	  //@}

	  //@{
	  /**
	   * 'set' methods to set the time and
	   * direction of a pointing.
	   */
	  void setDirection(const Direction direction);
	  void setTime(time_t time);
          //@}

	  //@{
	  /**
	   * Accessor methods. Get the time and
	   * direction of a pointing.
	   */
	  Direction direction() const;
	  time_t    time()      const;
	  bool      isTimeSet() const;
          //@}

	  /**
	   * Compare the time of two pointings.
	   */
	  bool operator<(Pointing const & right) const;

      private:
	  Direction m_direction;
	  time_t    m_time;
      };

  inline void      Pointing::setTime(time_t time) { m_time = time; }
  inline void      Pointing::setDirection(const Direction direction) { m_direction = direction; }
  inline time_t    Pointing::time() const                      { return m_time;  }
  inline bool      Pointing::isTimeSet() const                 { return m_time != 0; }
  inline Direction Pointing::direction() const                 { return m_direction;  }
  inline bool      Pointing::operator<(Pointing const & right) const
      {
	// inverse priority, earlier times are at the front of the queue
	return (m_time > right.m_time);
      }
};

#endif /* ABSPOINTING_H_ */
