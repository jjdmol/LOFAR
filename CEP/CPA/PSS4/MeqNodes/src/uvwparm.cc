//# uvwparm.cc: Fit polynomials to UVW coordinates and store in MEP
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

#include <MEQ/Parm.h>
#include <MEQ/Polc.h>
#include <MEQ/Cells.h>
#include <MEQ/Vells.h>
#include <MEQ/Request.h>
#include <MEQ/VellSet.h>
#include <MEQ/ParmTable.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Slice.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Containers/Block.h>
#include <scimath/Functionals/Polynomial.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <scimath/Fitting/LinearFit.h>
#include <casa/Exceptions/Error.h>
#include <Common/Debug.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <exception>


Meq::Polc calcCoeff (const Vector<double>& times,
		     const Vector<double>& values,
		     int order)
{
  int nr = times.nelements();
  Assert (values.nelements() == nr);
  double step = times(1) - times(0);
  Meq::Domain domain (0, 1e15, times(0)-step/2, times(nr-1)+step/2);
  Vector<double> normTimes(nr);
  for (int i=0; i<nr; i++) {
    normTimes(i) = times[i] - domain.startTime();
  }
  Meq::Polc polc;
  polc.setDomain (domain);
  polc.setTime0 (domain.startTime());

  Polynomial<AutoDiff<double> > poly(order);
  LinearFit<Double> fitter;
  fitter.setFunction (poly);
  Vector<double> sigma(nr, 1);
  Vector<Double> sol = fitter.fit (normTimes, values, sigma);
  LoMat_double lomat(sol.data(), LoMatShape(1,order),
		     blitz::neverDeleteData);
  polc.setCoeff (Meq::Vells(lomat));
  polc.setSimCoeff (Meq::Vells(lomat));
  lomat = 1.;
  polc.setPertSimCoeff (Meq::Vells(lomat));
  return polc;
}


void calcPolc (Meq::ParmTable& ptab, const std::vector<bool>& statFnd,
	       int order, double timeLength, Vector<double>& times,
	       Matrix<Double>& values, const string& parm)
{
  int nrtim = times.nelements();
  // Divide the data into chunks of 1 hour (assuming it is equally spaced).
  // Make all chunks about equally long.
  double interval = (times(nrtim-1) - times(0)) / (nrtim-1);
  int chunkLength = int(timeLength / interval + 0.5);
  int nrChunk = 1 + (nrtim-1) / chunkLength;
  chunkLength = nrtim / nrChunk;
  int chunkRem = nrtim % nrChunk;
  Assert (chunkLength > 4);
  // Loop through the data in periods of one hour.
  // Take care that the remainder is evenly spread over the chunks.
  char antName[32];
  for (int ant=0; ant<values.ncolumn(); ant++) {
    if (statFnd[ant]) {
      int nrdone = 0;
      snprintf(antName, 32, "[s=%d]", ant);
      for (int i=0; i<chunkRem; i++) {
	Matrix<double> vals = values(Slice(nrdone,chunkLength+1),
				     Slice(ant,1));
	Meq::Polc polc = calcCoeff (times(Slice(nrdone,chunkLength+1)),
				    vals.reform(IPosition(1,chunkLength+1)),
				    order);
	nrdone += chunkLength+1;
	ptab.putCoeff (parm+antName, polc);
      }
      for (int i=chunkRem; i<nrChunk; i++) {
	Matrix<double> vals = values(Slice(nrdone,chunkLength),
				     Slice(ant,1));
	Meq::Polc polc = calcCoeff (times(Slice(nrdone,chunkLength)),
				    vals.reform(IPosition(1,chunkLength)),
				    order);
	nrdone += chunkLength;
	ptab.putCoeff (parm+antName, polc);
      }
      Assert (nrdone == nrtim);
    }
  }
}


