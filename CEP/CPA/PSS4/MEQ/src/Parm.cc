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

InitDebugContext(Parm,"MeqParm");

//##ModelId=3F86886F021B
Parm::Parm()
: Function      (),
  itsParmId     (0),
  itsIsSolvable (false),
  itsTable      (0)
{}

//##ModelId=3F86886F0242
Parm::Parm (const string& name, ParmTable* table,
	    const Polc::Ref::Xfer & defaultValue)
: Function      (),
  itsParmId     (0),
  itsIsSolvable (false),
  itsName       (name),
  itsTable      (table),
  itsDefault    (defaultValue)
{}

//##ModelId=3F86886F021E
Parm::~Parm()
{}

//##ModelId=400E5352023D
void Parm::init (DataRecord::Ref::Xfer& initrec, Forest* frst)
{
  // do parent init (this will call our setStateImpl())
  Node::init (initrec, frst);
  // use default parm name ( = node name) if not set 
  if( itsName.empty() )
    wstate()[FParmName] = itsName = name(); 
}

//##ModelId=3F86886F0226
int Parm::initDomain (const Domain& domain)
{
  wstate()[FDomain].replace() <<= new Domain(domain);
  // Find the polc(s) for the given domain.
  cdebug(2)<<"initializing for domain "<<domain<<endl;
  vector<Polc::Ref> polcs;
  if( itsTable ) 
  {
    int n = itsTable->getPolcs(polcs,itsName,domain);
    cdebug(3)<<n<<" polcs found in MEP table"<<endl;
  }
  else
  {
    cdebug(3)<<"no MEP table assigned"<<endl;
  }
  // If none found, try to get a default value.
  if( polcs.size() == 0 ) 
  {
    Polc::Ref polc;
    if( itsTable )
    {
      int n = itsTable->getInitCoeff(polc,itsName);
      cdebug(3)<<"looking for polcs in defaults subtable: "<<n<<endl;
    }
    if( !polc.valid() )
    {
      FailWhen(!itsDefault.valid(),"no polcs found and no default specified");
      cdebug(3)<<"no polcs found, using default value from state record"<<endl;
      polc.copy(itsDefault).privatize(DMI::WRITE|DMI::DEEP);
    }
    FailWhen(polc->getCoeff().isNull(),"no polcs found");
    polc().setDomain(domain);
    polcs.push_back(polc);
  }
  else if( isSolvable() )
  {
    if( polcs.size()>1 )
    {
      cdebug(3)<<"multiple polcs found for solvable parm, using first polc"<<endl;
      polcs.resize(1);
    }
    cdebug(3)<<"original domain: "<<polcs[0]->domain()<<endl;
    polcs[0]().setDomain(domain);
  } 
  else 
  {
    // Check if the polc domains cover the entire domain and if they
    // do not overlap.
  }
  itsPolcs = polcs;
  // Now determine the spids if the parm is solvable.
  int nr = initSolvable();
  // Store a ref to the polcs into the state record
  if( itsPolcs.size() == 1 )
    wstate()[FPolcs] <<= itsPolcs[0].copy();
  else
  {
    DataField & polcrec = wstate()[FPolcs].replace() <<= new DataField(TpMeqPolc,itsPolcs.size());
    for (uint i=0; i<itsPolcs.size(); i++) 
      polcrec[i] <<= itsPolcs[i].copy();
  }
  return nr;
}

//##ModelId=400E5353019E
int Parm::initSolvable ()
{
  int nr = 0;
  if( isSolvable() ) 
  {
    // For the time being we allow only one polc if the parameter
    // has to be solved.
    FailWhen( itsPolcs.size() != 1,"multiple polcs in solvable Parm");
    int spidIndex = 256*nodeIndex();
    nr += itsPolcs[0]().makeSolvable(spidIndex);
    if( itsPolcs[0]->getPerturbation() == 0 )
    {
      cdebug(3)<<"warning: null polc perturbation, using default 1e-6"<<endl;
      itsPolcs[0]().setPerturbation(1e-6);
    }
  } 
  else 
  {
    for( uint i=0; i<itsPolcs.size(); i++) 
      itsPolcs[i]().clearSolvable();
  }
  return nr;
}

