//#  -*- mode: c++ -*-
//#  SubArraySubscription.cc: class implementation
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <APL/CAL_Protocol/SubArray.h>
#include "SubArraySubscription.h"
#include <APL/CAL_Protocol/CAL_Protocol.ph>

using namespace LOFAR;
using namespace CAL;
using namespace RTC;
using namespace CAL_Protocol;

void SubArraySubscription::update(Subject* subject)
{
  ASSERT(subject == static_cast<Subject*>(m_subarray));

  //AntennaGains calibratedGains;

  // get gains from the FRONT buffer
  //calibratedGains = m_subarray->getGains();
  CALUpdateEvent update;
  update.name = m_subarray->name();
  update.timestamp.setNow(0);
  update.status = CAL_SUCCESS;
  //update.handle = (memptr_t)this;
  update.gains = m_subarray->getGains();

  if (m_port.isConnected()) m_port.send(update);
}

