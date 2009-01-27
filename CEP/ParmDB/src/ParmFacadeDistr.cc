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
#include <Common/lofar_iostream.h>

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
      // Get info from VDS. It is automatically closed thereafter.
      int nparts;
      string cdescName;
      {
        VdsDesc vds(tableName);
        nparts    = vds.getParts().size();
        cdescName = vds.getDesc().getClusterDescName();
      }
      // Start all remote processes.
      string command("startparmdbdistr " + itsPort + ' ' +
                     cdescName + ' ' + tableName);
      ASSERT (system(command.c_str()) == 0);
      // Accept a connection from the clients and check if they are
      // initialized correctly.
      itsConn.addConnections (nparts);
      BlobString buf;
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        ASSERT (bbi.getOperation() == 1);    // ensure successful init
        string fname;
        bbi.blobStream() >> fname;
        bbi.finish();
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
      vector<double> range, result;
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        ASSERT (bbi.getOperation() == 1);    // ensure success
        if (i == 0) {
          bbi.blobStream() >> result;
        } else {
          bbi.blobStream() >> range;
          if (range[0] < result[0]) result[0] = range[0];
          if (range[1] > result[1]) result[1] = range[1];
          if (range[2] < result[2]) result[2] = range[2];
          if (range[3] > result[3]) result[3] = range[3];
        }
        bbi.finish();
      }
      return result;
    }

    // Get all parameter names in the table.
    vector<string> ParmFacadeDistr::getNames (const string& parmNamePattern) const
    {
      BlobString buf;
      MWBlobOut bbo(buf, 2, 0);
      bbo.blobStream() << parmNamePattern;
      bbo.finish();
      itsConn.writeAll (buf);
      vector<string> names, result;
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        ASSERT (bbi.getOperation() == 1);    // ensure success
        if (i == 0) {
          bbi.blobStream() >> result;
        } else {
          bbi.blobStream() >> names;
        }
        bbi.finish();
      }
      return result;
    }

    Record ParmFacadeDistr::getValues (const string& parmNamePattern,
                                       double freqv1, double freqv2, int nfreq,
                                       double timev1, double timev2, int ntime,
                                       bool asStartEnd)
    {
      BlobString buf;
      MWBlobOut bbo(buf, 3, 0);
      bbo.blobStream() << parmNamePattern
                       << freqv1 << freqv2 << nfreq
                       << timev1 << timev2 << ntime << asStartEnd;
      bbo.finish();
      itsConn.writeAll (buf);
      Record values, result;
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        ASSERT (bbi.getOperation() == 1);    // ensure success
        if (i == 0) {
          getRecord (bbi.blobStream(), result);
        } else {
          getRecord (bbi.blobStream(), values);
        }
        bbi.finish();
      }
      return result;
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
      bbo.blobStream() << parmNamePattern
                       << freqv1 << freqv2 << timev1 << timev2 << asStartEnd;
      bbo.finish();
      itsConn.writeAll (buf);
      Record values, result;
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        ASSERT (bbi.getOperation() == 1);    // ensure success
        if (i == 0) {
          getRecord (bbi.blobStream(), result);
        } else {
          getRecord (bbi.blobStream(), values);
        }
        bbi.finish();
      }
      return result;
    }

    Record ParmFacadeDistr::getValuesGrid (const string& parmNamePattern,
                                           double sfreq, double efreq,
                                           double stime, double etime)
    {
      BlobString buf;
      MWBlobOut bbo(buf, 5, 0);
      bbo.blobStream() << parmNamePattern
                       << sfreq << efreq << stime << etime;
      bbo.finish();
      itsConn.writeAll (buf);
      Record values, result;
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        ASSERT (bbi.getOperation() == 1);    // ensure success
        if (i == 0) {
          getRecord (bbi.blobStream(), result);
        } else {
          getRecord (bbi.blobStream(), values);
        }
        bbi.finish();
      }
      return result;
    }

    void ParmFacadeDistr::getRecord (BlobIStream& bis, Record& rec)
    {
      BlobAipsIO baio(bis);
      casa::AipsIO aio(&baio);
      aio >> rec;
    }

  } // namespace ParmDB
} // namespace LOFAR
