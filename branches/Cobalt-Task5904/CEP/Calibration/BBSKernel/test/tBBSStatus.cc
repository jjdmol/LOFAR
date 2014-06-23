//# tBBSStatus.cc: test program for the BBSStatus class.
//#
//# Copyright (C) 2002-2004
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <BBSKernel/BBSStatus.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufChar.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobOBufChar.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace LOFAR::BBS;

void checkBlobIO(const BBSStatus& obs)
{
  BlobOBufChar bob;
  BlobOStream bos(bob);
  obs.serialize(bos);

  BlobIBufChar bib(bob.getBuffer(), bob.size());
  BlobIStream bis(bib);
  BBSStatus* ibs = dynamic_cast<BBSStatus*>(BlobStreamable::deserialize(bis));
  ASSERT(ibs);
  ASSERTSTR(ibs->get() == obs.get() &&
	    ibs->text() == obs.text(),
	    "ibs = " << ibs << "; obs = " << obs);
  delete ibs;
}


int main(int /*argc*/, const char* const argv[])
{
  INIT_LOGGER(argv[0]);

  // These tests should all succeed.
  try {
    {
      BBSStatus bs;
      ASSERT(bs.get() == BBSStatus::OK);
      ASSERT(bs);
      checkBlobIO(bs);
      cout << bs << endl;
    }
    {
      BBSStatus bs(BBSStatus::OK, "OK, keep up the good work");
      ASSERT(bs.get() == BBSStatus::OK);
      ASSERT(bs);
      checkBlobIO(bs);
      cout << bs << endl;
    }
    {
      BBSStatus bs(BBSStatus::ERROR, "This is NOT good!");
      ASSERT(bs.get() == BBSStatus::ERROR);
      ASSERT(!bs);
      checkBlobIO(bs);
      cout << bs << endl;
    }
    {
      // Force the use of an undefined enumerated value.
      BBSStatus bs(static_cast<BBSStatus::Status>(18649),
                         "This should never happen!");
      ASSERT(bs.get() == BBSStatus::UNKNOWN);
      ASSERT(!bs);
      checkBlobIO(bs);
      cout << bs << endl;
    }
  }
  catch (Exception& e) {
    cerr << e << endl;
    return 1;
  }

  return 0;
}
