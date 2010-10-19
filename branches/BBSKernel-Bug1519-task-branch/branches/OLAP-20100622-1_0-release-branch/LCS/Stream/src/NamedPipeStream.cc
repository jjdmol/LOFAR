//# NamedPipeStream.cc: 
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
//# $Id: NamedPipeStream.cc 15528 2010-04-22 14:08:39Z romein $

#include <lofar_config.h>

#include <Common/SystemCallException.h>
#include <Stream/NamedPipeStream.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace LOFAR {

NamedPipeStream::NamedPipeStream(const char *name)
:
  itsName(name)
{
  if (mknod(name, 0600 | S_IFIFO, 0) < 0 && errno != EEXIST)
    throw SystemCallException(std::string("mknod ") + name, errno, THROW_ARGS);

  if ((fd = open(name, O_RDWR)) < 0) {
    unlink(name);
    throw SystemCallException(std::string("open ") + name, errno, THROW_ARGS);
  }
}


NamedPipeStream::~NamedPipeStream()
{
  unlink(itsName.c_str());
}

} // namespace LOFAR
