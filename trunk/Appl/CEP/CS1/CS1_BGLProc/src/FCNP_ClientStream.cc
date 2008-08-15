//# FCNP_ClientStream.cc: Fast Collective Network Protocol Stream
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#if defined HAVE_FCNP && defined HAVE_BGP

#include <Common/Timer.h>
#include <CS1_Interface/Align.h>
#include <CS1_Interface/AlignedStdAllocator.h>
#include <FCNP_ClientStream.h>

#include <fcnp_cn.h>

#include <cstring>
#include <vector>


namespace LOFAR {
namespace CS1 {


FCNP_ClientStream::~FCNP_ClientStream()
{
}


void FCNP_ClientStream::read(void *ptr, size_t size)
{
  //std::clog << "FCNP_ClientStream::read(" << std::hex << ptr << ", " << std::dec << size << ", ...)" << std::endl;

  if (!aligned(ptr, 16) || !aligned(size, 16)) {
    size_t alignedSize = align(size, 16);
    std::vector<char, AlignedStdAllocator<char, 16> > alignedBuffer(alignedSize);

    FCNP_CN::IONtoCN_ZeroCopy(&alignedBuffer[0], alignedSize);
    memcpy(ptr, &alignedBuffer[0], size);
  } else {
    FCNP_CN::IONtoCN_ZeroCopy(ptr, size);
  }
}


void FCNP_ClientStream::write(const void *ptr, size_t size)
{
  //std::clog << "FCNP_ClientStream::write(" << std::hex << ptr << ", " << std::dec << size << ", ...)" << std::endl;

  if (!aligned(ptr, 16) || !aligned(size, 16)) {
    size_t alignedSize = align(size, 16);
    std::vector<char, AlignedStdAllocator<char, 16> > alignedBuffer(alignedSize);

    memcpy(&alignedBuffer[0], ptr, size);
    FCNP_CN::CNtoION_ZeroCopy(&alignedBuffer[0], alignedSize);
  } else {
    FCNP_CN::CNtoION_ZeroCopy(ptr, size);
  }
}

} // namespace CS1
} // namespace LOFAR

#endif
