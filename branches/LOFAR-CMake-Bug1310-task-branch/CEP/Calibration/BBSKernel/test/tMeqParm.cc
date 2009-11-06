//# tMeqParm.cc: test program for class MeqParm
//#
//# Copyright (C) 2005
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
#include <BBSKernel/MNS/MeqPolc.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <BBSKernel/MNS/MeqResult.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <Common/Timer.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace casa;


bool compare (const MeqMatrix& m1, const MeqMatrix& m2)
{
  if (m1.nx() != m2.nx()  ||  m1.ny() != m2.ny()) {
    return false;
  }
  MeqMatrix res = sum(sqr(m1-m2));
  if (!res.isComplex()) {
    return (res.getDouble() < 1.e-7);
  }
  dcomplex resc = res.getDComplex();
  return (LOFAR::real(resc) < 1.e-7  &&  LOFAR::imag(resc) < 1.e-7);
}

void doEval (MeqParmFunklet& parm, const MeqDomain& domain, int nx, int ny)
{
  MeqRequest req(domain, nx, ny);
  MeqResult res = parm.getResult (req);
  cout << "domain: " << domain << ' ' << nx << ' ' << ny
       << "   npolc=" << parm.getFunklets().size() << endl;
  cout << "result: " << res.getValue() << endl;
}

void doIt (MeqParmFunklet& parm)
{
  // Evaluate the parameter polynomials for some requests.
  MeqDomain domain(2,6, 1,5);
  doEval (parm, domain, 1, 1);
  doEval (parm, domain, 2, 2);
  doEval (parm, domain, 8, 1);
  doEval (parm, domain, 1, 8);

  for (int i=0; i<4; ++i) {
    MeqDomain domain2(2,6, 1+i, 2+i);
    doEval (parm, domain2, 1, 1);
    doEval (parm, domain2, 2, 2);
    doEval (parm, domain2, 8, 1);
    doEval (parm, domain2, 1, 8);
  }

  MeqDomain domain3(0,4, 2,3);
  doEval (parm, domain3, 1, 1);
  doEval (parm, domain3, 2, 2);
  doEval (parm, domain3, 8, 1);
  doEval (parm, domain3, 1, 8);
}

int main()
{
  try {
    {
      cout << "\nTest1 with two constant parms" << endl;
      MeqParmFunklet parm("parm", 0);
      MeqPolc polc;
      polc.setDomain (MeqDomain(2,6, 1,3));
      polc.setCoeff (MeqMatrix(2.));
      parm.add (polc); 
      polc.setDomain (MeqDomain(2,6, 3,5));
      polc.setCoeff (MeqMatrix(3.));
      parm.add (polc); 
      doIt (parm);
    }
    {
      cout << "\nTest2 with two polynomial parms" << endl;
      MeqParmFunklet parm("parm", 0);
      vector<MeqPolc> polcs;
      MeqPolc polc;
      polc.setDomain (MeqDomain(2,6, 1,3));
      polc.setCoeff (MeqMatrix(2.,2,1,true));
      parm.add (polc); 
      polc.setDomain (MeqDomain(2,6, 3,5));
      polc.setCoeff (MeqMatrix(2.,1,2,true));
      parm.add (polc); 
      doIt (parm);
    }
  } catch (std::exception& x) {
    cerr << "Caught exception: " << x.what() << endl;
    return 1;
  }

  cout << "OK" << endl;
  return 0;
}
