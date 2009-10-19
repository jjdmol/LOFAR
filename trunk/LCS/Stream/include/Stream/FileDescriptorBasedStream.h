//# FileDescriptorBasedStream.h: 
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

#ifndef LOFAR_LCS_STREAM_FILE_DESRIPTOR_BASED_STREAM_H
#define LOFAR_LCS_STREAM_FILE_DESRIPTOR_BASED_STREAM_H

#include <Stream/Stream.h>


namespace LOFAR {

class FileDescriptorBasedStream : public Stream
{
  public:
    virtual	 ~FileDescriptorBasedStream();

    virtual void read(void *ptr, size_t size);
    virtual void write(const void *ptr, size_t size);

  protected:
		 FileDescriptorBasedStream(): fd(-1) {} // do not create directly

    int		 fd;
};

} // namespace LOFAR

#endif
