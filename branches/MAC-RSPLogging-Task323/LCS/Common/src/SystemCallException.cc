//# SystemCallException.cc: 
//#
//# Copyright (C) 2008
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

#include <Common/SystemCallException.h>

#include <cstring>


namespace LOFAR {

SystemCallException::SystemCallException(const std::string &syscall, int error, const std::string& file, int line, 
			const std::string& func, Backtrace* bt) throw()
: Exception(syscall + ": " + errorMessage(error), file, line, func, bt),
  error(error)
{
}


SystemCallException::~SystemCallException() throw()
{
}


const std::string& SystemCallException::type() const
{
  static const std::string theType("SystemCallException");
  return theType;
}


std::string SystemCallException::errorMessage(int error)
{
  char buffer[128];

  // there are two incompatible versions of versions of strerror_r()

#if !__linux__ || (!_GNU_SOURCE && (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600))
  if (strerror_r(error, buffer, sizeof buffer) == 0)
    return std::string(buffer);
  else
    return "could not convert error to string";
#else
  return strerror_r(error, buffer, sizeof buffer);
#endif
}


} // namespace LOFAR
