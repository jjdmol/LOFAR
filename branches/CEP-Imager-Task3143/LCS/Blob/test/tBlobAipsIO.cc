//# tBlobAipsIO.cc: Program to check class BlobAipsIO
//#
//# Copyright (C) 2006
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
#include <Common/LofarLogger.h>

#if defined(HAVE_AIPSPP)

#include <Blob/BlobAipsIO.h>
#include <Blob/BlobOBufChar.h>
#include <Blob/BlobIBufChar.h>
#include <casa/IO/AipsIO.h>

using namespace LOFAR;
using namespace casa;

void doIt()
{
  BlobOBufChar bob;
  {
    BlobOStream bos(bob);
    bos.putStart("test", 1);
    bos << "abc" << int(1) << double(2.2);
    BlobAipsIO baio(bos);
    AipsIO aio(&baio);
    aio.putstart("aio", 1);
    aio << "xyz";
    bos << int(34);
    aio.putend();
    bos << "de";
    bos.putEnd();
  }
  {
    BlobIBufChar bib(bob.getBuffer(), bob.size());
    BlobIStream bis(bib);
    bis.getStart("test");
    int iv;
    string sv;
    double dv;
    bis >> sv >> iv >> dv;
    ASSERT (iv == 1);
    ASSERT (sv == "abc");
    ASSERT (dv == 2.2);
    BlobAipsIO baio(bis);
    AipsIO aio(&baio);
    aio.getstart("aio");
    String asv;
    aio >> asv;
    ASSERT (asv == "xyz");
    bis >> iv;
    ASSERT (iv == 34);
    aio.getend();
    bis >> sv;
    ASSERT (sv == "de");
    bis.getEnd();
  }
}

int main()
{
  INIT_LOGGER("tBlobAipsIO");
  try {
    doIt();
  } catch (std::exception& x) {
    LOG_FATAL_STR("Unexpected expection: " << x.what());
    return 1;
  }
  return 0;
}

#else

int main()
{
  INIT_LOGGER("tBlobAipsIO");
  LOG_INFO("AIPS++ not available. Test skipped");
  return 3;
}

#endif
