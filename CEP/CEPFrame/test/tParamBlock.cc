// tParamBlock.cc: Program to test classes ParamBlock and ParamValue
//
//  Copyright (C) 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//
//////////////////////////////////////////////////////////////////////


#include "CEPFrame/ParamBlock.h"
#include <Common/lofar_iostream.h>

using namespace LOFAR;

void doIt()
{
  ParamBlock par;
  par["b1"] = true;
  par["i1"] = 10;
  par["f1"] = float(20);
  par["d1"] = double(30);
  par["c1"] = complex<float>(40,50);
  par["dc1"] = complex<double>(60,70);
  par["s1"] = "abc";
  par["s2"] = std::string("defg");
  cout << "ParamBlock par:" << endl;
  par.show (cout);
  ParamBlock par1(par);
  cout << "ParamBlock par1:" << endl;
  par1.show(cout);
  par["b2"] = false;
  par1 = par;
  cout << "ParamBlock par1:" << endl;
  par1.show (cout);
  par1["vb1"] = par1["b1"].getVecBool();
  par1["vi1"] = par1["i1"].getVecInt();
  par1["vf1"] = par1["f1"].getVecFloat();
  par1["vd1"] = par1["d1"].getVecDouble();
  par1["vc1"] = par1["c1"].getVecComplex();
  par1["vdc1"] = par1["dc1"].getVecDComplex();
  par1["vs1"] = par1["s1"].getVecString();

  cout << "ParamBlock par3:" << endl;
  ParamBlock par3(par1);
  par3.show (cout);
  cout << par3["b1"].getBool() << ' ' << par3["b1"].getVecBool() << endl;
  cout << par3["i1"].getInt() << ' ' << par3["i1"].getVecInt() << endl;
  cout << par3["i1"].getFloat() << ' ' << par3["i1"].getVecFloat() << endl;
  cout << par3["i1"].getDouble() << ' ' << par3["i1"].getVecDouble() << endl;
  cout << par3["i1"].getComplex() << ' ' << par3["i1"].getVecComplex() << endl;
  cout << par3["i1"].getDComplex() << ' ' << par3["i1"].getVecDComplex() << endl;
  cout << par3["f1"].getFloat() << ' ' << par3["f1"].getVecFloat() << endl;
  cout << par3["f1"].getDouble() << ' ' << par3["f1"].getVecDouble() << endl;
  cout << par3["f1"].getComplex() << ' ' << par3["f1"].getVecComplex() << endl;
  cout << par3["f1"].getDComplex() << ' ' << par3["f1"].getVecDComplex() << endl;
  cout << par3["d1"].getDouble() << ' ' << par3["d1"].getVecDouble() << endl;
  cout << par3["d1"].getDComplex() << ' ' << par3["d1"].getVecDComplex() << endl;
  cout << par3["c1"].getComplex() << ' ' << par3["c1"].getVecComplex() << endl;
  cout << par3["c1"].getDComplex() << ' ' << par3["c1"].getVecDComplex() << endl;
  cout << par3["dc1"].getDComplex() << ' ' << par3["dc1"].getVecDComplex() << endl;
  cout << par3["s1"].getString() << ' ' << par3["s1"].getVecString() << endl;
  cout << par3["vb1"].getVecBool() << endl;
  cout << par3["vi1"].getVecInt() << endl;
  cout << par3["vi1"].getVecFloat() << endl;
  cout << par3["vi1"].getVecDouble() << endl;
  cout << par3["vi1"].getVecComplex() << endl;
  cout << par3["vi1"].getVecDComplex() << endl;
  cout << par3["vf1"].getVecFloat() << endl;
  cout << par3["vf1"].getVecDouble() << endl;
  cout << par3["vf1"].getVecComplex() << endl;
  cout << par3["vf1"].getVecDComplex() << endl;
  cout << par3["vd1"].getVecDouble() << endl;
  cout << par3["vd1"].getVecDComplex() << endl;
  cout << par3["vc1"].getVecComplex() << endl;
  cout << par3["vc1"].getVecDComplex() << endl;
  cout << par3["vdc1"].getVecDComplex() << endl;
  cout << par3["vs1"].getVecString() << endl;
  cout << par3["b1"].dataType() << ' ' << par3["b1"].isVector() << ' '
       << par3["b1"].size() << endl;
  cout << par3["i1"].dataType() << ' ' << par3["i1"].isVector() << ' '
       << par3["i1"].size() << endl;
  cout << par3["f1"].dataType() << ' ' << par3["f1"].isVector() << ' '
       << par3["f1"].size() << endl;
  cout << par3["d1"].dataType() << ' ' << par3["d1"].isVector() << ' '
       << par3["d1"].size() << endl;
  cout << par3["c1"].dataType() << ' ' << par3["c1"].isVector() << ' '
       << par3["c1"].size() << endl;
  cout << par3["dc1"].dataType() << ' ' << par3["dc1"].isVector() << ' '
       << par3["dc1"].size() << endl;
  cout << par3["s1"].dataType() << ' ' << par3["s1"].isVector() << ' '
       << par3["s1"].size() << endl;
  cout << par3["vb1"].dataType() << ' ' << par3["vb1"].isVector() << ' '
       << par3["vb1"].size() << endl;
  cout << par3["vi1"].dataType() << ' ' << par3["vi1"].isVector() << ' '
       << par3["vi1"].size() << endl;
  cout << par3["vf1"].dataType() << ' ' << par3["vf1"].isVector() << ' '
       << par3["vf1"].size() << endl;
  cout << par3["vd1"].dataType() << ' ' << par3["vd1"].isVector() << ' '
       << par3["vd1"].size() << endl;
  cout << par3["vc1"].dataType() << ' ' << par3["vc1"].isVector() << ' '
       << par3["vc1"].size() << endl;
  cout << par3["vdc1"].dataType() << ' ' << par3["vdc1"].isVector() << ' '
       << par3["vdc1"].size() << endl;
  cout << par3["vs1"].dataType() << ' ' << par3["vs1"].isVector() << ' '
       << par3["vs1"].size() << endl;

  ParamBlock par2;
  par1 = par2;
  cout << "Empty ParamBlock par1:" << endl;
  par1.show (cout);
  par1["blk1"] = par;
  par1["blk2"] = par2;
  vector<ParamValue> vec;
  vec.push_back (true);
  vec.push_back (100);
  vec.push_back (float(100.01));
  vec.push_back (double(100.02));
  vec.push_back (complex<float> (3.5, 6.2));
  vec.push_back (complex<double> (1.123456789, -3));
  vec.push_back ("a");
  vec.push_back ("");
  vec.push_back ("abc");
  par1["vec"] = vec;
  par2 = par1;
  cout << "ParamBlock par2:" << endl;
  par2.show (cout);
  cout << par2["blk1"].dataType() << ' ' << par2["blk1"].isVector() << ' '
       << par2["blk1"].size() << endl;
  cout << par2["vec"].dataType() << ' ' << par2["vec"].isVector() << ' '
       << par2["vec"].size() << endl;
}

int main()
{
  try {
    doIt();
  } catch (...) {
    cout << "Unexpected exception" << endl;
    exit(1);
  }
  exit(0);
}
