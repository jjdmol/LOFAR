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
#include <Common/Thread/Cancellation.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace LOFAR {

NamedPipeStream::NamedPipeStream(const char *name, bool serverSide)
:
  itsReadName(std::string(name) + (serverSide ? "-0" : "-1")),
  itsWriteName(std::string(name) + (serverSide ? "-1" : "-0")),
  itsReadStream(0),
  itsWriteStream(0)
{
  try {
    if (mknod(itsReadName.c_str(), 0600 | S_IFIFO, 0) < 0 && errno != EEXIST)
      throw SystemCallException(std::string("mknod ") + itsReadName, errno, THROW_ARGS);

    if (mknod(itsWriteName.c_str(), 0600 | S_IFIFO, 0) < 0 && errno != EEXIST)
      throw SystemCallException(std::string("mknod ") + itsWriteName, errno, THROW_ARGS);

    itsReadStream = new FileStream(itsReadName.c_str(), O_RDWR, 0600); // strange; O_RDONLY hangs ???
    itsWriteStream = new FileStream(itsWriteName.c_str(), O_RDWR, 0600);
  } catch (...) {
    cleanUp();
    throw;
  }
}


NamedPipeStream::~NamedPipeStream()
{
  ScopedDelayCancellation dc; // unlink is a cancellation point

  cleanUp();
}


void NamedPipeStream::cleanUp()
{
  unlink(itsReadName.c_str());
  unlink(itsWriteName.c_str());
  delete itsReadStream;
  delete itsWriteStream;
}


size_t NamedPipeStream::tryRead(void *ptr, size_t size)
{
  return itsReadStream->tryRead(ptr, size);
}


size_t NamedPipeStream::tryWrite(const void *ptr, size_t size)
{
  return itsWriteStream->tryWrite(ptr, size);
}


void NamedPipeStream::sync()
{
  itsWriteStream->sync();
}


} // namespace LOFAR
