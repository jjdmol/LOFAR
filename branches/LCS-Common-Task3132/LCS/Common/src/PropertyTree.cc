//# PropertyTree.cc: Implements a nested tree of values index by key.
//#
//# Copyright (C) 2012
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/PropertyTree.h>
#include <Common/ParameterSet.h>
#include <Common/lofar_fstream.h>
#include <Common/lofar_sstream.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
  using namespace boost::property_tree;


  //## ---------------- Local methods ---------------- ##//

  void doWriteParset(ostream &os, const ptree &pt, const string &key="")
  {
    string value = pt.get_value<string>();
    if(!value.empty()) {
      os << key << "=" << value << endl;
    }
    for(ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
      doWriteParset(os, it->second,
                    key + (key.empty() ? "" : ".") + it->first);
    }
  }


  //## ---------------- Public methods ---------------- ##//

  void PropertyTree::readParset(istream &is)
  {
#if 0
    // Once ParameterSet provides a public readStream method, we can avoid
    // the detour of using stringstreams and a string buffer.
    ParameterSet ps;
    ps.readStream(is);
#else
    ostringstream oss;
    string line;
    while(getline(is, line)) oss << line << endl;
    ParameterSet ps;
    ps.adoptBuffer(oss.str());
#endif
    boost::property_tree::ptree pt;
    for(ParameterSet::const_iterator it = ps.begin(); it != ps.end(); ++it) {
      pt.put(it->first, it->second);
    }
    swap(pt, itsPT);
  }


  void PropertyTree::readParset(const string &fn)
  {
    ifstream ifs(fn.c_str());
    readParset(ifs);
  }


  void PropertyTree::writeParset(ostream &os) const
  {
    doWriteParset(os, itsPT);
  }


  void PropertyTree::writeParset(const string &fn) const
  {
    ofstream ofs(fn.c_str());
    writeParset(ofs);
  }


  void PropertyTree::readJSON(istream &is)
  {
  }


  void PropertyTree::readJSON(const string &fn)
  {
  }


  void PropertyTree::writeJSON(ostream &os, bool pretty) const
  {
  }


  void PropertyTree::writeJSON(const string &fn, bool pretty) const
  {
  }


  void PropertyTree::readXML(istream &is)
  {
  }


  void PropertyTree::readXML(const string &fn)
  {
  }


  void PropertyTree::writeXML(ostream &os, bool pretty) const
  {
  }


  void PropertyTree::writeXML(const string &fn, bool pretty) const
  {
  }

} // namespace LOFAR

