//# tSenderId.h: 
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <BBSControl/SenderId.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_set.h>

using namespace LOFAR;
using namespace LOFAR::BBS;

int main()
{
  INIT_LOGGER("tSenderId");
  try {
    {
      SenderId id;
      cout << id << endl;
      ASSERTSTR(!id.isValid(), id);
    }
    {
      SenderId id(SenderId::KERNEL, 1);
      cout << id << endl;
      ASSERTSTR(id.isValid(), id);
    }
    {
      SenderId id(static_cast<SenderId::Type>(3), 5);
      cout << id << endl;
      ASSERTSTR(!id.isValid(), id);
    }
    {
      SenderId id(SenderId::SOLVER, -10);
      cout << id << endl;
      ASSERTSTR(!id.isValid(), id);
    }
    set<SenderId> idSet;
    ASSERT(idSet.insert(SenderId(SenderId::KERNEL, 1)).second);
    ASSERT(idSet.insert(SenderId(SenderId::KERNEL, 2)).second);
    ASSERT(idSet.insert(SenderId(SenderId::SOLVER, 1)).second);
    ASSERT(idSet.insert(SenderId(SenderId::SOLVER, 2)).second);
    ASSERT(!idSet.insert(SenderId()).second);
    ASSERT(!idSet.insert(SenderId(static_cast<SenderId::Type>(3), 5)).second);
    for (set<SenderId>::const_iterator it = idSet.begin(); 
         it != idSet.end(); ++it) {
      cout << *it << endl;
    }
  }
  catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }
  return 0;
}
