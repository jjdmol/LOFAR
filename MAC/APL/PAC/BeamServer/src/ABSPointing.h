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
	   * Constructors for a pointing.
	   */
	  Pointing();
	  Pointing(Direction direction, struct timeval time);
	  //@}

	  //@{
	  /**
	   * 'set' methods to set the time and
	   * direction of a pointing.
	   */
	  void setDirection(Direction direction);
	  void setTime(struct timeval time);
          //@}

	  //@{
	  /**
	   * Accessor methods. Get the time and
	   * direction of a pointing.
	   */
	  Direction direction() const;
	  struct timeval time() const;
          //@}

	  /**
	   * Compare the time of two pointings.
	   */
	  bool operator<(Pointing const & right) const;

      private:
	  Direction      m_direction;
	  struct timeval m_time;
      };

  inline void           Pointing::setTime(struct timeval time)      { m_time      = time; }
  inline void           Pointing::setDirection(Direction direction) { m_direction = direction; }
  inline struct timeval Pointing::time() const                      { return m_time;  }
  inline Direction      Pointing::direction() const                 { return m_direction;  }
  inline bool Pointing::operator<(Pointing const & right) const
      {
	  return ((m_time.tv_sec < right.time().tv_sec ? true :
		   (m_time.tv_usec < right.time().tv_usec ? true : false)));
      }
};

#endif /* ABSPOINTING_H_ */
