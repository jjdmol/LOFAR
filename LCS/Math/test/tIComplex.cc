//# tIComplex.h: Test program for IComplex classes
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

#include <Math/IComplex.h>
#include <Common/LofarLogger.h>

template<typename IC1, typename IC2>
void doIt()
{
  IC1 ic1 (1,2);
  IC2 ic2 (3,5);
  ASSERT (ic2.re() == 3);
  ASSERT (ic2.im() == 5);
  ASSERT (conj(ic2) == IC2(3,-5));
  ASSERT (norm(ic2) == 34);
  ASSERT (ic1+ic2 == IC2(4,7));
  ASSERT (ic1-ic2 == IC2(-2,-3));
  ASSERT (ic1*ic2 == IC2(-7,11));
  ASSERT (ic2/ic1 == IC2(13/5,-1/5));
  ASSERT (mulconj(ic1,ic2) == IC2(13,1));
  IC1 ic3 = ic1;
  ic3.mulconj(ic2);
  ASSERT (ic3 == ic1*conj(ic2));
  ASSERT (ic3 == IC2(13,1));
  IC1 ic4;
  ic4 = ic3;
  ic4/=IC2(ic1.re(),ic1.im());
  ASSERT (ic4 == IC2(15/5,-25/5));
  ASSERT (ic4*ic1 == ic3);
}

int main(int argc, const char* argv[])
{
  try {
    INIT_LOGGER("tIComplex.log_prop");
    doIt<IComplex8, IComplex8>();
    //    doIt<IComplex8, IComplex16>();
    //    doIt<IComplex8, IComplex32>();
    doIt<IComplex16,IComplex8>();
    doIt<IComplex16,IComplex16>();
    //    doIt<IComplex16,IComplex32>();
    doIt<IComplex32,IComplex8>();
    doIt<IComplex32,IComplex16>();
    doIt<IComplex32,IComplex32>();
  } catch (std::exception& x) {
    std::cout << "caught exception: " << x.what() << std::endl;
    return 1;
  }
  std::cout << "OK" << std::endl;
  return 0;
}
