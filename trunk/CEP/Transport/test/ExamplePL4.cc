//# tExamplePL4.cc: Test program for classes DH_PL and TH_PL
//#
//# Copyright (C) 2004
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

#include <Transport/TH_PL.h>
#include <DH_Example2.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/LofarLogger.h>
#include <iostream>

// This test program tests if the DataHolder::xxxDB functions work
// fine when using an extra blob.

using namespace LOFAR;

int main()
{
  try {
    INIT_LOGGER("ExamplePL4.log_prop");

    cout << "Transport ExamplePL4 test program" << endl;

    TH_PL::useDatabase ("test");

    // Connect to itself to attach the TH_PL to the data holder.
    DH_Example2 DH1("dh1", 1, true);
    DH1.setID(1);
    DH1.connectTo(DH1, TH_PL("ExamplePL2"));
    DH1.init();
    
    // fill the DataHolders with some initial data
    {
      DH1.getBuffer()[0] = fcomplex(17,-3.5);
      DH1.setCounter(2);
      BlobOStream& bos = DH1.createExtraBlob();
      bos << "test1";
      BlobIStream& bis = DH1.getExtraBlob();
      string str;
      bis >> str;
      ASSERT (str == "test1");
      DH1.insertDB();
    }
    // write another record.
    {
      DH1.getBuffer()[0] = fcomplex(117,-3.5);
      DH1.setCounter(3);
      BlobOStream& bos = DH1.createExtraBlob();
      bos << float(4.5) << "test1a";
      DH1.insertDB();
    }
    // read first record back and check it.
    {
      ASSERT (DH1.queryDB ("counter=2") == 1);
      ASSERT (DH1.getBuffer()[0] == fcomplex(17,-3.5)
	      &&  DH1.getCounter() == 2);
      bool found;
      int version;
      BlobIStream& bis = DH1.getExtraBlob (found, version);
      ASSERT (found);
      string str;
      bis >> str;
      bis.getEnd();
      ASSERT (str == "test1");
      BlobIStream& bis2 = DH1.getExtraBlob();
      bis2 >> str;
      ASSERT (str == "test1");
    }
    // update the record, read it back and check it.
    DH1.setCounter(4);
    DH1.updateDB();
    {
      ASSERT (DH1.queryDB ("counter=4") == 1);
      ASSERT (DH1.getBuffer()[0] == fcomplex(17,-3.5)
	      &&  DH1.getCounter() == 4);
      bool found;
      int version;
      BlobIStream& bis = DH1.getExtraBlob (found, version);
      ASSERT (found);
      string str;
      bis >> str;
      bis.getEnd();
      ASSERT (str == "test1");
    }

    // read second record back and check it.
    {
      ASSERT (DH1.queryDB ("counter=3") == 1);
      ASSERT (DH1.getBuffer()[0] == fcomplex(117,-3.5)
	      &&  DH1.getCounter() == 3);
      bool found;
      int version;
      BlobIStream& bis = DH1.getExtraBlob (found, version);
      ASSERT (found);
      float val;
      string str;
      bis >> val >> str;
      bis.getEnd();
      ASSERT (val == 4.5  &&  str == "test1a");
    }
    // update the record, read it back and check it.
    DH1.getBuffer()[0] = -DH1.getBuffer()[0];
    DH1.clearExtraBlob();
    DH1.updateDB();
    {
      ASSERT (DH1.queryDB ("counter=3") == 1);
      ASSERT (DH1.getBuffer()[0] == fcomplex(-117,3.5)
	      &&  DH1.getCounter() == 3);
      bool found;
      int version;
      DH1.getExtraBlob (found, version);
      ASSERT (!found);
    }
    return 0;

  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
}
