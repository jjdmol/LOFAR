//# PropertyTree.h: Implements a nested tree of values index by key.
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

#ifndef LOFAR_COMMON_PROPERTYTREE_H
#define LOFAR_COMMON_PROPERTYTREE_H

// \file
// Implements a nested tree of values index by key.

#include <Common/lofar_iosfwd.h>
#include <Common/lofar_string.h>
#include <boost/property_tree/ptree.hpp>

namespace LOFAR {

  //# Forward declarations
  class Parameterset;

  // \addtogroup Common
  // @{

  // The PropertyTree class is a data structure that stores a nested tree of
  // values, index by a key. Internally, it uses the Boost.PropertyTree to
  // store the data. Data can be read from and written to file or stream in a
  // number of formats, including the LOFAR ParameterSet, JSON, and XML.
  class PropertyTree
  {
  public:
    // @name I/O functions for ParameterSet.
    // Bla bla bla.
    // @{

    // Read ParameterSet from input stream @a is or file @a fn, and store the
    // key/value pairs in our property tree. If @a append it @c false, then
    // the property tree will be cleared first, otherwise the key/value pairs
    // will be appended.
    void readParset(istream &is); //, bool append=false);
    void readParset(const string &fn); //, bool append=false);

    // Write our property tree to a ParameterSet output stream @a os, or file
    // @a fn. Key/value pairs with empty values will not be written. The
    // reason for this is that it is impossible to distinguish an empty leaf
    // from a node in the property tree.
    // @{
    void writeParset(ostream &os) const;
    void writeParset(const string &fn) const;
    // @}

    // @name I/O functions for JSON.
    // @{
    void readJSON(istream &is); //, bool append=false);
    void readJSON(const string &fn); //, bool append=false);
    void writeJSON(ostream &os, bool pretty=true) const;
    void writeJSON(const string &fn, bool pretty=true) const;
    // @}

    // @name I/O functions for XML.
    // @{
    void readXML(istream &is); //, bool append=false);
    void readXML(const string &fn); //, bool append=false);
    void writeXML(ostream &os, bool pretty=true) const;
    void writeXML(const string &fn, bool pretty=true) const;
    // @}

  private:
    // Data is stored in a Boost Property-tree.
    boost::property_tree::ptree itsPT;

  };
  // @}

} // namespace LOFAR

#endif
