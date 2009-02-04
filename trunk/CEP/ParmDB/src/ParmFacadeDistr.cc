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
#include <casa/Containers/Record.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIO.h>
#include<casa/Utilities/GenSort.h>
#include <Common/lofar_iostream.h>

using namespace LOFAR::CEP;
using namespace casa;


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
      string fname;
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        ASSERT (bbi.getOperation() == 1);    // ensure successful init
        bbi.blobStream() >> fname;           // get part name
        itsPartNames.push_back (fname);
        if (i == 0) {
          bbi.blobStream() >> itsParmNames;
        } else {
          vector<string> names;
          bbi.blobStream() >> names;
          checkNames (names, i);
        }
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
      MWBlobOut bbo(buf, Quit, 0);
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
      if (parmNamePattern.empty()  ||  parmNamePattern == "*") {
        return itsParmNames;
      }
      Regex regex(Regex::fromPattern(parmNamePattern));
      vector<string> result;
      for (vector<string>::const_iterator iter=itsParmNames.begin();
           iter!=itsParmNames.end(); ++iter) {
        if (String(*iter).matches (regex)) {
          result.push_back (*iter);
        }
      }
      return result;
    }

    void ParmFacadeDistr::checkNames (const vector<string>& names,
                                      uint inx) const
    {
      bool same = (names.size() == itsParmNames.size());
      uint i=0;
      while (same  &&  i<names.size()) {
        same = (names[i] == itsParmNames[i]);
        ++i;
      }
      if (!same) {
        LOG_WARN_STR ("names sizes of parts " << itsPartNames[0] << " and "
                      << itsPartNames[inx] << " differ");
      }
    }

    Record ParmFacadeDistr::getValues (const string& parmNamePattern,
                                       double freqv1, double freqv2,
                                       double freqStep,
                                       double timev1, double timev2,
                                       double timeStep,
                                       bool asStartEnd)
    {
      BlobString buf;
      MWBlobOut bbo(buf, GetValues, 0);
      if (!asStartEnd) {
        double width = freqv2;
        freqv1 -= width/2;
        freqv2 = freqv1 + width;
        width = timev2;
        timev1 -= width/2;
        timev2 = timev1 + width;
      }
      bbo.blobStream() << parmNamePattern
                       << freqv1 << freqv2 << freqStep
                       << timev1 << timev2 << timeStep;
      bbo.finish();
      itsConn.writeAll (buf);
      vector<Record> recs(itsConn.size());
      string msg;
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        ASSERT (bbi.getOperation() == 1);    // ensure success
        getRecord (bbi.blobStream(), recs[i]);
        bbi.blobStream() >> msg;
        bbi.finish();
      }
      return combineRemote (recs);
    }

    Record ParmFacadeDistr::getValues (const string& parmNamePattern,
                                       const vector<double>& freqv1,
                                       const vector<double>& freqv2,
                                       const vector<double>& timev1,
                                       const vector<double>& timev2,
                                       bool asStartEnd)
    {
      BlobString buf;
      MWBlobOut bbo(buf, GetValuesVec, 0);
      bbo.blobStream() << parmNamePattern
                       << freqv1 << freqv2 << timev1 << timev2 << asStartEnd;
      bbo.finish();
      itsConn.writeAll (buf);
      vector<Record> recs(itsConn.size());
      string msg;
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        ASSERT (bbi.getOperation() == 1);    // ensure success
        getRecord (bbi.blobStream(), recs[i]);
        bbi.blobStream() >> msg;
        bbi.finish();
      }
      return combineRemote (recs);
    }

    Record ParmFacadeDistr::getValuesGrid (const string& parmNamePattern,
                                           double freqv1, double freqv2,
                                           double timev1, double timev2,
                                           bool asStartEnd)
    {
      double sfreq = freqv1;
      double efreq = freqv2;
      double stime = timev1;
      double etime = timev2;
      if (!asStartEnd) {
        sfreq = freqv1 - freqv2/2;
        efreq = sfreq  + freqv2;
        stime = timev1 - timev2/2;
        etime = stime  + timev2;
      }
      BlobString buf;
      MWBlobOut bbo(buf, GetValuesGrid, 0);
      bbo.blobStream() << parmNamePattern
                       << sfreq << efreq << stime << etime;
      bbo.finish();
      itsConn.writeAll (buf);
      vector<Record> recs(itsConn.size());
      string msg;
      for (int i=0; i<itsConn.size(); ++i) {
        itsConn.read (i, buf);
        MWBlobIn bbi(buf);
        ASSERT (bbi.getOperation() == 1);    // ensure success
        getRecord (bbi.blobStream(), recs[i]);
        bbi.blobStream() >> msg;
        bbi.finish();
      }
      return combineRemote (recs);
    }

    Record ParmFacadeDistr::combineRemote (const vector<Record>& recs) const
    {
      // Find all possible parm names.
      set<String> names;
      findParmNames (recs, names);
      // Combine the data for each parameter.
      Record result, gridInfo;
      // Step through all parameters.
      for (set<String>::const_iterator iter=names.begin();
           iter!=names.end(); ++iter) {
        combineInfo (*iter, recs, result, gridInfo);
      }
      result.defineRecord ("_grid", gridInfo);
      return result;
    }

    void ParmFacadeDistr::findParmNames (const vector<Record>& recs,
                                         set<String>& names) const
    {
      for (vector<Record>::const_iterator iter=recs.begin();
           iter!=recs.end(); ++iter) {
        for (uint i=0; i<iter->nfields(); ++i) {
          const String& parmName = iter->name(i);
          if (parmName != "_grid") {
            names.insert (parmName);
          }
        }
      }
    }

    void ParmFacadeDistr::combineInfo (const String& name,
                                       const vector<Record>& recs,
                                       Record& result, Record& gridRec) const
    {
      // Get references to all data arrays.
      vector<const Array<double>*> values;
      vector<const Array<double>*> fcenters;
      vector<const Array<double>*> fwidths;
      const Array<double>* tcenters;
      const Array<double>* twidths;
      vector<double> freqs;
      values.reserve (recs.size());
      fcenters.reserve (recs.size());
      fwidths.reserve (recs.size());
      uint ntime=0;
      for (vector<Record>::const_iterator iter=recs.begin();
           iter!=recs.end(); ++iter) {
        if (iter->isDefined (name)) {
          const Record& grid = iter->subRecord ("_grid");
          if (values.empty()) {
            // First time.
            tcenters = &grid.asArrayDouble(name + ";times");
            twidths  = &grid.asArrayDouble(name + ";timewidths");
            ntime = tcenters->size();
          } else {
            // Check if the time axis is the same.
            // That should always be the case.
            ASSERT (allNear (*tcenters,
                             grid.asArrayDouble(name + ";times"), 1e-10));
          }
          values.push_back   (&iter->asArrayDouble (name));
          fcenters.push_back (&grid.asArrayDouble (name + ";freqs"));
          fwidths.push_back  (&grid.asArrayDouble (name + ";freqwidths"));
          freqs.push_back (fcenters[freqs.size()]->data()[0]);
        }
      }
      // Exit if no matching values found.
      if (values.empty()) {
        return;
      }
      // Now sort (indirectly) the parts in freq order.
      Vector<uInt> indexf;
      GenSortIndirect<double>::sort (indexf, &(freqs[0]), freqs.size());
      // Get the unique frequency domains.
      // This is needed because sometimes BBS solves (partially) globally.
      vector<uint> index;
      index.reserve (indexf.size());
      // The first one always has to be used.
      index.push_back (indexf[0]);
      const Array<double>* lastUsed = fcenters[indexf[0]];
      uint nfreq = lastUsed->size();
      for (uint i=1; i<indexf.size(); ++i) {
        uint inx = indexf[i];
        const Array<double>* arr = fcenters[inx];
        if (arr->size() != lastUsed->size()  ||
            !allNear (*arr, *lastUsed, 1e-10)) {
          index.push_back (inx);
          lastUsed = fcenters[inx];
          nfreq += lastUsed->size();
        }
      }
      // Times are the same for all parts, so define them.
      gridRec.define (name+";times", *tcenters);
      gridRec.define (name+";timeWidhts", *twidths);
      // If only one part left, take that one (which is the first one).
      if (index.size() == 1) {
        result.define (name, *values[0]);
        gridRec.define (name+";freqs", *fcenters[0]);
        gridRec.define (name+";freqwidths", *fwidths[0]);
        return;
      }
      // Combine the values and freq grid.
      Array<double> data(IPosition(2, nfreq, ntime));
      Array<double> fcenter(IPosition(1, nfreq));
      Array<double> fwidth(IPosition(1, nfreq));
      double* dtp = data.data();
      double* fcp = fcenter.data();
      double* fwp = fwidth.data();
      // We do the copying ourselves instead of using Array sectioning.
      // It is faster and about as easy to write as we know that all Arrays
      // are contiguous.
      for (uint i=0; i<index.size(); ++i) {
        uint inx = index[i];
        uint nf = fcenters[inx]->size();
        memcpy (fcp, fcenters[inx]->data(), nf*sizeof(double));
        memcpy (fwp, fwidths[inx]->data(),  nf*sizeof(double));
        double* to = dtp;
        const double* from = values[inx]->data();
        for (uint j=0; j<ntime; ++j) {
          memcpy (to, from, nf*sizeof(double));
          to += nfreq;
          from += nf;
        }
        dtp += nf;
        fcp += nf;
        fwp += nf;
      }
      result.define (name, data);
      gridRec.define (name+";freqs", fcenter);
      gridRec.define (name+";freqwidths", fwidth);
    }

    void ParmFacadeDistr::getRecord (BlobIStream& bis, Record& rec)
    {
      BlobAipsIO baio(bis);
      casa::AipsIO aio(&baio);
      aio >> rec;
    }

  } // namespace ParmDB
} // namespace LOFAR
