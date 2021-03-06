//# FileStream.h: 
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

#ifndef LOFAR_LCS_STREAM_FILE_STREAM_H
#define LOFAR_LCS_STREAM_FILE_STREAM_H

#include <Stream/FileDescriptorBasedStream.h>
#include <string>


namespace LOFAR {

class FileStream : public FileDescriptorBasedStream
{
  public:
	    FileStream(const std::string &name); // read-only; existing file
	    FileStream(const std::string &name, int mode); // rd/wr; create file
	    FileStream(const std::string &name, int flags, int mode); // rd/wr; create file, use given flags
						   
    virtual ~FileStream();

    virtual void skip( size_t bytes ); // seek ahead

    virtual size_t size(); // return file size
};

} // namespace LOFAR

#endif
