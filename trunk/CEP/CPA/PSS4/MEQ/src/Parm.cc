//# Parm.cc: Stored parameter with polynomial coefficients
//#
//# Copyright (C) 2002
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

#include "Parm.h"
#include "Request.h"
#include "VellSet.h"
#include "Cells.h"
#include "MeqVocabulary.h"
#include <Common/Debug.h>
#include <Common/Lorrays.h>
#include <aips/Mathematics/Math.h>

namespace Meq {

Parm::Parm()
: Function      (),
  itsParmId     (0),
  itsIsSolvable (false),
  itsTable      (0)
{}

Parm::Parm (const string& name, ParmTable* table,
	    const Vells& defaultValue)
: Function      (),
  itsParmId     (0),
  itsIsSolvable (false),
  itsName       (name),
  itsTable      (table),
  itsDefault    (defaultValue)
{}

Parm::~Parm()
{}

void Parm::init (DataRecord::Ref::Xfer& initrec, Forest* frst)
{
  // do parent init (this will call our setStateImpl())
  Node::init (initrec, frst);
  // use default parm name ( = node name) if not set 
  if( itsName.empty() )
    wstate()[FParmName] = itsName = name(); 
}

int Parm::initDomain (const Domain& domain)
{
  itsCurrentDomain = domain;
  // Find the polc(s) for the given domain.
  vector<Polc> polcs;
  if (itsTable) {
    polcs = itsTable->getPolcs (itsName, domain);
  }
  // If none found, try to get a default value.
  if (polcs.size() == 0) {
    Polc polc;
    if (itsTable) {
      polc = itsTable->getInitCoeff (itsName);
    }
    if (polc.ncoeff() == 0) {
      polc.setCoeff (itsDefault);
      polc.setSimCoeff (itsDefault);
      polc.setPertSimCoeff (Vells(0.));
      polc.setNormalize (true);
    }
    AssertMsg (!polc.getCoeff().isNull(), "No value found for parameter "
	       << itsName);
    polc.setDomain (domain);
    // If needed, normalize the initial values.
    if (polc.isNormalized()) {
      polc.setCoeffOnly (polc.normalize(polc.getCoeff(), domain));
      polc.setSimCoeff  (polc.normalize(polc.getSimCoeff(), domain));
    }
    polcs.push_back (polc);
  } else if (isSolvable()) {
    AssertMsg (polcs.size() == 1, "Solvable parameter " << itsName <<
	       " has multiple matching domains for freq "
	       << domain.startFreq() << ':' << domain.endFreq()
	       << " and time "
	       << domain.startTime() << ':' << domain.endTime());
    const Domain& polDom = polcs[0].domain();
    AssertMsg (near(domain.startFreq(), polDom.startFreq())  &&
	       near(domain.endFreq(), polDom.endFreq())  &&
	       near(domain.startTime(), polDom.startTime())  &&
	       near(domain.endTime(), polDom.endTime()),
	       "Solvable parameter " << itsName <<
	       " has a partially instead of fully matching entry for freq "
	       << domain.startFreq() << ':' << domain.endFreq()
	       << " and time "
	       << domain.startTime() << ':' << domain.endTime());
  } else {
    // Check if the polc domains cover the entire domain and if they
    // do not overlap.
  }
  setPolcs (polcs);
  // Now determine the spids if the parm is solvable.
  int nr = 0;
  if (isSolvable()) {
    // For the time being we allow only one polc if the parameter
    // has to be solved.
    AssertStr (itsPolcs.size() == 1,
	       "Multiple polcs used in the solve domain for parameter "
	       << itsName);
    int spidIndex = 256*nodeIndex();
    for (unsigned int i=0; i<itsPolcs.size(); i++) {
      int nrs = itsPolcs[i].makeSolvable (spidIndex);
      nr += nrs;
      spidIndex += nrs;
    }
  } else {
    for (unsigned int i=0; i<itsPolcs.size(); i++) {
      itsPolcs[i].clearSolvable();
    }
  }
  // Store the polcs found into the state.
  DataRecord & polcrec = wstate()[FPolcs].replace() <<= new DataRecord();
  for (uint i=0; i<itsPolcs.size(); i++) 
  {
    DataRecord& rec = polcrec[i] <<= new DataRecord();
    rec[FDomain] <<= new Domain(itsPolcs[i].domain());
    rec[FVellSets] <<= &(itsPolcs[i].getCoeff().getDataArray());
  }
  return nr;
}

int Parm::getResult (Result::Ref &resref,
                     const std::vector<Result::Ref> &,
                     const Request &request,bool newreq)
{
  const Domain & domain = request.cells().domain();
  if( domain != itsCurrentDomain )
    initDomain(domain);
  // Create result object and attach to the ref that was passed in
  Result &result = resref <<= new Result(1,request); // result has one vellset
  // *** NB: Should pass in the proper # of spids here, because
  // VellSet does not allow for changing it later!
  VellSet & vs = result.setNewVellSet(0);
  // return depencies: depends on parm value, if solvable
  // NB: should set UPDATED here if we've received a new parm value
  int retcode = itsIsSolvable ? RES_DEP_VALUE : 0;
  // A single polc can be evaluated immediately.
  if( itsPolcs.size() == 1 ) 
  {
    itsPolcs[0].evaluate(vs,request);
    // no further dependencies (specifically, not on domain)
    return retcode;
  }
  // value now depends on domain and has probably been updated
  // NB: we need to compare domains above, and adjust UPDATED accordingly
  retcode |= RES_DEP_DOMAIN | (newreq?RES_UPDATED:0);
  // Get the domain, etc.
  const Cells &cells = request.cells();
  int ndFreq = cells.nfreq();
  int ndTime = cells.ntime();
  double* datar = 0;
  double stepFreq = cells.stepFreq();
  double halfStepFreq = stepFreq * .5;
  double firstMidFreq = domain.startFreq() + halfStepFreq;
  double lastMidFreq  = firstMidFreq + (ndFreq-1) * stepFreq;
  double firstMidTime = cells.time(0);
  double lastMidTime  = cells.time(ndTime-1);
  vs.setReal (ndFreq, ndTime);
  // Iterate over all polynomials.
  // Evaluate one if its domain overlaps the request domain.
  for (unsigned int i=0; i<itsPolcs.size(); i++) {
    Polc& polc = itsPolcs[i];
    const Domain& polDom = polc.domain();
    if (firstMidFreq < polDom.endFreq() && lastMidFreq > polDom.startFreq()
    &&  firstMidTime < polDom.endTime() && lastMidTime > polDom.startTime()) {
      // Determine which part of the request domain is covered by the
      // polynomial.
      int stFreq = 0;
      if (firstMidFreq < polDom.startFreq()) {
        stFreq = 1 + int((polDom.startFreq() - firstMidFreq) / stepFreq);
      }
      int nrFreq = ndFreq - stFreq;
      if (lastMidFreq > polDom.endFreq()) {
        int remFreq = 1 + int((lastMidFreq - polDom.endFreq()) / stepFreq);
        nrFreq -= remFreq;
      }
      int stTime = 0;
      while (cells.time(stTime) < polDom.startTime()) {
        stTime++;
      }
      int lastTime = ndTime-1;
      while (cells.time(lastTime) > polDom.endTime()) {
        lastTime--;
      }
      int nrTime = lastTime - stTime + 1;
      // If the overlap is full, only this polynomial needs to be evaluated.
      if (stFreq == 0  &&  nrFreq == ndFreq
      &&  stTime == 0  &&  nrTime == ndTime) {
        polc.evaluate (vs, request);
        return retcode;
      }
      // Form the domain and request for the overlapping part
      // and evaluate the polynomial.
      double startFreq = domain.startFreq() + stFreq*stepFreq;
      double startTime = cells.time(stTime) - cells.stepTime(stTime) / 2;
      double endTime   = cells.time(lastTime) + cells.stepTime(lastTime) / 2;
      Domain partDom(startFreq, startFreq + nrFreq*stepFreq,
                     startTime, endTime);
      LoVec_double partStartTime(nrTime);
      LoVec_double partEndTime(nrTime);
      for (int j=0; j<nrTime; j++) {
        partStartTime(j) = cells.time(stTime+j) - cells.stepTime(stTime+j);
        partEndTime(j)   = cells.time(stTime+j) + cells.stepTime(stTime+j);
      }
      Cells partCells (partDom, nrFreq, partStartTime, partEndTime);
      Request partReq(partCells);
      VellSet partRes;
      polc.evaluate (partRes, partReq);
      // Create the result matrix if it is the first Time.
      // Now it is initialized with zeroes (to see possible errors).
      // In the future the outcommnented statement can be used
      // which saves the initialization Time. It requires that the
      // request domain is entirely covered by the polcs.
      if (datar == 0) {
        LoMat_double& mat = vs.setReal (ndFreq, ndTime);
        datar = mat.data();
      }
      // Move the values to the correct place in the output result.
      // Note that in principle a polynomial could be a single coefficient
      // in which case it returns a single value.
      const double* from = partRes.getValue().realStorage();
      double* to = datar + stFreq + stTime*ndFreq;
      if (partRes.getValue().nelements() == 1) {
        for (int iTime=0; iTime<nrTime; iTime++) {
          for (int iFreq=0; iFreq<nrFreq; iFreq++) {
            to[iFreq] = *from;
          }
          to += ndFreq;
        }
      } else {
        for (int iTime=0; iTime<nrTime; iTime++) {
          for (int iFreq=0; iFreq<nrFreq; iFreq++) {
            to[iFreq] = *from++;
          }
          to += ndFreq;
        }
      }
    }
  }
  return retcode;
}

void Parm::save()
{
  const vector<Polc>& polcs = getPolcs();
  for (uint i=0; i<polcs.size(); i++) {
    if (itsTable) {
      itsTable->putCoeff (itsName, polcs[i]);
    }
  }
}

void Parm::setStateImpl (DataRecord& rec, bool initializing)
{
  // inhibit changing of FPolcs field
  protectStateField(rec,FPolcs);
  Function::setStateImpl(rec,initializing);
  // Get solvable flag; clear domain if it changes (to force initDomain call).
  bool oldSolvable = itsIsSolvable;
  getStateField(itsIsSolvable,rec,FSolvable);
  if (oldSolvable != itsIsSolvable) {
    itsCurrentDomain = Domain();
  }
  // Get parm value
  if (rec[FValue].exists()) {
    // Update the polc coefficients with the new values.
    LoVec_double values = rec[FValue].as<LoVec_double>();
    ////    vector<double>& values = rec[FValue].as<vector<double> >();
    uint inx = 0;
    for (uint i=0; i<itsPolcs.size(); i++) {
      inx += itsPolcs[i].update (&values(inx), values.size()-inx);
    }
    Assert (inx == values.size());
    // Also save the parms (might need to be changed later).
    save();
  }
  // Get default value (to be used if no table exists)
  if( rec[FDefault].exists() )
    itsDefault = Vells(rec[FDefault].as_wp<DataArray>());
  // Get ParmTable name 
  if( rec[FTableName].exists() )
  {
    string tableName = state()[FTableName].as<string>();
    if( tableName.empty() )  // no table
      itsTable = 0;
    else    // else open a table
      itsTable = ParmTable::openTable(tableName);
  }
  // Override ParmName if supplied
  getStateField(itsName,rec,FParmName);
}

string Parm::sdebug (int detail, const string &prefix,const char* nm) const
{
  string out = Node::sdebug(detail,prefix,nm);
  if( detail>=2 || detail == -2) {
    Debug::appendf(out,"  parmtable=%s", itsTable->name().c_str());
  }
  return out;
}

} // namespace Meq
