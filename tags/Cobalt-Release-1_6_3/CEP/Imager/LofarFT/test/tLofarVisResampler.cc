//# tLofarVisResampler.cc: Test program for class LofarVisResampler
//#
//# Copyright (C) 2011
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
#include <LofarFT/LofarVisResampler.h>
#include <LofarFT/LofarCFStore.h>
#include <Common/OpenMP.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Utilities/GenSort.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/BasicSL/Constants.h>
#include <casa/OS/Timer.h>

using namespace LOFAR;
using namespace casa;

void fillBaselines (int nrant, Vector<Int>& ant1, Vector<Int>& ant2,
                    Matrix<Double>& uvw)
{
  // Create fake uvw coordinates for each antenna.
  Matrix<Double> antuvw(3, nrant);
  indgen (antuvw, 0., 50.);
  ant1.resize (nrant*(nrant+1)/2);
  ant2.resize (ant1.size());
  uvw.resize (3, ant1.size());
  int inx = 0;
  for (int i=0; i<nrant; ++i) {
    for (int j=i; j<nrant; ++j) {
      ant1[inx] = j;
      ant2[inx] = i;
      uvw.column(inx) = antuvw.column(j) - antuvw.column(i);
      inx++;
    }
  }
}

void fillVBStore (LofarVBStore& vbs, const Matrix<Double>& uvw)
{
  int nrbl = uvw.nrow();
  cout << "nrbl="<<nrbl<<endl;
  vbs.nRow_p = nrbl;
  vbs.uvw_p.reference (uvw);
  vbs.rowFlag_p.resize (nrbl);
  vbs.rowFlag_p = False;
  vbs.flagCube_p.resize (4, 1, nrbl);
  vbs.flagCube_p = False;
  vbs.imagingWeight_p.resize (1, nrbl);
  vbs.imagingWeight_p = 1.;
  vbs.visCube_p.resize (1, 4, nrbl);
  indgen (vbs.visCube_p, Complex(0.1,0.2), Complex(0.02, 0.01));
  vbs.freq_p.resize (1);
  vbs.freq_p = 1e8;
  vbs.dopsf_p = False;
  vbs.useCorrected_p = True;
}

LofarCFStore fillCFStore1 (int oversampling, int support)
{
  Vector<Float> samp(2);
  samp[0] = samp[1] = oversampling;
  Vector<Int> xsup(2), ysup(2);
  xsup[0] = xsup[1] = ysup[0] = ysup[1] = support;
  Int maxXSup=100, maxYSup=100;     // not used
  Quantity pa;                      // not used
  CoordinateSystem cs;              // not used
  CountedPtr<CFTypeVec> dataPtr(new CFTypeVec);
  CFTypeVec& cfuncs = *dataPtr;
  cfuncs.resize (1);
  cfuncs[0].resize(4);
  int cfsz = (2*support + 1) * oversampling;
  Matrix<Complex> cfZero(cfsz, cfsz);
  for (int i=0; i<4; ++i) {
    cfuncs[0][i].resize(4);
    for (int j=0; j<4; ++j) {
      if (i == j) {
        Matrix<Complex> mat(cfsz, cfsz);
        indgen (mat, Complex(4*i+j+2, 4*i+j+3), Complex(0.1, 0.2));
        cfuncs[0][i][j].reference (mat);
      } else {
        cfuncs[0][i][j].reference (cfZero);
      }
    }
  }
  Matrix<Bool> muellerMask(4,4);
  muellerMask = False;
  muellerMask.diagonal() = True;
  return LofarCFStore(dataPtr, cs, samp, xsup, ysup, maxXSup, maxYSup,
                      pa, 0, muellerMask);
}

