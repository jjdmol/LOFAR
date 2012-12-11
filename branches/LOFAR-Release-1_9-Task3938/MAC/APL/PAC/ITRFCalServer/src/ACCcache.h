//#  -*- mode: c++ -*-
//#  ACCcache.h: definition of the Auto Correlation Cube class
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
//#  $Id: ACCcache.h 6967 2005-10-31 16:28:09Z wierenga $

#ifndef ACC_CACHE_H_
#define ACC_CACHE_H_

#include "ACC.h"
#include "SharedResource.h"

namespace LOFAR {
  namespace ICAL {

	/**
	* Factory class for ACC (Array Correlation Cube) instances.
	* This class manages ACC instances: the front and back ACC buffers.
	* The calibration algorithm works with the front ACC buffer while 
	* in the background the next ACC buffer is filled by the CAL::ACCService.
	*/
class ACCcache : public SharedResource
{
public:
	ACCcache();
	~ACCcache();

	// @return a reference to the front ACC buffer. This buffer can be used
	// to pass to the calibration algorithm.
	ACC& getFront() const { return (*itsFrontCache); }

	// @return a reference to the back ACC buffer. This buffer can be filled
	// for the next calibration iteration.
	ACC& getBack() const  { return (*itsBackCache); }

	// Swap the front and back buffers after the calibration has completed and
	// the back buffer is filled with the most recent ACC for use in the next
	// calibration iteration.
	void swap();

private:
	ACC*	itsBackCache;
	ACC*	itsFrontCache;
	bool	itsIsSwapped;
	
};

  }; // namespace ICAL
}; // namespace LOFAR

#endif /* ACC_CACHE_H_ */

