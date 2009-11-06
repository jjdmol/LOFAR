//# TH_FCNP.cc: Fast Collective Network Protocol
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
#include <Transport/DataHolder.h>
#include <TH_FCNP_Server.h>

#include <fcnp_ion.h>

#include <algorithm>


namespace LOFAR {
namespace RTCP {


std::vector<TH_FCNP_Server *> TH_FCNP_Server::theirTHs;


TH_FCNP_Server::TH_FCNP_Server(unsigned core)
:
  itsCore(core)
{
  if (theirTHs.size() <= core)
    theirTHs.resize(core + 1);

  theirTHs[core] = this;
}


TH_FCNP_Server::~TH_FCNP_Server()
{
  *std::find(theirTHs.begin(), theirTHs.end(), this) = 0;
}


#if 0
void TH_FCNP_Server::createAllTH_FCNP_Servers(unsigned nrCoresPerPset)
{
  theirTHs.resize(nrCoresPerPset);

  for (unsigned core = 0; core < nrCoresPerPset; core ++)
    theirTHs[core] = new TH_FCNP_Server(core);
}


void TH_FCNP_Server::deleteAllTH_FCNP_Servers()
{
  for (unsigned core = 0; core < theirTHs.size(); core ++)
    delete theirTHs[core];

  theirTHs.clear();
}
#endif


bool TH_FCNP_Server::init()
{
  return true;
}


bool TH_FCNP_Server::sendBlocking(void *buf, int size, int unaligned, DataHolder *)
{
  //std::clog << std::dec << "TH_FCNP_Server::sendBlocking(" << buf << ", " << size << ", ...) to " << itsCore << std::endl;
  if (unaligned) {
    size_t alignedSize = (size + 15) & ~ (size_t) 15;
    char   tmp[alignedSize] __attribute__ ((aligned(16)));

    memcpy(tmp, buf, size);
    FCNP_ION::IONtoCN_ZeroCopy(itsCore, tmp, alignedSize);
  } else {
    FCNP_ION::IONtoCN_ZeroCopy(itsCore, const_cast<const void *>(buf), size);
  }

  return true;
}


bool TH_FCNP_Server::recvBlocking(void *buf, int size, int unaligned, int, DataHolder *)
{
  //std::clog << std::dec << "TH_FCNP_Server::recvBlocking(" << buf << ", " << size << ", ...)from " << itsCore << std::endl;

  if (unaligned) {
    size_t alignedSize = (size + 15) & ~ (size_t) 15;
    char   tmp[alignedSize] __attribute__ ((aligned(16)));

    FCNP_ION::CNtoION_ZeroCopy(itsCore, tmp, alignedSize);
    memcpy(buf, tmp, size);
  } else {
    FCNP_ION::CNtoION_ZeroCopy(itsCore, buf, size);
  }

  return true;
}


// functions below are not supported

int32 TH_FCNP_Server::recvNonBlocking(void *, int32, int, int32, DataHolder *)
{
  return false;
}


void TH_FCNP_Server::waitForReceived(void *, int, int)
{
}


bool TH_FCNP_Server::sendNonBlocking(void *, int, int, DataHolder *)
{
  return false;
}


void TH_FCNP_Server::waitForSent(void *, int, int)
{
}


string TH_FCNP_Server::getType() const
{
  return "TH_FCNP_Server";
}


bool TH_FCNP_Server::isClonable() const
{
  return true;
}


TransportHolder *TH_FCNP_Server::clone() const
{
  return new TH_FCNP_Server(itsCore);
}


void TH_FCNP_Server::reset()
{
}

} // namespace RTCP
} // namespace LOFAR

#endif
