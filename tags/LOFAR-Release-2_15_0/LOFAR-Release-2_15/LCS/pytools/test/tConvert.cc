//# tConvert.cc: Test program for C++-python converters
//# Copyright (C) 2011
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <lofar_config.h>
#include <pytools/PycExcp.h>
#include <pytools/PycBasicData.h>
#include <boost/python.hpp>
#include <Common/LofarTypes.h>
#include <Common/StreamUtil.h>

using namespace boost::python;
using namespace std;

namespace LOFAR { namespace pytools {

  struct TConvert
  {
    TConvert() {}
    bool testbool (bool in)
      {cout << "bool " << in << endl; return in;}
    int testint (int in)
      {cout << "Int " << in << endl; return in;}
    int64 testint64 (int64 in)
      {cout << "Int64 " << in << endl; return in;}
    int testssize (::ssize_t in)
      {cout << "ssize " << in << endl; return in;}
    double testfloat (double in)
      {cout << "Float " << in << endl; return in;}
    dcomplex testcomplex (const dcomplex& in)
      {cout << "DComplex " << in << endl; return in;}
    string teststring (const string& in)
      {cout << "String " << in << endl; string out=in; return out;}
    vector<int> testvecint (const vector<int>& in)
      {cout << "VecInt " << in << endl; return in;}
    vector<dcomplex> testveccomplex (const vector<dcomplex>& in)
      {cout << "VecComplex " << in << endl; return in;}
    vector<string> testvecstr (const vector<string>& in)
      {cout << "VecStr " << in << endl; return in;}
  };


  void testConvert()
  {
    class_<TConvert> ("tConvert", init<>())
      .def ("testbool",       &TConvert::testbool)
      .def ("testint",        &TConvert::testint)
      .def ("testint64",      &TConvert::testint64)
      .def ("testssize",      &TConvert::testssize)
      .def ("testfloat",      &TConvert::testfloat)
      .def ("testcomplex",    &TConvert::testcomplex)
      .def ("teststring",     &TConvert::teststring)
      .def ("testvecint",     &TConvert::testvecint)
      .def ("testveccomplex", &TConvert::testveccomplex)
      .def ("testvecstr",     &TConvert::testvecstr)
      ;
  }

}}


BOOST_PYTHON_MODULE(_tConvert)
{
  LOFAR::pytools::register_convert_excp();
  LOFAR::pytools::register_convert_basicdata();
  LOFAR::pytools::register_convert_std_vector<std::string>();

  LOFAR::pytools::testConvert();
}