//##ModelId=3F86886F022E
int Parm::getResult (Result::Ref &resref,
                     const std::vector<Result::Ref> &,
                     const Request &request,bool newreq)
{
  const Domain &domain = request.cells().domain();
  HIID domain_id = getDomainId(request.id()); 
  if( domain_id.empty() || domain_id != itsCurrentDomainId )
  {
    initDomain(domain);
    wstate()[FDomainId] = itsCurrentDomainId = domain_id;
  }
  // Create result object and attach to the ref that was passed in
  Result &result = resref <<= new Result(1,request); // result has one vellset
  VellSet & vs = result.setNewVellSet(0,0,request.calcDeriv());
  // return depencies: depends on parm value, if solvable
  // NB: should set UPDATED here if we've received a new parm value
  int retcode = itsIsSolvable ? RES_DEP_ITER : 0;
  // A single polc can be evaluated immediately.
  if( itsPolcs.size() == 1 ) 
  {
    itsPolcs[0]->evaluate(vs,request);
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
  for( uint i=0; i<itsPolcs.size(); i++ ) 
  {
    const Polc& polc = *(itsPolcs[i]);
    const Domain& polDom = polc.domain();
    if (firstMidFreq < polDom.endFreq() && lastMidFreq > polDom.startFreq()
        &&  firstMidTime < polDom.endTime() && lastMidTime > polDom.startTime()) 
    {
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
      Request partReq(partCells, request.calcDeriv());
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

//##ModelId=3F86886F023C
void Parm::save()
{
  if( itsTable ) 
  {
    for( uint i=0; i<itsPolcs.size(); i++) 
      itsTable->putCoeff(itsName,*itsPolcs[i]);
  }
}

//##ModelId=400E5353033A
void Parm::setStateImpl (DataRecord& rec, bool initializing)
{
  // inhibit changing of FPolcs field
  if( !initializing )
  {
    protectStateField(rec,FPolcs);
  }
  Function::setStateImpl(rec,initializing);
  getStateField(itsCurrentDomainId,rec,FDomainId);
  // Get solvable flag; clear domain if it changes (to force 
  // initDomain call next time 'round)
  bool oldSolvable = itsIsSolvable;
  getStateField(itsIsSolvable,rec,FSolvable);
  if (oldSolvable != itsIsSolvable) {
    itsCurrentDomainId = HIID();
  }
  // Are polcs specified? 
  int npolcs = rec[FPolcs].size(TpMeqPolc);
  FailWhen(npolcs<0,"illegal "+FPolcs.toString()+" state field");
  if( npolcs )
  {
    itsPolcs.resize(npolcs);
    if( npolcs == 1 )
      itsPolcs[0] <<= rec[FPolcs].as_wp<Polc>();
    else
      for( int i=0; i<npolcs; i++ )
        itsPolcs[i] <<= rec[FPolcs][i].as_wp<Polc>();
    initSolvable();
  }
  else
  {
    // Is the parm value specified? use it to update polcs
    // (ignore when initializing)
    if( rec[FValue].exists() ) 
    {
      // Update the polc coefficients with the new values.
      LoVec_double values = rec[FValue].as<LoVec_double>();
      ////    vector<double>& values = rec[FValue].as<vector<double> >();
      uint inx = 0;
      for (uint i=0; i<itsPolcs.size(); i++) {
        inx += itsPolcs[i]().update(&values(inx), values.size()-inx);
      }
      Assert (inx == uint(values.size()));
      // Also save the parms (might need to be changed later).
      save();
    }
  }
  // Get default polc (to be used if no table exists)
  if( rec[FDefault].exists() )
    itsDefault <<= rec[FDefault].as_p<Polc>();
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

//##ModelId=400E53520391
string Parm::sdebug (int detail, const string &prefix,const char* nm) const
{
  string out = Node::sdebug(detail,prefix,nm);
  if( detail>=2 || detail == -2) {
    Debug::appendf(out,"  parmtable=%s", itsTable->name().c_str());
  }
  return out;
}

} // namespace Meq
