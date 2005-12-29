//# tExampleDB2.cc: Test program for classes DH_DB and TH_Postgres
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <TransportPostgres/TH_Postgresql.h>
#include <DH_ExampleExtra3.h>
#include <Transport/Connection.h>
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
    INIT_LOGGER("ExampleDB2.log_prop");

    cout << "Transport ExampleDB2 test program" << endl;

    TH_Postgresql::useDatabase ("dop50.astron.nl","test");

    // Connect 2 DataHolders with a TH_Postgresql.
    DH_ExampleExtra3 DH1("dh1", 1);
    TH_Postgresql pgTH("ExampleDB2");
    DH_ExampleExtra3 DH2("dh2", 1);
    Connection conn("connection1", &DH1, &DH2, &pgTH);
    DH1.init();
    DH2.init();
    pgTH.init();

    // fill the DataHolders with some initial data
    {
      DH1.getBuffer()[0] = makefcomplex(17,-3.5);
      DH1.setCounter(2);
      DH2.getBuffer()[0] = makefcomplex(-5,18);
      DH2.setCounter(3);
      BlobOStream& bos = DH1.fillVariableBuffer();
      bos << "test1";
      BlobIStream& bis = DH1.readVariableBuffer();
      string str;
      bis >> str;
      ASSERT (str == "test1");
      // do the data transport
      conn.write();
      conn.read();
      ASSERT (DH2.getBuffer()[0] == makefcomplex(17,-3.5)
	      &&  DH2.getCounter() == 2);     
    }
    // write another 2 records.
    {
      DH1.getBuffer()[0] = makefcomplex(117,-3.5);
      DH1.setCounter(3);
      BlobOStream& bos = DH1.fillVariableBuffer();
      bos << float(4.5) << "test1a";
      conn.write();
    }
    {
      DH1.getBuffer()[0] = makefcomplex(22,-6);
      DH1.setCounter(4);
      BlobOStream& bos = DH1.fillVariableBuffer();
      bos << "bla";
      conn.write();
    }
    {
      // read 
      conn.read();
      ASSERT (DH2.getBuffer()[0] == makefcomplex(117,-3.5)
	      &&  DH2.getCounter() == 3);
      bool found;
      int version;
      BlobIStream& bis = DH2.readVariableBuffer (found, version);
      ASSERT (found);
      float val;
      string str;
      bis >> val >> str;
      bis.getEnd();
      ASSERT (val == 4.5  &&  str == "test1a");
    }
    {
      // and read again
      conn.read();
      ASSERT (DH2.getBuffer()[0] == makefcomplex(22,-6)
	      &&  DH2.getCounter() == 4);
      bool found;
      int version;
      BlobIStream& bis = DH2.readVariableBuffer (found, version);
      ASSERT (found);
      string str;
      bis >> str;
      bis.getEnd();
      ASSERT (str == "bla");
    }
    return 0;

  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
}
