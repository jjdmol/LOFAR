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
: Function  (),
  parmid_   (0),
  solvable_ (false),
  auto_save_(false),
  parmtable_(0)
{}

//##ModelId=3F86886F0242
Parm::Parm (const string& name, ParmTable* table,
	          const Polc::Ref::Xfer & defaultValue)
: Function   (),
  parmid_    (0),
  solvable_  (false),
  auto_save_ (false),
  name_      (name),
  parmtable_ (table),
  default_polc_(defaultValue)
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
  if( name_.empty() )
    wstate()[FParmName] = name_ = name(); 
}


void Parm::findRelevantPolcs (vector<Polc::Ref> &polcs,const Domain &domain)
{
  cdebug(2)<<"looking for suitable polcs"<<endl;
  if( parmtable_ )
  {
    int n = parmtable_->getPolcs(polcs,name_,domain);
    cdebug(3)<<n<<" polcs found in MEP table"<<endl;
  }
  else
  {
    cdebug(3)<<"no MEP table assigned"<<endl;
  }
  // If none found, try to get a default value.
  if( polcs.size() == 0 )
  {
    Polc::Ref defpolc;
    if( parmtable_ )
    {
      int n = parmtable_->getInitCoeff(defpolc,name_);
      cdebug(3)<<"looking for polcs in defaults subtable: "<<n<<endl;
    }
    if( !defpolc.valid() )
    {
      FailWhen(!default_polc_.valid(),"no polcs found and no default specified");
      cdebug(3)<<"no polcs found, using default value from state record"<<endl;
      defpolc.copy(default_polc_).privatize(DMI::WRITE|DMI::DEEP);
    }
    FailWhen(defpolc->getCoeff().isNull(),"no polcs found");
    defpolc().setDomain(domain);
    polcs.push_back(defpolc);
  }
}

//##ModelId=400E5353019E
int Parm::initSpids ()
{
  int nr = 0;
  for( uint i=0; i<polcs_.size(); i++) 
    polcs_[i]().clearSolvable();
  if( isSolvable() )
  {
    Polc & polc = polcs_.front();
    int spidIndex = 256*nodeIndex();
    nr += polc.makeSolvable(spidIndex);
    if( polc.getPerturbation() == 0 )
    {
      cdebug(3)<<"warning: null polc perturbation, using default 1e-6"<<endl;
      polc.setPerturbation(1e-6);
    }
  } 
  return nr;
}

// define binary predicate for comparing polcs
// compare by weight first, and dbid afterwards (higher dbids have priority)
class ComparePolcs {
  public: 
    int operator () (const Polc &a,const Polc &b) const {
      return a.getWeight() < b.getWeight() ||
           ( a.getWeight() == b.getWeight() && a.getDbId() < b.getDbId() );
    }
};
    
int Parm::initSolvable (const Domain &domain)
{
  // if solvable but polcs_ has more than one element, reduce it to
  // the single "most relevant" polc
  if( isSolvable() )
  {
    if( polcs_.size()>1 )
    {
      cdebug(3)<<"multiple polcs found for solvable parm, looking for best match"<<endl;
      // look for polc with max weight, and also an exact-domain polc with max weight
      int iexact=-1,imax=-1;
      for( uint i=0; i<polcs_.size(); i++ )
      {
        const Polc &np = *polcs_[i];
        if( imax<0 || ComparePolcs()(*polcs_[imax],np) )
          imax = i;
        if( np.domain() == domain && ( iexact<0 || ComparePolcs()(*polcs_[iexact],np) ) )
          iexact = i;
      }
      if( iexact>=0 )
      {
        cdebug(3)<<"using polc "<<iexact<<": exact domain match"<<endl;
        polcs_.front() = polcs_[iexact];
      }
      else
      {
        cdebug(3)<<"using polc "<<imax<<": no domain match"<<endl;
        polcs_.front() = polcs_[imax];
      }
      polcs_.resize(1);
    }
  // if parm is solvable, make sure the current domain overrides the solvable 
  // polc's domain (they may be different in case of growing domains, etc.)
    cdebug(3)<<"original domain: "<<polcs_.front()->domain()<<endl;
    polcs_.front()().setDomain(domain);
  }
  // assign spids
  return initSpids();
}

