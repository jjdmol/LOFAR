//# ParmFacadeDistr.cc: Object access the parameter database
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
#include <ParmDB/ParmFacadeDistr.h>
#include <MWCommon/VdsDesc.h>
#include <MWCommon/MWBlobIO.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobAipsIO.h>
#include <Common/LofarLogger.h>
#include <casa/IO/AipsIO.h>

using namespace LOFAR::CEP;
using namespace casa;

// Create tParmFacade.in_mep with parmdb using:
//   create tablename='tParmFacade.in_mep'
//   add parm1 domain=[1,5,4,10],values=2
//   add parm2 domain=[1,5,4,10],values=[2,0.1],nx=2
//   add parm3 type='expression',expression='parm1*parm2'

namespace LOFAR {
  namespace BBS {

    // Initialize statics.
    int ParmFacadeDistr::theirNextPort = 4195;
    vector<string> ParmFacadeDistr::theirFreePorts;


    ParmFacadeDistr::ParmFacadeDistr (const string& tableName)
      : itsPort (getPort()),
        itsConn (itsPort)
    {
      VdsDesc vds(tableName);
      int nparts = vds.getParts().size();
      string cdescName = vds.getDesc().getClusterDescName();
      // Start all clients.
      string command("startdistproc -mode 0 -nomasterhost -cdn " +
                     cdescName + "parmdbclient");
      // Accept a connection from the clients and check if they are
      // initialized correctly.
      itsConn.addConnections (nparts);
      ASSERT (system(command.c_str()) == 0);
      BlobString buf;
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        bbi.finish();
        ASSERT (bbi.getOperation() == 1);    // ensure successful init
      }
    }

    ParmFacadeDistr::~ParmFacadeDistr()
    {
      quit();
    }

    void ParmFacadeDistr::quit()
    {
      // Send all parts an end message.
      BlobString buf;
      MWBlobOut bbo(buf, 0, 0);
      bbo.finish();
      itsConn.writeAll (buf);
      freePort();
    }

    string ParmFacadeDistr::getPort()
    {
      if (! theirFreePorts.empty()) {
        string port = theirFreePorts.back();
        theirFreePorts.pop_back();
        return port;
      }
      ostringstream ostr;
      ostr << theirNextPort;
      theirNextPort++;
      return ostr.str();
    }

    void ParmFacadeDistr::freePort()
    {
      theirFreePorts.push_back (itsPort);
    }

    vector<double> ParmFacadeDistr::getRange (const string& parmNamePattern) const
    {
      BlobString buf;
      MWBlobOut bbo(buf, 1, 0);
      bbo.blobStream() << parmNamePattern;
      bbo.finish();
      itsConn.writeAll (buf);
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        bbi.finish();
        ASSERT (bbi.getOperation() == 1);    // ensure successful init
      }
      vector<double> res(4);
      return res;
    }

    // Get all parameter names in the table.
    vector<string> ParmFacadeDistr::getNames (const string& parmNamePattern) const
    {
      BlobString buf;
      MWBlobOut bbo(buf, 2, 0);
      bbo.blobStream() << parmNamePattern;
      bbo.finish();
      itsConn.writeAll (buf);
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        bbi.finish();
        ASSERT (bbi.getOperation() == 1);    // ensure successful init
      }
      vector<string> res;
      return res;
    }

    Record ParmFacadeDistr::getValues (const string& parmNamePattern,
                                       double freqv1, double freqv2, int nfreq,
                                       double timev1, double timev2, int ntime,
                                       bool asStartEnd)
    {
      BlobString buf;
      MWBlobOut bbo(buf, 3, 0);
      bbo.blobStream() << parmNamePattern;
      bbo.finish();
      itsConn.writeAll (buf);
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        bbi.finish();
        ASSERT (bbi.getOperation() == 1);    // ensure successful init
      }
      Record res;
      return res;
    }

    Record ParmFacadeDistr::getValues (const string& parmNamePattern,
                                       const vector<double>& freqv1,
                                       const vector<double>& freqv2,
                                       const vector<double>& timev1,
                                       const vector<double>& timev2,
                                       bool asStartEnd)
    {
      BlobString buf;
      MWBlobOut bbo(buf, 4, 0);
      bbo.blobStream() << parmNamePattern;
      bbo.finish();
      itsConn.writeAll (buf);
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        bbi.finish();
        ASSERT (bbi.getOperation() == 1);    // ensure successful init
      }
      Record res;
      return res;
    }

    Record ParmFacadeDistr::getValuesGrid (const string& parmNamePattern,
                                           double sfreq, double efreq,
                                           double stime, double etime)
    {
      BlobString buf;
      MWBlobOut bbo(buf, 5, 0);
      bbo.blobStream() << parmNamePattern;
      bbo.finish();
      itsConn.writeAll (buf);
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        bbi.finish();
        ASSERT (bbi.getOperation() == 1);    // ensure successful init
      }
      Record res;
      return res;
    }

  } // namespace ParmDB
} // namespace LOFAR
