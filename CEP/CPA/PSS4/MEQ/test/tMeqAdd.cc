//# tMeqAdd.cc: test program for class MEQ::Add
//#
//# Copyright (C) 2003
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


#include <MEQ/Add.h>
#include <MEQ/Cos.h>
#include <MEQ/Request.h>
#include <MEQ/Result.h>
#include <MEQ/VellsTmp.h>
#include <MEQ/ParmPolcStored.h>
#include <MEQ/ParmTable.h>
#include <Common/Debug.h>
#include <exception>

using namespace MEQ;


bool compare(const Vells& m1, const Vells& m2)
{
  if (m1.nx() != m2.nx()  ||  m1.ny() != m2.ny()) {
    return false;
  }
  Vells res = sum(sqr(m1-m2));
  if (res.isReal()) {
    return (res.realStorage()[0] < 1.e-7);
  }
  complex<double> resc = res.complexStorage()[0];
  return (resc.real() < 1.e-7  &&  resc.imag() < 1.e-7);
}

int main()
{
  try {
    ParmTable ptab("meqadd.MEP");
    ParmPolcStored p1("p1", &ptab);
    ParmPolcStored p2("p2", &ptab);
    Cos ncos(&p1);
    Add add(&ncos, &p2);
    Domain domain(1,4, -2,3);
    Request req(Cells(domain, 4, 4));
    Result res;
    Result::Ref refres(res, DMI::WRITE||DMI::EXTERNAL);;
    int flag = add.getResult (refres, req);
    cout << flag << endl;
    cout << res.getValue() << endl;
  } catch (std::exception& x) {
    cout << "Caught exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