//##ModelId=3F86886F0226
int Parm::initDomain (const Domain& domain)
{
  cdebug(2)<<"initializing for domain "<<domain<<endl;
  // do we have a source for new polcs (i.e. table or default polc?)
  if( parmtable_ || default_polc_.valid() )
  {
    // Check if polc_ already contains a suitable polc
    if( polcs_.size() == 1 )
    {
      bool reuse = true;
      Polc & polc = polcs_.front();
      // check for cases where the current polc may be reused
      if( polc[FInfDomain].as<bool>(false) )
      { 
        cdebug(2)<<"current polc has infinite domain, re-using"<<endl;
      }
      else if( polc[FGrowDomain].as<bool>(false) &&
               polc.domain().subsetOf(domain) )
      {
        cdebug(2)<<"using growing domain for current polc"<<endl;
        polc.setDomain(domain); // extend the domain
      }
      else if( polc.domain().supersetOf(domain) )
      {
        cdebug(2)<<"current polc defined for superset of requested domain, re-using"<<endl;
      }
      else
        reuse = false;
      // if reusing, then set up solvability and return
      if( reuse )
      {
        initSolvable(domain);
        return 1;
      }
    }
    // no suitable polcs -- go looking
    vector<Polc::Ref> newpolcs;
    findRelevantPolcs(newpolcs,domain);
    polcs_ = newpolcs;
  }
  else
  {
    FailWhen(!polcs_.size(),"no polcs");
    cdebug(2)<<"no MEP table and no default specified, will use current polcs"<<endl;
    // some additional checking may be required here
  }
  // set/clear the solvable attributes, determine the spids, etc.
  initSolvable(domain);
  if( polcs_.size() == 1 )
    wstate()[FPolcs] <<= polcs_[0].copy();
  else
  {
    DataField & polcrec = wstate()[FPolcs].replace() <<= new DataField(TpMeqPolc,polcs_.size());
    for (uint i=0; i<polcs_.size(); i++) 
      polcrec[i] <<= polcs_[i].copy();
  }
  return polcs_.size();
}

