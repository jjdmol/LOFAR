//# tExampleDB.cc: Test program for classes DH_DB and TH_Postgres
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
    INIT_LOGGER("ExampleDB.log_prop");

    cout << "Transport ExampleDB test program" << endl;

    TH_Postgresql::useDatabase ("dop50.astron.nl","test");

    // Connect to itself to attach the TH_Postgresql to the data holder.
    DH_ExampleExtra3 DH1("dh1", 1);
    TH_Postgresql pgTH;
    Connection conn("connection1", &DH1, &DH1, &pgTH);
    DH1.init();
    pgTH.init();

    // fill the DataHolders with some initial data
    {
      DH1.getBuffer()[0] = makefcomplex(17,-3.5);
      DH1.setCounter(2);
      BlobOStream& bos = DH1.fillVariableBuffer();
      bos << "test1";
      BlobIStream& bis = DH1.readVariableBuffer();
      string str;
      bis >> str;
      ASSERT (str == "test1");
      DH1.insertDB(conn);
    }
    // write another record.
    {
      DH1.getBuffer()[0] = makefcomplex(117,-3.5);
      DH1.setCounter(3);
      BlobOStream& bos = DH1.fillVariableBuffer();
      bos << float(4.5) << "test1a";
      DH1.insertDB(conn);
    }
    // read first record back and check it.
    {
      string query("SELECT * FROM exampledb WHERE counter=2");
      ASSERT (DH1.queryDB (query, conn) == 1);
      ASSERT (DH1.getBuffer()[0] == makefcomplex(17,-3.5)
	      &&  DH1.getCounter() == 2);
      bool found;
      int version;
      BlobIStream& bis = DH1.readVariableBuffer (found, version);
      ASSERT (found);
      string str;
      bis >> str;
      bis.getEnd();
      ASSERT (str == "test1");
      BlobIStream& bis2 = DH1.readVariableBuffer();
      bis2 >> str;
      ASSERT (str == "test1");
    }
    // update the record, read it back and check it.
    DH1.getBuffer()[0] = makefcomplex(8,-7);
    DH1.updateDB(conn);
    DH1.getBuffer()[0] = makefcomplex(23,89);
    {
      ASSERT (DH1.queryDB ("SELECT * FROM exampledb WHERE counter=2", conn) == 1);
      ASSERT (DH1.getBuffer()[0] == makefcomplex(8,-7)
	      &&  DH1.getCounter() == 2);
      bool found;
      int version;
      BlobIStream& bis = DH1.readVariableBuffer (found, version);
      ASSERT (found);
      string str;
      bis >> str;
      bis.getEnd();
      ASSERT (str == "test1");
    }

    // read second record back and check it.
    {
      ASSERT (DH1.queryDB ("SELECT * FROM exampledb WHERE counter=3", conn) == 1);
      ASSERT (DH1.getBuffer()[0] == makefcomplex(117,-3.5)
	      &&  DH1.getCounter() == 3);
      bool found;
      int version;
      BlobIStream& bis = DH1.readVariableBuffer (found, version);
      ASSERT (found);
      float val;
      string str;
      bis >> val >> str;
      bis.getEnd();
      ASSERT (val == 4.5  &&  str == "test1a");
    }
    // update the record, read it back and check it.
    DH1.getBuffer()[0] = makefcomplex(0,0)-DH1.getBuffer()[0];
    DH1.clearVariableBuffer();
    DH1.updateDB(conn);
    {
      ASSERT (DH1.queryDB ("SELECT * FROM exampledb WHERE counter=3", conn) == 1);
      ASSERT (DH1.getBuffer()[0] == makefcomplex(-117,3.5)
	      &&  DH1.getCounter() == 3);
      bool found;
      int version;
      DH1.readVariableBuffer (found, version);
      ASSERT (!found);
    }
    return 0;

  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
}
