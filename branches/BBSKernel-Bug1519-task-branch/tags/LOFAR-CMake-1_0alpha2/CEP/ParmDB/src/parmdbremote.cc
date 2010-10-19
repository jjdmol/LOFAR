//# parmdbremote.cc: Remote handling a distributed ParmDB part
//#
//# Copyright (C) 2009
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
#include <ParmDB/ParmFacadeLocal.h>
#include <ParmDB/ParmFacadeDistr.h>
#include <MWCommon/SocketConnection.h>
#include <MWCommon/MWBlobIO.h>
#include <MWCommon/VdsPartDesc.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobAipsIO.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <casa/IO/AipsIO.h>
#include <casa/Containers/Record.h>
#include <iostream>
#include <unistd.h>     //# for basename
#include <libgen.h>

using namespace LOFAR::BBS;
using namespace LOFAR::CEP;
using namespace LOFAR;
using namespace std;

void putRecord (BlobOStream& bos, const casa::Record& rec)
{
  BlobAipsIO baio(bos);
  casa::AipsIO aio(&baio);
  aio << rec;
}

void getRange (ParmFacadeLocal& pdb, BlobIStream& bis, BlobOStream& bos)
{
  string pattern;
  bis >> pattern;
  bos << pdb.getRange (pattern);
}

void getValues (ParmFacadeLocal& pdb, BlobIStream& bis, BlobOStream& bos,
                const vector<double>& range)
{
  string pattern;
  double sfreq, efreq, stime, etime, freqStep, timeStep;
  bis >> pattern >> sfreq >> efreq >> freqStep
      >> stime >> etime >> timeStep;
  if (sfreq < range[0]) sfreq = range[0];
  if (efreq > range[1]) efreq = range[1];
  if (stime < range[2]) stime = range[2];
  if (etime > range[3]) etime = range[3];
  casa::Record rec;
  string msg;
  if (sfreq <= efreq  &&  stime <= etime) {
    try {
      rec = pdb.getValues (pattern, sfreq, efreq, freqStep,
                           stime, etime, timeStep, true);
    } catch (std::exception& x) {
      msg = x.what();
    }
  }
  putRecord (bos, rec);
  bos << msg;
}

void getValuesVec (ParmFacadeLocal& pdb, BlobIStream& bis, BlobOStream& bos)
{
  string pattern;
  vector<double> freqv1, freqv2, timev1, timev2;
  bool asStartEnd;
  bis >> pattern >> freqv1 >> freqv2 >> timev1 >> timev2 >> asStartEnd;
  casa::Record rec;
  string msg;
  try {
    rec = pdb.getValues (pattern, freqv1, freqv2,
                         timev1, timev2, asStartEnd);
  } catch (std::exception& x) {
    msg = x.what();
  }
  putRecord (bos, rec);
  bos << msg;
}

void getValuesGrid (ParmFacadeLocal& pdb, BlobIStream& bis, BlobOStream& bos)
{
  string pattern;
  double sfreq, efreq, stime, etime;
  bis >> pattern >> sfreq >> efreq >> stime >> etime;
  casa::Record rec;
  string msg;
  if (sfreq <= efreq  &&  stime <= etime) {
    try {
      rec = pdb.getValuesGrid (pattern, sfreq, efreq, stime, etime, true);
    } catch (std::exception& x) {
      msg = x.what();
    }
  }
  putRecord (bos, rec);
  bos << msg;
}

void getCoeff (ParmFacadeLocal& pdb, BlobIStream& bis, BlobOStream& bos)
{
  string pattern;
  double sfreq, efreq, stime, etime;
  bis >> pattern >> sfreq >> efreq >> stime >> etime;
  casa::Record rec;
  string msg;
  if (sfreq <= efreq  &&  stime <= etime) {
    try {
      rec = pdb.getCoeff (pattern, sfreq, efreq, stime, etime, true);
    } catch (std::exception& x) {
      msg = x.what();
    }
  }
  putRecord (bos, rec);
  bos << msg;
}

void doIt (SocketConnection& conn, ParmFacadeLocal& pdb)
{
  vector<double> range = pdb.getRange("*");
  // Allocate buffers here, so they are kept alive and not resized all the time.
  BlobString bufin;
  BlobString bufout;
  while (true) {
    // Read and handle the message.
    // Stop if such a message is given.
    conn.read (bufin);
    MWBlobIn bbi(bufin);
    MWBlobOut bbo(bufout, 1, 0);
    switch (bbi.getOperation()) {
    case ParmFacadeDistr::Quit:
      bbi.finish();
      return;
    case ParmFacadeDistr::GetRange:
      getRange (pdb, bbi.blobStream(), bbo.blobStream());
      break;
    case ParmFacadeDistr::GetValues:
      getValues (pdb, bbi.blobStream(), bbo.blobStream(), range);
      break;
    case ParmFacadeDistr::GetValuesVec:
      getValuesVec (pdb, bbi.blobStream(), bbo.blobStream());
      break;
    case ParmFacadeDistr::GetValuesGrid:
      getValuesGrid (pdb, bbi.blobStream(), bbo.blobStream());
      break;
    case ParmFacadeDistr::GetCoeff:
      getCoeff (pdb, bbi.blobStream(), bbo.blobStream());
      break;
    default:
      ASSERTSTR(false, "parmdbremote: unknown command-id "
                << bbi.getOperation());
    }
    // Finish the blobstreams and write the result message.
    bbi.finish();
    bbo.finish();
    conn.write (bufout);
  }
}

int main (int argc, char* argv[])
{
  const char* progName = basename(argv[0]);
  INIT_LOGGER(progName);
  SocketConnection::ShPtr conn;
  try {
    ASSERTSTR (argc >= 4, "Use as: parmdbremote <host> <port> <mspart>");
    string host (argv[1]);
    string port (argv[2]);
    string fname(argv[3]);
    // Setup the connection.
    conn = SocketConnection::ShPtr(new SocketConnection(host, port));
    // Open the ParmDB.
    ParmFacadeLocal parmdb(fname);
    {
      // Tell the master the part name and the parm names it contains.
      BlobString bufout;
      MWBlobOut bbo(bufout, 1, 0);
      bbo.blobStream() << fname;
      bbo.blobStream() << parmdb.getNames("*");
      bbo.finish();
      conn->write (bufout);
    }
    // Handle requests.
    doIt (*conn, parmdb);
  } catch (std::exception& x) {
    LOG_FATAL (string("Unexpected exception in parmdbremote: ") + x.what());
    // Tell master there is an error.
    BlobString bufout;
    MWBlobOut bbo(bufout, 0, 0);
    bbo.blobStream() << x.what();
    bbo.finish();
    cerr << "Unexpected parmdbremote exception: " << x.what() << endl;
    conn->write (bufout);
    return 1;
  }
  return 0;
}
