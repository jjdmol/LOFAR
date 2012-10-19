//#  -*- mode: c++ -*-
//#
//#  Subject.h: Observer/Subject pattern class.
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

#ifndef SUBJECT_H_
#define SUBJECT_H_

#include "Observer.h"
#include <set>

namespace LOFAR {
  namespace RTC {

    /**
     * Subject class of the Observer design pattern.
     */
    class Subject
      {
      public:
	virtual ~Subject();

	/**
	 * Attach a new observer.
	 * @param observer Pointer to the observer to attach.
	 */
	virtual void attach(Observer* observer);

	/**
	 * Detach an observer. The observer will be deleted.
	 * @param observer Pointer to the observer to detach and delete
	 * @return true if the observer has been removed, otherwise false 
	 */
	virtual bool detach(Observer* observer);

	/**
	 * Notify all observers that the subject has
	 * changed.
	 */
	virtual void notify();

      protected:
	Subject() {}

      private:
	std::set<Observer*> m_observers;
      };

  }; // namespace RTC
}; // namespace LOFAR

#endif /* SUBJECT_H_ */
