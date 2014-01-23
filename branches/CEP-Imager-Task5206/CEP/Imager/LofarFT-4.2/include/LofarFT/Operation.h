//# Command.h: Base class for commands
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

#ifndef LOFAR_LOFARFT_COMMAND_H
#define LOFAR_LOFARFT_COMMAND_H

// \file

#include <Common/ObjectFactory.h>
#include <Common/Singleton.h>
#include <Common/lofar_string.h>

namespace LOFAR
{

  class Operation
  {
  public:
    // Destructor.
    virtual ~Command() {}

    // Return the operation type of \c *this as a string.
    virtual const string& type() const = 0;

    virtual CommandResult run() const = 0;

  };

  // Factory that can be used to generate new Operation objects.
  // The factory is defined as a singleton.
  typedef Singleton< ObjectFactory< Operation*(), String > > OperationFactory;

} //# namespace LOFAR

#endif
