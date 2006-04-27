//# tBlobAipsIO.cc: Program to check class BlobAipsIO
//#
//# Copyright (C) 2006
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

#include <lofar_config.h>
#include <BBS/BlobAipsIO.h>
#include <Blob/BlobOBufChar.h>
#include <Blob/BlobIBufChar.h>
#include <Common/LofarLogger.h>
#include <casa/IO/AipsIO.h>

using namespace LOFAR;
using namespace casa;
using namespace std;

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
  try {
    doIt();
  } catch (exception& x) {
    cout << "Unexpected expection: " << x.what() << endl;
    return 1;
  }
  return 0;
}