void findUVW (const string& msName, const string& mepName,
	      int order, double timeLength)
{
  Table ms(msName);
  Block<String> keys(3);
  keys[0] = "TIME";
  keys[1] = "ANTENNA1";
  keys[2] = "ANTENNA2";
  Table sorms = ms.sort (keys, Sort::Ascending,
			 Sort::NoDuplicates||Sort::QuickSort);
  // Find number of antennas.
  int nant = 1+std::max(max(ROScalarColumn<Int>(sorms,"ANTENNA1").getColumn()),
		      max(ROScalarColumn<Int>(sorms,"ANTENNA2").getColumn()));
  int ntime;
  {
    Table timms = sorms.sort ("TIME", Sort::Ascending,
			      Sort::NoDuplicates|Sort::InsSort);
    ntime = timms.nrow();
  }
  cdebug(1) << "get UVW coordinates from MS" << endl;
  std::vector<bool> statFnd (nant);
  std::vector<bool> statDone (nant);
  std::vector<double> statuvw(3*nant);
  Matrix<double> u(ntime, nant);
  Matrix<double> v(ntime, nant);
  Matrix<double> w(ntime, nant);
  u = 0;
  v = 0;
  w = 0;
  Vector<double> times(ntime);
  // Step time by time through the MS.
  TableIterator iter(sorms, "TIME", TableIterator::Ascending,
		     TableIterator::NoSort);
  int timinx = 0;
  while (!iter.pastEnd()) {
    statFnd.assign (statFnd.size(), false);
    statDone.assign (statFnd.size(), false);
    Table tab = iter.table();
    ROScalarColumn<double> timeCol (tab, "TIME");
    ROScalarColumn<int>    ant1Col (tab, "ANTENNA1");
    ROScalarColumn<int>    ant2Col (tab, "ANTENNA2");
    ROArrayColumn<double>  uvwCol  (tab, "UVW");
    double time = timeCol(0);
    times[timinx] = time;
    int nfnd = 0;
    Vector<int> ant1d = ant1Col.getColumn();
    Vector<int> ant2d = ant2Col.getColumn();
    int* ant1 = &(ant1d(0));
    int* ant2 = &(ant2d(0));
    for (unsigned int i=0; i<ant1d.nelements(); i++) {
      if (!statFnd[ant1[i]]) {
	nfnd++;
	statFnd[ant1[i]] = true;
      }
      if (!statFnd[ant2[i]]) {
	nfnd++;
	statFnd[ant2[i]] = true;
      }
    }
    // Set UVW of first station to 0 (UVW coordinates are relative!).
    Matrix<double> uvwd = uvwCol.getColumn();
    double* uvw = &(uvwd(0,0));
    statDone.assign (statDone.size(), false);
    statuvw[3*ant1[0]]   = 0;
    statuvw[3*ant1[0]+1] = 0;
    statuvw[3*ant1[0]+2] = 0;
    statDone[ant1[0]] = true;
    int ndone = 1;
    // Loop until all found stations are handled.
    while (ndone < nfnd) {
      int nd = 0;
      for (unsigned int i=0; i<ant1d.nelements(); i++) {
	int a1 = ant1[i];
	int a2 = ant2[i];
	if (!statDone[a2]) {
	  if (statDone[a1]) {
	    statuvw[3*a2]   = uvw[3*i]   + statuvw[3*a1];
	    statuvw[3*a2+1] = uvw[3*i+1] + statuvw[3*a1+1];
	    statuvw[3*a2+2] = uvw[3*i+2] + statuvw[3*a1+2];
	    statDone[a2] = true;
	    u(timinx,a2) = statuvw[3*a2];
	    v(timinx,a2) = statuvw[3*a2+1];
	    w(timinx,a2) = statuvw[3*a2+2];
	    ndone++;
	    nd++;
	  }
	} else if (!statDone[a1]) {
	  if (statDone[a2]) {
	    statuvw[3*a1]   = statuvw[3*a2]   - uvw[3*i];
	    statuvw[3*a1+1] = statuvw[3*a2+1] - uvw[3*i+1];
	    statuvw[3*a1+2] = statuvw[3*a2+2] - uvw[3*i+2];
	    statDone[a1] = true;
	    u(timinx,a1) = statuvw[3*a1];
	    v(timinx,a1) = statuvw[3*a1+1];
	    w(timinx,a1) = statuvw[3*a1+2];
	    ndone++;
	    nd++;
	  }
	}
	if (ndone == nfnd) {
	  break;
	}
      }
      Assert (nd > 0);
    }
    iter++;
    timinx++;
  }
  Meq::ParmTable::createTable (mepName);
  Meq::ParmTable ptab(mepName);
  calcPolc (ptab, statFnd, order, timeLength, times, u, "U");
  calcPolc (ptab, statFnd, order, timeLength, times, v, "V");
  calcPolc (ptab, statFnd, order, timeLength, times, w, "W");
}


