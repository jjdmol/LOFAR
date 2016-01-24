//#  MessageContent.cc: one_line_description
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <MessageBus/Exceptions.h>
#include <MessageBus/XMLDoc.h>

#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

#ifdef HAVE_LIBXMLXX
#include <sstream>
#include <libxml++/parsers/domparser.h>
#include <libxml++/nodes/textnode.h>

using namespace xmlpp;
#endif

using namespace std;
using namespace LOFAR::StringUtil;

namespace LOFAR {

XMLDoc::XMLDoc(const std::string &content)
{
#ifdef HAVE_LIBXMLXX
  itsParser = new DomParser;

  try {
    itsParser->parse_memory(content);
    itsDocument = itsParser->get_document();
  } catch(xmlpp::exception &e) {
    delete itsParser;

    THROW(XMLException, "Could not parse XML: " << e.what());
  }
#else
  itsContent = content;
#endif
}

#ifdef HAVE_LIBXMLXX
XMLDoc::XMLDoc(const XMLDoc &other)
{
  itsParser = new DomParser;

  try {
    itsParser->parse_memory(other.getContent());
    itsDocument = itsParser->get_document();
  } catch(xmlpp::exception &e) {
    delete itsParser;

    THROW(XMLException, "Could not parse XML: " << e.what());
  }
}
#endif

XMLDoc::XMLDoc(const XMLDoc &other, const std::string &key)
{
#ifdef HAVE_LIBXMLXX
  itsParser = 0; 
  itsDocument = 0;

  try {
    itsDocument = new Document;

    itsDocument->create_root_node_by_import(other.getXMLnode(key));
  } catch(xmlpp::exception &e) {
    delete itsDocument;

    THROW(XMLException, "Could not parse XML: " << e.what());
  }
#else
  const string content = other.getXMLvalue(key);

  // Extract root element (last element in 'key')
  const vector<string> labels = split(key, '/');
  ASSERT(!labels.empty());
  const string root_element = labels[labels.size()-1];

  itsContent = formatString("<%s>%s</%s>", root_element.c_str(), other.getXMLvalue(key).c_str(), root_element.c_str());
#endif
}

XMLDoc::~XMLDoc()
{
#ifdef HAVE_LIBXMLXX
  if (itsParser != NULL) {
    // We used a parser, it owns the document
    delete itsParser;
  } else {
    // We own the document
    delete itsDocument;
  }
#endif
}

std::string XMLDoc::getContent() const
{
#ifdef HAVE_LIBXMLXX
  return itsDocument->write_to_string_formatted();
#else
  return itsContent;
#endif
}

string XMLDoc::getXMLvalue(const string& key) const
{
#ifdef HAVE_LIBXMLXX
  Element *e = getXMLnode(key);

  // Extract the text, if any
  TextNode *t = e->get_child_text();
  if (!t) return "";

  return t->get_content();
#else
  // get copy of content
  vector<string>  labels = split(key, '/');

  // loop over subkeys
  string::size_type  offset = 0;
  string::size_type  begin = string::npos;
  string::size_type  end = string::npos;
  string        startTag;
  for (size_t i = 0; i <  labels.size(); ++i) {
    // define tags to find
    startTag = string("<"+labels[i]+">");
    // search begin tag
    begin  = itsContent.find(startTag, offset);
    if (begin == string::npos) {
      THROW(XMLException, "XML element not found (could not find begin tag): " << key);
    }
    offset = begin;
  }
  // search end tag
  string stopTag ("</"+labels[labels.size()-1]+">");
  begin+=startTag.size();
  end = itsContent.find(stopTag, begin);
  if (end == string::npos) {
    THROW(XMLException, "XML element not found (could not find end tag): " << key);
  }
  return (itsContent.substr(begin, end - begin));
#endif
}

void XMLDoc::setXMLvalue(const string& key, const string &data)
{
#ifdef HAVE_LIBXMLXX
  Element *e = getXMLnode(key);

  e->set_child_text(data);
#else
  // get copy of content
  vector<string>  labels = split(key, '/');

  // loop over subkeys
  string::size_type  offset = 0;
  string::size_type  begin = string::npos;
  string::size_type  end = string::npos;
  string        startTag;
  for (size_t i = 0; i <  labels.size(); ++i) {
    // define tags to find
    startTag = string("<"+labels[i]+">");
    // search begin tag
    begin  = itsContent.find(startTag, offset);
    if (begin == string::npos) {
      THROW(XMLException, "XML element not found (could not find begin tag): " << key);
    }
    offset = begin;
  }
  // search end tag
  string stopTag ("</"+labels[labels.size()-1]+">");
  begin+=startTag.size();
  end = itsContent.find(stopTag, begin);
  if (end == string::npos) {
    THROW(XMLException, "XML element not found (could not find end tag): " << key);
  }

  itsContent.replace(begin, end - begin, data);
#endif
}

void XMLDoc::insertXML(const string &key, const string &xml)
{
#ifdef HAVE_LIBXMLXX
  // Find insert spot
  Element *e = getXMLnode(key);

  try {
    // Parse provided XML
    DomParser parser;
    parser.parse_memory(xml);

    Document *document = parser.get_document();
    ASSERT(document);

    Element *root = document->get_root_node();
    ASSERT(root);

    // Insert the XML into our document
    e->import_node(root);
  } catch(xmlpp::exception &e) {
    THROW(XMLException, "Could not parse XML: " << e.what());
  }
#else
  setXMLvalue(key, xml);
#endif
}

#ifdef HAVE_LIBXMLXX
Element *XMLDoc::getXMLnode(const string &name) const
{
  Element *root = itsDocument->get_root_node();
  ASSERT(root);

  // assume key is an XPath relative to root, see http://www.w3schools.com/xpath/xpath_syntax.asp
  NodeSet nodeset = root->find("/" + name);
  if (nodeset.empty()) {
    // Element not found
    THROW(XMLException, "XML element not found: /" << name);
  }

  Element *e = dynamic_cast<Element*>(nodeset[0]);

  if (!e) {
    // Key points to a special element
    THROW(XMLException, "XML element not a text element: /" << name);
  }

  return e;
}
#endif


} // namespace LOFAR
