//# FCNP.cc: Fast Collective Network Protocol
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

#if defined HAVE_FCNP && defined __PPC__

#include <Common/Timer.h>
#include <FCNP_ServerStream.h>

#include <fcnp_ion.h>

#include <algorithm>


namespace LOFAR {
namespace CS1 {


std::vector<FCNP_ServerStream *> FCNP_ServerStream::allStreams;


FCNP_ServerStream::FCNP_ServerStream(unsigned core)
:
  itsCore(core)
{
  if (allStreams.size() <= core)
    allStreams.resize(core + 1);

  allStreams[core] = this;
}


FCNP_ServerStream::~FCNP_ServerStream()
{
  *std::find(allStreams.begin(), allStreams.end(), this) = 0;
}


#if 0
void FCNP_ServerStream::createAllFCNP_ServerStreams(unsigned nrCoresPerPset)
{
  allStreams.resize(nrCoresPerPset);

  for (unsigned core = 0; core < nrCoresPerPset; core ++)
    allStreams[core] = new FCNP_ServerStream(core);
}


void FCNP_ServerStream::deleteAllFCNP_ServerStreams()
{
  for (unsigned core = 0; core < allStreams.size(); core ++)
    delete allStreams[core];

  allStreams.clear();
}
#endif


void FCNP_ServerStream::write(const void *buf, size_t size)
{
  //std::clog << "FCNP_ServerStream::write(" << std::hex << buf << ", " << std::dec << size << ") to " << itsCore << std::endl;

  if (reinterpret_cast<size_t>(buf) % 16 != 0 || size % 16 != 0) {
    size_t alignedSize = (size + 15) & ~ (size_t) 15;
    char   tmp[alignedSize] __attribute__ ((aligned(16)));

    memcpy(tmp, buf, size);
    FCNP_ION::IONtoCN_ZeroCopy(itsCore, tmp, alignedSize);
  } else {
    FCNP_ION::IONtoCN_ZeroCopy(itsCore, const_cast<const void *>(buf), size);
  }
}


void FCNP_ServerStream::read(void *buf, size_t size)
{
  //std::clog << std::dec << "FCNP_ServerStream::read(" << std::hex << buf << ", " << std::dec << size << ") from " << itsCore << std::endl;

  if (reinterpret_cast<size_t>(buf) % 16 != 0 || size % 16 != 0) {
    size_t alignedSize = (size + 15) & ~ (size_t) 15;
    char   tmp[alignedSize] __attribute__ ((aligned(16)));

    FCNP_ION::CNtoION_ZeroCopy(itsCore, tmp, alignedSize);
    memcpy(buf, tmp, size);
  } else {
    FCNP_ION::CNtoION_ZeroCopy(itsCore, buf, size);
  }
}

} // namespace CS1
} // namespace LOFAR

#endif
