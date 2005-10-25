//# MeqHist.cc: A class holding a histogram.
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

#include <MNS/MeqHist.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/ArrayMath.h>

using namespace casa;

void MeqHist::update (unsigned int bucket)
{
  if (bucket >= itsVec.nelements()) {
    unsigned int nrold = itsVec.nelements();
    itsVec.resize (bucket+1, true);
    for (unsigned int i=nrold; i<=bucket; i++) {
      itsVec(i) = 0;
    }
  }
  itsVec(bucket)++;
}

Vector<int> MeqHist::merge (const vector<MeqHist>& hists)
{
  // Find the maximum nr of elements in all histograms.
  unsigned int nr = 0;
  for (vector<MeqHist>::const_iterator iter=hists.begin();
       iter!=hists.end();
       iter++) {
    if (iter->get().nelements() > nr) {
      nr = iter->get().nelements();
    }
  }
  Vector<int> result(nr, 0);
  for (vector<MeqHist>::const_iterator iter=hists.begin();
       iter!=hists.end();
       iter++) {
    const Vector<int>& vec = iter->get();
    Vector<int> tmp = result(Slice(0,vec.nelements()));
    tmp += vec;
  }
  return result;
}
