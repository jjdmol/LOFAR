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

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace LOFAR
{
  using namespace boost::property_tree;


  //## ---------------- Local methods ---------------- ##//

  void doWriteParset(ostream &os, const ptree &pt, const string &key="")
  {
    string value = pt.get_value<string>();
    if(!value.empty()) {
      // If value matches "#", the we've got an empty leaf, remove its value.
      os << key << "=" << (value == "#" ? "" : value) << endl;
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
    ptree pt;
    for(ParameterSet::const_iterator it = ps.begin(); it != ps.end(); ++it) {
      if(it->second.isVector()) {
//        cout << it->first << " is a vector" << endl;
      }
      else if(it->second.isRecord()) {
//        cout << it->first << " is a record" << endl;
      }
      else {
//        cout << it->first << " is a value" << endl;
      }
      // To discriminate between a leaf with an empty value and a node (which,
      // by definition, has an empty value), we assign "#" to an empty leaf.
      pt.put(it->first, (it->second.get().empty() ? "#" : it->second.get()));
    }
    // Debug
    for(ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
      cout << it->first << " -> " << it->second.data() << endl;
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
    json_parser::read_json(is, itsPT);
  }


  void PropertyTree::readJSON(const string &fn)
  {
    json_parser::read_json(fn, itsPT);
  }


  void PropertyTree::writeJSON(ostream &os, bool pretty) const
  {
    json_parser::write_json(os, itsPT, pretty);
  }


  void PropertyTree::writeJSON(const string &fn, bool pretty) const
  {
    json_parser::write_json(fn, itsPT, std::locale(), pretty);
  }


  void PropertyTree::readXML(istream &is)
  {
    xml_parser::read_xml(is, itsPT);
  }


  void PropertyTree::readXML(const string &fn)
  {
    xml_parser::read_xml(fn, itsPT);
  }


  void PropertyTree::writeXML(ostream &os, bool pretty) const
  {
    if(pretty) {
      xml_parser::write_xml(os, itsPT, xml_writer_make_settings(' ', 2));
    } else {
      xml_parser::write_xml(os, itsPT);
    }
  }


  void PropertyTree::writeXML(const string &fn, bool pretty) const
  {
    if(pretty) {
      xml_parser::write_xml(fn, itsPT, std::locale(), 
                            xml_writer_make_settings(' ', 2));
    } else {
      xml_parser::write_xml(fn, itsPT);
    }
  }

} // namespace LOFAR

