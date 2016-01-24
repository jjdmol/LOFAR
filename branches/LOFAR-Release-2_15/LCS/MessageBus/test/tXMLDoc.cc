//#  Copyright (C) 2015
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
#include <Common/LofarLogger.h>
#include <UnitTest++.h>
#include <MessageBus/XMLDoc.h>
#include <MessageBus/Exceptions.h>

using namespace LOFAR;

SUITE(Parsing) {
  TEST(ValidXML) {
    XMLDoc doc("<foo></foo>");
  }

#ifdef HAVE_LIBXMLXX
  TEST(EmptyXML) {
    CHECK_THROW(XMLDoc(""), XMLException);
  }

  TEST(InvalidXML) {
    CHECK_THROW(XMLDoc("<foo>"), XMLException);
  }
#endif

  TEST(Subdocument) {
    XMLDoc base("<foo><bar>x</bar></foo>");
    XMLDoc derived(base, "foo/bar");

    CHECK_EQUAL("x", derived.getXMLvalue("bar"));
  }
}

TEST(getXMLvalue) {
  XMLDoc doc("<foo><bar>x</bar></foo>");

  CHECK_EQUAL("x",          doc.getXMLvalue("foo/bar"));
  CHECK_THROW(doc.getXMLvalue("invalid"), XMLException);
}

TEST(setXMLvalue) {
  XMLDoc doc("<foo><bar>x</bar></foo>");

  doc.setXMLvalue("foo/bar", "y");

  CHECK_EQUAL("y",          doc.getXMLvalue("foo/bar"));
  CHECK_THROW(doc.setXMLvalue("invalid", ""), XMLException);
}

TEST(insertXML) {
  XMLDoc doc("<foo><bar>x</bar></foo>");

  doc.insertXML("foo/bar", "<baz>y</baz>");
  CHECK_EQUAL("y",          doc.getXMLvalue("foo/bar/baz"));

  CHECK_THROW(doc.insertXML("invalid", "<baz>y</baz>"), XMLException);
#ifdef HAVE_LIBXMLXX
  CHECK_THROW(doc.insertXML("foo/bar", "<invalid>"),    XMLException);
#endif
}

TEST(getContent) {
  XMLDoc doc("<foo><bar>x</bar></foo>");

  string docstr = doc.getContent();

  XMLDoc reparsed(docstr);

  CHECK_EQUAL(docstr, reparsed.getContent());
  CHECK_EQUAL("x",    reparsed.getXMLvalue("foo/bar"));
}

int main() {
  INIT_LOGGER("tXMLDoc");

  return UnitTest::RunAllTests() > 0;
}