//##ModelId=3F86886F022E
int Parm::getResult (Result::Ref &resref,
                     const std::vector<Result::Ref> &,
                     const Request &request,bool newreq)
{
  const Domain &domain = request.cells().domain();
  HIID domain_id = getDomainId(request.id()); 
  cdebug(2)<<"evaluating parm for domain "<<domain<<endl;
  if( domain_id.empty() || domain_id != domain_id_ )
  {
    cdebug(2)<<"domain changed, initializing"<<domain<<endl;
    initDomain(domain);
    wstate()[FDomain].replace() <<= new Domain(domain);
    wstate()[FDomainId] = domain_id_ = domain_id;
  }
  // Create result object and attach to the ref that was passed in
  Result &result = resref <<= new Result(1,request); // result has one vellset
  VellSet & vs = result.setNewVellSet(0,0,request.calcDeriv());
  // return depencies: depends on parm value, if solvable
  // NB: should set UPDATED here if we've received a new parm value
  int retcode = solvable_ ? RES_DEP_ITER : 0;
  // A single polc can be evaluated immediately.
  if( polcs_.size() == 1 ) 
  {
    cdebug(3)<<"evaluating and returning single polc"<<endl;
    polcs_[0]->evaluate(vs,request);
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
  vs.setReal(ndFreq, ndTime);
  // Iterate over all polynomials.
  // Evaluate one if its domain overlaps the request domain.
  cdebug(3)<<"midfreqs: "<<firstMidFreq<<":"<<lastMidFreq<<endl;
  cdebug(3)<<"midtimes: "<<firstMidTime<<":"<<lastMidTime<<endl;
  cdebug(3)<<"evaluating for "<<polcs_.size()<<" polcs"<<endl;
  for( uint i=0; i<polcs_.size(); i++ )
  {
    const Polc& polc = *(polcs_[i]);
    cdebug(3)<<"polc "<<i<<" domain is "<<polc.domain()<<endl;
    double pfreq0 = polc.domain().startFreq(), 
           pfreq1 = polc.domain().endFreq(),
           ptime0 = polc.domain().startTime(), 
           ptime1 = polc.domain().endTime();
    if( firstMidFreq < pfreq1 && lastMidFreq > pfreq0 &&
        firstMidTime < ptime1 && lastMidTime > ptime0 )
    {
      // Determine which part of the request domain is covered by the
      // polynomial.
      int stFreq = 0;
      if (firstMidFreq < pfreq0) {
        stFreq = 1 + int((pfreq0 - firstMidFreq) / stepFreq);
      }
      int nrFreq = ndFreq - stFreq;
      if (lastMidFreq > pfreq1) {
        int remFreq = 1 + int((lastMidFreq - pfreq1) / stepFreq);
        nrFreq -= remFreq;
      }
      int stTime = 0;
      while (cells.time(stTime) < ptime0) {
        stTime++;
      }
      int lastTime = ndTime-1;
      while (cells.time(lastTime) > ptime1) {
        lastTime--;
      }
      int nrTime = lastTime - stTime + 1;
      cdebug(3)<<"polc "<<i<<" overlap: "<<stFreq<<" "<<nrFreq
                <<" "<<stTime<<" "<<nrTime<<endl;
      // If the overlap is full, only this polynomial needs to be evaluated.
      if( stFreq == 0 && nrFreq == ndFreq &&
          stTime == 0 && nrTime == ndTime ) {
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
        partStartTime(j) = cells.time(stTime+j) - cells.stepTime(stTime+j)/2;
        partEndTime(j)   = cells.time(stTime+j) + cells.stepTime(stTime+j)/2;
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
  if( parmtable_ ) 
  {
    cdebug(2)<<"saving "<<polcs_.size()<<" polcs"<<endl;
    for( uint i=0; i<polcs_.size(); i++) 
      parmtable_->putCoeff1(name_,polcs_[i]());
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
  getStateField(domain_id_,rec,FDomainId);
  getStateField(auto_save_,rec,FAutoSave);
  getStateField(name_,rec,FParmName);
  // Get solvable flag; clear domain if it changes (to force 
  // initDomain call next time 'round)
  bool oldSolvable = solvable_;
  getStateField(solvable_,rec,FSolvable);
  // reset domain ID, if solvability changes
  if( oldSolvable != solvable_) 
    domain_id_ = HIID();
  // Are polcs specified? 
  int npolcs = rec[FPolcs].size(TpMeqPolc);
  FailWhen(npolcs<0,"illegal "+FPolcs.toString()+" state field");
  if( npolcs )
  {
    polcs_.resize(npolcs);
    if( npolcs == 1 )
      polcs_[0] <<= rec[FPolcs].as_wp<Polc>();
    else
      for( int i=0; i<npolcs; i++ )
        polcs_[i] <<= rec[FPolcs][i].as_wp<Polc>();
    // reset domain ID
    domain_id_ = HIID();
  }
  // Get default polc (to be used if no table exists)
  if( rec[FDefault].exists() )
    default_polc_ <<= rec[FDefault].as_p<Polc>();
  // Get ParmTable name 
  if( rec[FTableName].exists() )
  {
    string tableName = state()[FTableName].as<string>();
    if( tableName.empty() )  // no table
      parmtable_ = 0;
    else    // else open a table
      parmtable_ = ParmTable::openTable(tableName);
  }
}

void Parm::processCommands (const DataRecord &rec,const Request &req)
{
  // process parent class's commands
  Function::processCommands(rec,req);
  bool saved  = False;
  // Is the parm value specified? use it to update polcs
  DataRecord::Hook hset(rec,FSetValue);
  if( hset.exists() )
  {
    cdebug(4)<<"got "<<FSetValue<<" command"<<endl;
    // Update the polc coefficients with the new values.
    LoVec_double values = hset.as<LoVec_double>();
    uint inx = 0;
    for (uint i=0; i<polcs_.size(); i++) 
      inx += polcs_[i]().update(&values(inx), values.size()-inx);
    Assert(inx == uint(values.size()));
    if( auto_save_ )
    {
      save();
      saved = True;
    }
  }
  // if not already saved, then check for a Save.Polc command
  if( !saved && rec[FSavePolc].as<bool>(true) )
    save();
}

//##ModelId=400E53520391
string Parm::sdebug (int detail, const string &prefix,const char* nm) const
{
  string out = Node::sdebug(detail,prefix,nm);
  if( detail>=2 || detail == -2) {
    Debug::appendf(out,"  parmtable=%s", parmtable_->name().c_str());
  }
  return out;
}

} // namespace Meq