void checkUVW (const string& msName, const string& mepName, double threshold)
{
  Meq::ParmTable ptab(mepName);
  Table ms(msName);
  Block<String> keys(3);
  keys[0] = "ANTENNA1";
  keys[1] = "ANTENNA2";
  keys[2] = "TIME";
  Table sorms = ms.sort (keys, Sort::Ascending,
			 Sort::NoDuplicates||Sort::QuickSort);
  Block<String> ikeys(2);
  ikeys[0] = "ANTENNA1";
  ikeys[1] = "ANTENNA2";
  TableIterator iter(sorms, ikeys, TableIterator::Ascending,
		     TableIterator::NoSort);
  double maxdu = 0;
  double maxdv = 0;
  double maxdw = 0;
  char parmName[32];
  while (!iter.pastEnd()) {
    int ant1 = ROScalarColumn<int>(iter.table(), "ANTENNA1")(0);
    int ant2 = ROScalarColumn<int>(iter.table(), "ANTENNA2")(0);
    ROScalarColumn<double> timeCol(iter.table(), "TIME");
    ROArrayColumn<double> uvwCol(iter.table(), "UVW");
    Vector<Double> times = timeCol.getColumn();
    double step = times(1) - times(0);
    Matrix<Double> uvw = uvwCol.getColumn();
    int nrtim = times.nelements();
    Meq::Domain domain(1, 2, times(0)-step/2, times(nrtim-1)+step/2);
    Meq::Cells cells(domain, 1, nrtim);
    Meq::Request request(cells, false);
    Meq::Result::Ref resset1,resset2;
    const Meq::VellSet *res1,*res2;
    {
      snprintf (parmName, 32, "U[s=%d]", ant1);
      Meq::Parm pu1(parmName, &ptab);
      snprintf (parmName, 32, "U[s=%d]", ant2);
      Meq::Parm pu2(parmName, &ptab);
      pu1.execute (resset1, request);
      pu2.execute (resset2, request);
      res1 = &(resset1->vellSet(0));
      res2 = &(resset2->vellSet(0));
      LoMat_double diff(LoMatShape(1,nrtim));
      diff = res2->getValue().getRealArray() - res1->getValue().getRealArray();
      Vector<double> vec (IPosition(1,nrtim), diff.data(), SHARE);
      double d = max(abs(vec-uvw.row(0)));
      maxdu = std::max(d, maxdu);
      if (d > threshold) {
	std::cout << ant1 << '-' << ant2 << " U=" << vec-uvw.row(0)
		  << std::endl;
      }
    }
    {
      snprintf (parmName, 32, "V[s=%d]", ant1);
      Meq::Parm pu1(parmName, &ptab);
      snprintf (parmName, 32, "V[s=%d]", ant2);
      Meq::Parm pu2(parmName, &ptab);
      pu1.execute (resset1, request);
      pu2.execute (resset2, request);
      res1 = &(resset1->vellSet(0));
      res2 = &(resset2->vellSet(0));
      LoMat_double diff(LoMatShape(1,nrtim));
      diff = res2->getValue().getRealArray() - res1->getValue().getRealArray();
      Vector<double> vec (IPosition(1,nrtim), diff.data(), SHARE);
      double d = max(abs(vec-uvw.row(1)));
      maxdv = std::max(d, maxdv);
      if (d > threshold) {
	std::cout << ant1 << '-' << ant2 << " V=" << vec-uvw.row(1)
		  << std::endl;
      }
    }
    {
      snprintf (parmName, 32, "W[s=%d]", ant1);
      Meq::Parm pu1(parmName, &ptab);
      snprintf (parmName, 32, "W[s=%d]", ant2);
      Meq::Parm pu2(parmName, &ptab);
      pu1.execute (resset1, request);
      pu2.execute (resset2, request);
      res1 = &(resset1->vellSet(0));
      res2 = &(resset2->vellSet(0));
      LoMat_double diff(LoMatShape(1,nrtim));
      diff = res2->getValue().getRealArray() - res1->getValue().getRealArray();
      Vector<double> vec (IPosition(1,nrtim), diff.data(), SHARE);
      double d = max(abs(vec-uvw.row(2)));
      maxdw = std::max(d, maxdw);
      if (d > threshold) {
	std::cout << ant1 << '-' << ant2 << " W=" << vec-uvw.row(2)
		  << std::endl;
      }
    }
    iter++;
  }
  std::cout << "maxdiff: U=" << maxdu << "  V=" << maxdv
	    << "  W=" << maxdw << std::endl;
}

int main(int argc, char* argv[])
{
  if (argc < 3) {
    std::cerr <<
      "Run as:  uvwparm msname mepname [order(=2)] [timedomain in sec(=1800)]"
	      << "[check_threshold]" << std::endl;
    return 1;
  }
  try {
    int order=2;
    double timeLength=1800;
    if (argc > 3) {
      std::istringstream os(argv[3]);
      os >> order;
    }
    if (argc > 4) {
      std::istringstream os(argv[4]);
      os >> timeLength;
    }
    std::cout << "Fitting UVW ..." << std::endl;
    findUVW (argv[1], argv[2], order, timeLength);
    if (argc > 5) {
      double threshold = 0.01;
      std::istringstream os(argv[5]);
      os >> threshold;
      std::cout << "Checking UVW ..." << std::endl;
      checkUVW (argv[1], argv[2], threshold);
    }
  } catch (AipsError& x) {
    std::cerr << "Caught AIPS++ exception: " << x.getMesg() << std::endl;
    return 1;
  } catch (std::exception& x) {
    std::cerr << "Caught std exception: " << x.what() << std::endl;
    return 1;
  }
  std::cerr << "OK" << std::endl;
  return 0;
}
