//# MeqParmFunklet.cc: Stored parameter with funklet coefficients
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

#include <lofar_config.h>
#include <BBSKernel/MNS/MeqParmFunklet.h>
#include <BBSKernel/MNS/MeqParmExpr.h>
#include <BBSKernel/MNS/MeqPolc.h>
#include <BBSKernel/ParmData.h>
#include <ParmDB/ParmValue.h>
#include <ParmDB/ParmDB.h>
#include <Common/LofarLogger.h>
#include <casa/BasicMath/Math.h>

using namespace casa;
using namespace std;

// Make sorting vector<MPFHolder> possible (on domain).
// So define operator< for it.
bool less_mp (LOFAR::BBS::MeqFunklet* x, LOFAR::BBS::MeqFunklet* y)
{
  return (x->domain().startY() < y->domain().startY()  ||
	  (x->domain().startY() == y->domain().startY()  &&
	   x->domain().startX() <  y->domain().startX()));
}


namespace LOFAR
{
namespace BBS
{

MeqParmFunklet::MeqParmFunklet (const string& name, LOFAR::ParmDB::ParmDB* table)
: MeqParm    (name),
  itsDefUsed (false),
  itsNrPert  (0),
  itsPertInx (-1),
  itsTable   (table)
{}

MeqParmFunklet::~MeqParmFunklet()
{
  removeFunklets();
}

MeqExpr MeqParmFunklet::create (const string& name,
				 MeqParmGroup& group,
				 LOFAR::ParmDB::ParmDB* table)
{
  // If the parm already exists, return it.
  MeqParmGroup::iterator pos = group.find (name);
  if (pos != group.end()) {
    return pos->second;
  }
  // If the parm is an expression, use that.
  map<string,LOFAR::ParmDB::ParmValueSet> pset;
  table->getDefValues (pset, name);
  if (! pset.empty()) {
    LOFAR::ParmDB::ParmValueSet& pvset = pset.begin()->second;
    if (! pvset.getValues().empty()) {
      LOFAR::ParmDB::ParmValueRep& pv = pvset.getValues()[0].rep();
      if (pv.itsType == "parmexpr") {
	return new MeqParmExpr (pv.itsExpr, group, table);
      }
    }
  }
  // It is a normal funklet.
  MeqPExpr expr (new MeqParmFunklet (name, table));
  group.add (expr);
  return expr;
}

void MeqParmFunklet::add (const MeqFunklet& funklet)
{
  itsFunklets.push_back (funklet.clone());
}

MeqResult MeqParmFunklet::getResult (const MeqRequest& request)
{
  int nrpert = request.nspid() > 0  ?  itsNrPert : 0;
  // A single funklet can be evaluated immediately.
  if (itsFunklets.size() == 1) {
    MeqResult res = itsFunklets[0]->getResult (request,
					       itsNrPert, itsPertInx);
    for (int ip=0; ip<nrpert; ip++) {
      res.setPerturbedParm (itsPertInx+ip, this);
    }
    return res;
  }

  // Get the domain, etc.
  const MeqDomain& domain = request.domain();
  MeqResult result(0);
  int ndx = request.nx();
  int ndy = request.ny();
  double stepx = request.stepX();
  double halfStepx = stepx * .5;
  double firstMidx = domain.startX() + halfStepx;
  double lastMidx = firstMidx + (ndx-1) * stepx;
  double firstMidy = request.y(0);
  double lastMidy = request.y(ndy-1);
  // Create the result matrix and initialize to zero.
  MeqMatrix mat(0., ndx, ndy);
  result.setValue (mat);
  // Create and initialize the perturbed values.
  for (int ip=0; ip<nrpert; ip++) {
    MeqMatrix matp(0., ndx, ndy);
    result.setPerturbedValue (itsPertInx+ip, matp);
    result.setPerturbedParm (itsPertInx+ip, this);
  }

//  LOG_DEBUG_STR("Name: " << getName() << " nrpert: " << nrpert << " ["
//    << itsPertInx << " - " << itsPertInx + nrpert - 1 << "]");

  // Iterate over all funklets.
  // Evaluate one if its domain overlaps the request domain.
  int sty = 0;
  int endy = 0;
  for (uint i=0; i<itsFunklets.size(); i++) {
    MeqFunklet& funklet = *itsFunklets[i];
    const MeqDomain& polDom = funklet.domain();
    if (firstMidx < polDom.endX()  &&  lastMidx >= polDom.startX()
    &&  firstMidy < polDom.endY()  &&  lastMidy >= polDom.startY()) {
      // Determine which part of the request domain is covered by the
      // funklet.
      int stx = 0;
      if (firstMidx < polDom.startX()) {
	stx = 1 + int((polDom.startX() - firstMidx) / stepx);
      }
      int nrx = ndx - stx;
      if (lastMidx > polDom.endX()) {
	int remx = 1 + int((lastMidx - polDom.endX()) / stepx);
	nrx -= remx;
      }
      // Try to find the y values for this domain.
      // Start searching at the end of the previous one.
      sty = endy;
      if (sty >= request.ny()  ||  request.y(sty) > polDom.startY()) {
	sty = 0;                            // restart at begin
      }
      while (request.y(sty) < polDom.startY()) {
	sty++;
      }
      endy = sty;
      while (endy < ndy  &&  request.y(endy) < polDom.endY()) {
	endy++;
      }
      int nry = endy - sty;
      // If the overlap is full, only this funklet needs to be evaluated.
      if (stx == 0  &&  nrx == ndx  &&  sty == 0  &&  nry == ndy) {
	MeqResult res = funklet.getResult (request, itsNrPert, itsPertInx);
	for (int ip=0; ip<nrpert; ip++) {
	  res.setPerturbedParm (itsPertInx+ip, this);
	}
	return res;
      }
      // Form the request for the overlapping part and evaluate the funklet.
      MeqRequest partReq(request, stx, nrx, sty, nry);
      MeqResult partRes = funklet.getResult (partReq, itsNrPert, itsPertInx);
      // Move the values to the correct place in the output result.
      // Note that in principle a funklet could be a single coefficient
      // in which case it returns a single value.
      const double* from = partRes.getValue().doubleStorage();
      double* to = result.getValueRW().doubleStorage() + stx + sty*ndx;
      if (partRes.getValue().nelements() == 1) {
	for (int iy=0; iy<nry; iy++) {
	  for (int ix=0; ix<nrx; ix++) {
	    to[ix] = *from;
	  }
	  to += ndx;
	}
      } else {
	for (int iy=0; iy<nry; iy++) {
	  for (int ix=0; ix<nrx; ix++) {
	    to[ix] = *from++;
	  }
	  to += ndx;
	}
      }
      // Copy all perturbed values.
      for (int ip=0; ip<nrpert; ip++) {
	int pertinx = itsPertInx+ip;
	const MeqMatrix& pertfr = partRes.getPerturbedValue (pertinx);
	const double* pfrom = pertfr.doubleStorage();
	MeqMatrix& pertto = result.getPerturbedValueRW (pertinx);
	double* pto = pertto.doubleStorage() + stx + sty*ndx;
	if (pertfr.nelements() == 1) {
	  for (int iy=0; iy<nry; iy++) {
	    for (int ix=0; ix<nrx; ix++) {
	      pto[ix] = *pfrom;
	    }
	    pto += ndx;
	  }
	} else {
	  for (int iy=0; iy<nry; iy++) {
	    for (int ix=0; ix<nrx; ix++) {
	      pto[ix] = *pfrom++;
	    }
	    pto += ndx;
	  }
	}
      }
    }
  }
  return result;
}

LOFAR::ParmDB::ParmDBMeta MeqParmFunklet::getParmDBMeta() const
{
  return itsTable->getParmDBMeta();
}

int MeqParmFunklet::getParmDBSeqNr() const
{
  return itsTable->getParmDBSeqNr();
}

void MeqParmFunklet::removeFunklets()
{
  for (vector<MeqFunklet*>::iterator iter = itsFunklets.begin();
       iter != itsFunklets.end();
       iter++) {
    delete *iter;
  }
  itsFunklets.clear();
}

void MeqParmFunklet::fillFunklets 
                (const map<string,LOFAR::ParmDB::ParmValueSet>& parmSet,
		 const MeqDomain& domain)
{
  // Only fill if not filled yet.
  if (! itsFunklets.empty()) {
    return;
  }
  LOFAR::ParmDB::ParmDomain pdomain(domain.startX(), domain.endX(),
			     domain.startY(), domain.endY());
  bool found = false;
  // Try to find the funklets in the parm set.
  map<string,LOFAR::ParmDB::ParmValueSet>::const_iterator pos =
                                                   parmSet.find(getName());
  if (pos != parmSet.end()) {
    const vector<LOFAR::ParmDB::ParmValue>& vec = pos->second.getValues();
    if (vec.size() > 0) {
      // Check if the funklet domains cover the entire domain and if they
      // do not overlap.  /// Not implemented yet!!!
      // Convert all parmvalues to funklets.
      itsFunklets.resize (0);
      for (uint i=0; i<vec.size(); ++i) {
	itsFunklets.push_back (MeqFunklet::make (vec[i], getName()));
      }
      itsDefUsed = false;
      found = true;
      // Sort the funklets in order of domain.
      if (itsFunklets.size() > 1) {
	std::sort (itsFunklets.begin(), itsFunklets.end(), &less_mp);
      }
    }
  }
  if (!found) {
    // No value found, so use a default value.
    LOFAR::ParmDB::ParmValue pval = itsTable->getDefValue (getName());
    ASSERTSTR (!pval.rep().itsShape.empty(),
	       "No value found for parameter " << getName()
	       << " in given domain");
    pval.rep().setDomain (pdomain);
    itsFunklets.push_back (MeqFunklet::make (pval, getName()));
    itsDefUsed = true;
  }
  itsWorkDomain = domain;
}

int MeqParmFunklet::initDomain (const vector<MeqDomain>& solveDomains,
				int& pertIndex, vector<int>& scidIndex)
{
  itsNrPert  = 0;
  itsPertInx = -1;
  if (isSolvable()) {
    // See if the values of the previous domain can be used.
    // We only do that if the default was used in a single previous domain.
    ///    if (itsDefUsed  &&  polcs.size() == 1) {
    ///      polcs[0].setDomain (domain);
    ///      polcs[0].setNewParm();
    ///    } else {

    uint nDomain = solveDomains.size();
    ASSERTSTR (nDomain > 0 &&
	       solveDomains[0].endX()>itsWorkDomain.startX() &&
	       solveDomains[0].endY()>itsWorkDomain.startY() &&
	       solveDomains[nDomain-1].startX()<itsWorkDomain.endX() &&
	       solveDomains[nDomain-1].startY()<itsWorkDomain.endY(),
	       "MeqParmFunklet::initDomain of parm " << getName() << " - "
	       "solvedomains " << solveDomains[0] << solveDomains[nDomain-1]
	       << " mismatch workdomain " << itsWorkDomain
	       << " given in last fillFunklets");
    // The nr of solve domains should match the nr of funklets.
    // However, if a default value is used, only one funklet is in use.
    // In that case more funklets are created if needed.
    if (itsDefUsed  &&  nDomain > 1  &&  itsFunklets.size() != nDomain) {
      ASSERT (itsFunklets.size() == 1);
      itsFunklets[0]->setDomain (solveDomains[0]);
      itsFunklets.resize (nDomain);
      for (uint i=1; i<nDomain; ++i) {
	itsFunklets[i] = itsFunklets[0]->clone();
	itsFunklets[i]->setDomain (solveDomains[i]);
      }
    }
    ASSERTSTR (itsFunklets.size() == nDomain,
	       "Solvable parameter " << getName() << " has "
	       << itsFunklets.size() << " funklet domains mismatching the "
	       << nDomain << " solve domains for work domain "
	       << itsWorkDomain);
    for (uint i=0; i<nDomain; ++i) {
      const MeqDomain& polDom = itsFunklets[i]->domain();
      const MeqDomain& solDom = solveDomains[i];
      ASSERTSTR (near(solDom.startX(), polDom.startX())  &&
		 near(solDom.endX(), polDom.endX())  &&
		 near(solDom.startY(), polDom.startY())  &&
		 near(solDom.endY(), polDom.endY()),
		 "Solvable parameter " << getName() <<
		 " has a partially instead of fully matching entry for "
		 "solve domain " << solDom << endl
		 << '(' << polDom << ')');
    }
    // Determine the solvable coeff ids for each funklet.
    // The highest nr of scids is the nr of perturbed values.
    itsPertInx = pertIndex;
    for (uint i=0; i<nDomain; ++i) {
//      int nrs = itsFunklets[i]->makeSolvable (scidIndex[i]);
      int nrs = itsFunklets[i]->makeSolvable (itsPertInx);
      scidIndex[i] += nrs;
      if (nrs > itsNrPert) {
      	itsNrPert = nrs;
      }
    }
//    itsPertInx = pertIndex;
    pertIndex += itsNrPert;
  } else {
    for (uint i=0; i<itsFunklets.size(); i++) {
      itsFunklets[i]->clearSolvable();
    }
  }    
  return itsNrPert;
}

const vector<MeqFunklet*>& MeqParmFunklet::getFunklets() const
{
  return itsFunklets;
}

void MeqParmFunklet::update (const ParmData& values)
{
  ASSERT(values.size() == int(itsFunklets.size()));
  for (int i=0; i<values.size(); ++i) {
    itsFunklets[i]->update (values.getCoeff(i));
  }
}

void MeqParmFunklet::update (const vector<double>& value)
{
  for (uint i=0; i<itsFunklets.size(); i++) {
    itsFunklets[i]->update (value);
  }
}

void MeqParmFunklet::updateFromTable()
{
  uint nDomain = itsFunklets.size();
  ASSERTSTR (nDomain > 0 &&
	     itsFunklets[0]->domain().startX()==itsWorkDomain.startX() &&
	     itsFunklets[0]->domain().startY()==itsWorkDomain.startY() &&
	     itsFunklets[nDomain-1]->domain().endX()==itsWorkDomain.endX() &&
	     itsFunklets[nDomain-1]->domain().endY()==itsWorkDomain.endY(),
	     "MeqParmFunklet::updateFromTable - "
	     "domain mismatches domain given in itsFunklets");
  // Find the funklet(s) for the current domain.
  LOFAR::ParmDB::ParmDomain pdomain = itsWorkDomain.toParmDomain();
  LOFAR::ParmDB::ParmValueSet pset = itsTable->getValues (getName(), pdomain);
  ASSERT(pset.getValues().size() == 1);
  MeqPolc newpolc (pset.getValues()[0]);
  // Update the coefficients with the new ones.
  //// update (newpolc.getCoeff());
}

void MeqParmFunklet::save()
{
//  itsTable->lock();
  for (unsigned int i=0; i<itsFunklets.size(); i++) {
    LOFAR::ParmDB::ParmValue pval = itsFunklets[i]->getParmValue();
    itsTable->putValue (getName(), pval);
  }
//  itsTable->unlock();
}

void MeqParmFunklet::update(size_t domain, const vector<double> &unknowns)
{
    ASSERT(domain < itsFunklets.size());
    itsFunklets[domain]->update(unknowns);
}

void MeqParmFunklet::save(size_t domainIndex)
{
  ASSERT(domainIndex < itsFunklets.size());
  //itsTable->lock();
  LOFAR::ParmDB::ParmValue pval = itsFunklets[domainIndex]->getParmValue();
  itsTable->putValue(getName(), pval);
  //itsTable->unlock();
}

} // namespace BBS
} // namespace LOFAR
