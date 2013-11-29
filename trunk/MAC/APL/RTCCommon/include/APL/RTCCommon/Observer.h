//#  -*- mode: c++ -*-
//#
//#  Observer.h: Observer/Subject pattern class.
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

#ifndef OBSERVER_H_
#define OBSERVER_H_

namespace LOFAR {
  namespace RTC {

    class Subject; // forward declaration

    /**
     * Observer class in the Observer design pattern.
     */
    class Observer
      {
      public:
	virtual ~Observer() {}

	/**
	 * This is called by the subject to
	 * indicate that the subject has changed.
	 * @param subject Pointer to the subject
	 * can be used to query the subject for
	 * the actual changes.
	 */
	virtual void update(Subject* subject) = 0;

      protected:
	Observer() {}
      };

  }; // namespace RTC
}; // namespace LOFAR

#endif /* OBSERVER_H_ */
