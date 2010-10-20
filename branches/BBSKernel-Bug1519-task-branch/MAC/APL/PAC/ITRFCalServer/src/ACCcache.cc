//#  -*- mode: c++ -*-
//#  ACCcache.cc: implementation of the Auto Correlation Cube class
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
//#  $Id: ACC.cc 6967 2005-10-31 16:28:09Z wierenga $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>

#include "ACCcache.h"

namespace LOFAR {
  namespace ICAL {

ACCcache::ACCcache() : 
	itsIsSwapped(false)
{
	itsBackCache  = new ACC(MAX_SUBBANDS, MAX_RCUS);
	itsFrontCache = new ACC(MAX_SUBBANDS, MAX_RCUS);
}

ACCcache::~ACCcache()
{
	delete itsBackCache;
	delete itsFrontCache;
}

void ACCcache::swap()
{
	ACC*	tmp = itsFrontCache;
	itsFrontCache = itsBackCache;
	itsBackCache = tmp;
	itsIsSwapped = !itsIsSwapped;
}

  } // namespace ICAL
} // namespace LOFAR
