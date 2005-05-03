//# MeqUVWPolc.cc: Class to calculate the UVW from a fitted polynomial
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <PSS3/MNS/MeqUVWPolc.h>
#include <Common/Debug.h>

#include <PSS3/MNS/MeqPointDFT.h>

using namespace casa;

namespace LOFAR {

MeqUVWPolc::MeqUVWPolc()
: itsPoly   (3),
  itsUCoeff ("u"),
  itsVCoeff ("v"),
  itsWCoeff ("w")
{
  itsFitter.setFunction (itsPoly);
}

void MeqUVWPolc::calcCoeff (const Vector<double>& times,
			    const Matrix<double>& uvws,
			    bool addPolc)
{
  int nr = times.nelements();
  Assert (uvws.shape()(0) == 3  &&  uvws.shape()(1) == nr);
  double step = times(1) - times(0);
  MeqDomain domain (times(0)-step/2, times(nr-1)+step/2, 0, 1e15);
  Vector<double> normTimes(nr);
  for (int i=0; i<nr; i++) {
    normTimes(i) = domain.normalizeX (times(i));
  }
  if (!addPolc) {
    vector<MeqPolc> emptyPolc;
    itsUCoeff.setPolcs (emptyPolc);
    itsVCoeff.setPolcs (emptyPolc);
    itsWCoeff.setPolcs (emptyPolc);
  }
  MeqPolc polc;
  polc.setDomain (domain);
  Vector<double> sigma(nr, 1);
  Vector<Double> sol = itsFitter.fit (normTimes, uvws.row(0), sigma);
  polc.setCoeff (Matrix<double>(sol));
  itsUCoeff.addPolc (polc);
  sigma = 1;
  sol = itsFitter.fit (normTimes, uvws.row(1), sigma);
  polc.setCoeff (Matrix<double>(sol));
  itsVCoeff.addPolc (polc);
  sigma = 1;
  sol = itsFitter.fit (normTimes, uvws.row(2), sigma);
  polc.setCoeff (Matrix<double>(sol));
  itsWCoeff.addPolc (polc);
}

void MeqUVWPolc::calcUVW (const MeqRequest& request)
{
  if (MeqPointDFT::doshow) {
    cout << "UCoeff: " << itsUCoeff.getPolcs()[0].getCoeff() << endl;
    cout << "VCoeff: " << itsVCoeff.getPolcs()[0].getCoeff() << endl;
    cout << "WCoeff: " << itsWCoeff.getPolcs()[0].getCoeff() << endl;
  }
  itsU = itsUCoeff.getResult (request);
  itsV = itsVCoeff.getResult (request);
  itsW = itsWCoeff.getResult (request);
}

void MeqUVWPolc::setName(const string& name)
{
  itsUCoeff.setName("u" + name);
  itsVCoeff.setName("v" + name);
  itsWCoeff.setName("w" + name);
}

}
