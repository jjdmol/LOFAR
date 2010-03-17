//# Exceptions.h: Declaration and definition of SINFONI specific exceptions.
//#
//# Copyright (C) 2002-2006
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
//# $Id: Exceptions.h 14057 2009-09-18 12:26:29Z diepen $

#ifndef LOFAR_COMMON_EXCEPTIONS_H
#define LOFAR_COMMON_EXCEPTIONS_H

//# Includes
#include <Common/Exception.h>

namespace LOFAR
{
  //
  // This exception will be thrown when an assertion fails.
  //
  EXCEPTION_CLASS(AssertError,Exception);

  //
  // This exception will be thrown when an I/O error occurs.
  //
  EXCEPTION_CLASS(IOException, Exception);

  //
  // This exception will be thrown when a math error occurs.
  //
  EXCEPTION_CLASS(MathException, Exception);

  //
  // This exception will be thrown when a method is called that has not been
  // implemented (yet).
  //
  EXCEPTION_CLASS(NotImplemented, Exception);

  //
  // This exception will be thrown when an error occurs in the ParameterSet
  // classes.
  //
  EXCEPTION_CLASS(APSException, Exception);

} // namespace ASTRON

#endif
