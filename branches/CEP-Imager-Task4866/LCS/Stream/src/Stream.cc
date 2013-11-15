//# Stream.cc: 
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

#include <Stream/Stream.h>


namespace LOFAR {

Stream::~Stream()
{
}


void Stream::read(void *ptr, size_t size)
{
  while (size > 0) {
    size_t bytes = tryRead(ptr, size);

    size -= bytes;
    ptr   = static_cast<char *>(ptr) + bytes;
  }
}


void Stream::write(const void *ptr, size_t size)
{
  while (size > 0) {
    size_t bytes = tryWrite(ptr, size);

    size -= bytes;
    ptr   = static_cast<const char *>(ptr) + bytes;
  }
}


std::string Stream::readLine()
{
  // TODO: do not do a system call per character

  std::string str;
  char	      ch;

  for (;;) {
    tryRead(&ch, 1);

    if (ch == '\n')
      return str;

    str += ch;
  }
}


void Stream::sync()
{
}

} // namespace LOFAR
