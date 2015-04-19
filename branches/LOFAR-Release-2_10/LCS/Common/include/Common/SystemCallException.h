//# SystemCallException.h: 
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

#ifndef LOFAR_LCS_COMMON_SYSTEM_CALL_EXCEPTION_H
#define LOFAR_LCS_COMMON_SYSTEM_CALL_EXCEPTION_H

#include <Common/Exception.h>

#include <cerrno>
#include <string>


namespace LOFAR {

// Use THROW_SYSCALL(...) below instead of 'throw SystemCallException(...)'
// to enforce sampling errno right after the syscall before it can be clobbered.
// (If an errno was already avail (e.g. pthread functions return it), throw ... is fine.)
#define THROW_SYSCALL(msg) \
  do { \
    int err = errno; \
    throw LOFAR::SystemCallException(msg, err, THROW_ARGS); \
  } while (0)

class SystemCallException : public Exception
{
  public:
			      SystemCallException(const std::string &syscall,
						  int error = errno,
						  const std::string &file = "",
						  int line = 0,
						  const std::string &func = "",
						  Backtrace *bt = 0) throw();

    virtual		      ~SystemCallException() throw();
    virtual const std::string &type() const;

    const std::string&        syscall() const;
    
    const int		      error;

  private:
    static std::string	      errorMessage(int error);

    const std::string         syscallName;
};

} // namespace LOFAR

#endif