void sortBaseline (int nrant, const Vector<Int>& ant1, const Vector<Int>& ant2,
                   const LofarVBStore& vbs, Double wmax,
                   Vector<uInt>& blIndex,
                   vector<int>& blStart, vector<int>& blEnd)
{
  // Sort on baseline (use a baseline nr which is faster to sort).
  Vector<Int> blnr(nrant*ant1);
  blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
  GenSortIndirect<Int>::sort (blIndex, blnr);
  // Now determine nr of unique baselines and their start index.
  blStart.reserve (nrant*(nrant+1)/2);
  blEnd.reserve   (nrant*(nrant+1)/2);
  Int  lastbl     = -1;
  Int  lastIndex  = 0;
  bool usebl      = false;
  bool allFlagged = true;
  const Vector<Bool>& flagRow = vbs.rowFlag_p;
  for (uInt i=0; i<blnr.size(); ++i) {
    Int inx = blIndex[i];
    Int bl = blnr[inx];
    if (bl != lastbl) {
      // New baseline. Write the previous end index if applicable.
      if (usebl  &&  !allFlagged) {
	double Wmean = 0.5 * (vbs.uvw_p(2, blIndex[lastIndex]) +
                              vbs.uvw_p(2, blIndex[i-1]));
	if (abs(Wmean) <= wmax) {
	  blStart.push_back (lastIndex);
	  blEnd.push_back (i);
	}
      }
      // Skip auto-correlations and high W-values.
      // All w values are close, so if first w is too high, skip baseline.
      usebl = false;

      if (ant1[inx] != ant2[inx]) {
	usebl = true;
      }
      lastbl=bl;
      lastIndex=i;
    }
    // Test if the row is flagged.
    if (! flagRow[inx]) {
      allFlagged = false;
    }
  }
  // Write the last end index if applicable.
  if (usebl  &&  !allFlagged) {
    double Wmean = 0.5 * (vbs.uvw_p(2, blIndex[lastIndex]) +
                          vbs.uvw_p(2, blIndex[blnr.size()-1]));
    if (abs(Wmean) <= wmax) {
      blStart.push_back (lastIndex);
      blEnd.push_back (blnr.size());
    }
  }
}


int main()
{
  int nrant = 4;
  int nx = 80;
  int ny = 80;
  double imageSize = 1. / (180. / C::pi);
  double wmax = 10000;
  // Create the grids per thread.
  int nthread = OpenMP::maxThreads();
  vector<Array<Complex> > grids(nthread);
  vector<Matrix<Double> > sumWeights(nthread);
  for (int i=0; i<nthread; ++i) {
    grids[i].resize (IPosition(4,8,8,4,1));
    sumWeights[i].resize (8,8);
  }
  // Create fake data for uvw, visibilities, and convolution function.
  Vector<Int> ant1, ant2;
  Matrix<Double> uvw;
  fillBaselines (nrant, ant1, ant2, uvw);
  LofarVBStore vbs;
  fillVBStore (vbs, uvw);
  // Sort in order of baseline.
  Vector<uInt> blIndex;
  vector<int> blStart, blEnd;
  sortBaseline (nrant, ant1, ant2, vbs, wmax, blIndex, blStart, blEnd);
  // Set up the gridder.
  LofarVisResampler gridder;
  Vector<Int> chanMap(1, 0);
  Vector<Int> polMap(4);
  indgen (polMap);
  gridder.setMaps(chanMap, polMap);
  Vector<Double> uvScale(3);
  Vector<Double> uvOffset(3);
  uvScale.resize(3);
  uvScale[0] = imageSize;
  uvScale[1] = imageSize;
  uvScale[2] = imageSize;
  uvOffset.resize(3);
  uvOffset[0] = nx/2;
  uvOffset[1] = ny/2;
  uvOffset[2] = 0;
  Vector<Double> dphase(vbs.uvw_p.ncolumn(), 0.);
  gridder.setParams(uvScale,uvOffset,dphase);
  // Now do the gridding in order of baseline.
  Timer timer;
  for (int i=0; i<100; ++i) {
#pragma omp parallel 
  {
    // No thread-private variables.
    // The for loop can be parallellized. This must be done dynamically,
    // because the execution times of iterations can vary.
#pragma omp for schedule(dynamic)
    for (int i=0; i<int(blStart.size()); ++i) {
      int threadNum = OpenMP::threadNum();
      LofarCFStore cfs (fillCFStore1(20, 10));
      gridder.lofarDataToGrid (grids[threadNum], vbs, blIndex, blStart[i],
                               blEnd[i], sumWeights[threadNum], False, cfs);
    }
  }
  }
  timer.show("gridding");
  for (int i=1; i<nthread; ++i) {
    grids[0] += grids[i];
    sumWeights[0] += sumWeights[i];
  }
  cout << grids[0] << sumWeights[0];
  return 0;
}
