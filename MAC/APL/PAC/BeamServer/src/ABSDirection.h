//#  ABSDirection.h: interface of the Direction class
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

#ifndef ABSDIRECTION_H_
#define ABSDIRECTION_H_

namespace ABS
{

  /**
   * Class with 
   */
  class Direction
      {
      public:

	  /**
	   * A direction can have one of these three types.
	   */
	  enum Types {
	      J2000 = 1,
	      AZEL,

	      /**
	       * The LOFAR station local coordinate system.
	       * For the definition of this coordinate system see [ref].
	       */
	      LOFAR_LMN,
	  };

	  Direction(double angle1 = 0.0,
		    double angle2 = 0.0,
		    Types type = LOFAR_LMN);

	  virtual ~Direction();

	  //@{
	  /**
	   * 'set' methods to set the angles and the type of the
	   * direction.
	   */
	  void setAngle1(double angle);
	  void setAngle2(double angle);
	  void setType(Types type);
          //@}

	  //@{
	  /**
	   * Accessor methods. Get the angles and the type of the
	   * direction.
	   */
	  double angle1() const;
	  double angle2() const;
	  Types type() const;
          //@}

      private:
	  double m_angle1;
	  double m_angle2;
	  Types  m_type;
      };

  inline void             Direction::setAngle1(double angle) { m_angle1 = angle; }
  inline void             Direction::setAngle2(double angle) { m_angle2 = angle; }
  inline void             Direction::setType(Types type)     { m_type   = type;  }
  inline double           Direction::angle1() const          { return m_angle1;  }
  inline double           Direction::angle2() const          { return m_angle2;  }
  inline Direction::Types Direction::type()const             { return m_type;    }
};

#endif /* ABSDIRECTION_H_ */
