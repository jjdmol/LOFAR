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

#if defined HAVE_FCNP && defined HAVE_BGP

#include <Common/Timer.h>
#include <Transport/DataHolder.h>
#include <TH_FCNP_Client.h>

#include <fcnp_cn.h>

#include <algorithm>


namespace LOFAR {
namespace CS1 {


TH_FCNP_Client::~TH_FCNP_Client()
{
}


bool TH_FCNP_Client::init()
{
  return true;
}


bool TH_FCNP_Client::sendBlocking(void *buf, int size, int unaligned, DataHolder *)
{
  //std::clog << std::dec << "TH_FCNP_Client::sendBlocking(" << buf << ", " << size << ", ...)" << std::endl;

  if (unaligned) {
    size_t alignedSize = (size + 15) & ~ (size_t) 15;
    char   tmp[alignedSize] __attribute__ ((aligned(16)));

    memcpy(tmp, buf, size);
    FCNP_CN::CNtoION_ZeroCopy(tmp, alignedSize);
  } else {
    FCNP_CN::CNtoION_ZeroCopy(const_cast<const void *>(buf), size);
  }

  return true;
}


bool TH_FCNP_Client::recvBlocking(void *buf, int size, int unaligned, int, DataHolder *)
{
  //std::clog << std::dec << "TH_FCNP_Client::recvBlocking(" << buf << ", " << size << ", ...)" << std::endl;

  if (unaligned) {
    size_t alignedSize = (size + 15) & ~ (size_t) 15;
    char   tmp[alignedSize] __attribute__ ((aligned(16)));

    FCNP_CN::IONtoCN_ZeroCopy(tmp, alignedSize);
    memcpy(buf, tmp, size);
  } else {
    FCNP_CN::IONtoCN_ZeroCopy(buf, size);
  }

  return true;
}


// functions below are not supported

int32 TH_FCNP_Client::recvNonBlocking(void *, int32, int, int32, DataHolder *)
{
  return false;
}


void TH_FCNP_Client::waitForReceived(void *, int, int)
{
}


bool TH_FCNP_Client::sendNonBlocking(void *, int, int, DataHolder *)
{
  return false;
}


void TH_FCNP_Client::waitForSent(void *, int, int)
{
}


string TH_FCNP_Client::getType() const
{
  return "TH_FCNP_Client";
}


bool TH_FCNP_Client::isClonable() const
{
  return true;
}


TransportHolder *TH_FCNP_Client::clone() const
{
  return new TH_FCNP_Client;
}


void TH_FCNP_Client::reset()
{
}

} // namespace CS1
} // namespace LOFAR

#endif
