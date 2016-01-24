//# XMLDoc.h: Represents an XML document.
//#
//# Copyright (C) 2015
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

#ifndef LOFAR_MESSAGEBUS_XMLDOC_H
#define LOFAR_MESSAGEBUS_XMLDOC_H

#ifdef HAVE_LIBXMLXX
#include <libxml++/parsers/domparser.h>
#endif

#include <string>
#include <ostream>

namespace LOFAR {

/*
 * Represnt an XML document
 */
class XMLDoc
{
public:
  XMLDoc(const std::string &);

#ifdef HAVE_LIBXMLXX
  // Allow copying even though we have non-copyable mmbers
  XMLDoc(const XMLDoc &);
#endif

  // Extract a subset from another XML document
  XMLDoc(const XMLDoc &other, const std::string &key);

  ~XMLDoc();

  // Return the full document. Note that the content string
  // is possibly generated, and may not be an exact copy
  // of what was provided in the constructor.
  std::string getContent() const;

  // Return a value, given an XPath key.
  std::string getXMLvalue(const std::string& key) const;

  // Set a value in the XML content for a given XPath key.
  void setXMLvalue(const std::string& key, const std::string& data);

  // Import an XML subdocument under the given XPath key.
  void insertXML(const std::string& key, const std::string& xml);

protected:
#ifdef HAVE_LIBXMLXX
  // Locates and returns a node given by its XPATH key ("/a/b/c")
  xmlpp::Element *getXMLnode(const std::string &name) const;
#endif

  // -- datamembers -- 
#ifdef HAVE_LIBXMLXX
  // itsParser is the owner of the XML Document and Elements that
  // will be accessed. It takes care of the memory management and
  // thus free all elements at destruction.
  //
  // If itsParser == 0, we did not use a parser, and we own itsDocument.
  xmlpp::DomParser *itsParser;   // NOTE: non-copyable
  xmlpp::Document *itsDocument; // NOTE: non-copyable
#else
  std::string itsContent;
#endif
};

inline std::ostream &operator<<(std::ostream &os, const XMLDoc &xml)
{
  os << xml.getContent();
  return os;
}


} // namespace LOFAR

#endif

