//  SAXParser.cc:
//
//  Copyright (C) 2000, 2001
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

#include "CEPFrame/SAXSimulParser.h"
#include "Common/Debug.h"

namespace LOFAR
{

SAXSimulParser::SAXSimulParser (const string& fileName)
: file   (0),
  parser (0),
  handler(0)
{
  init (fileName.c_str(), 0);
}

SAXSimulParser::SAXSimulParser (const string& fileName,
				const string& rootSimul)
: file   (0),
  parser (0),
  handler(0)
{
  init (fileName.c_str(), rootSimul.c_str());
}

#ifdef HAVE_XERCES
void SAXSimulParser::init (const char* fileName, const char* rootSimul)
{
  if (strlen(fileName) == 0) {
    return;
  }
  file = XMLString::transcode (fileName);
  // Initialize the XML4C2 system
  try {
    XMLPlatformUtils::Initialize();
  } catch (const XMLException& toCatch) {
    cerr << "Error during initialization! :\n"
	 << SAXLocalStr(toCatch.getMessage()) << endl;
    return;
  }
  //  Create a SAX parser object. 
  parser = XMLReaderFactory::createXMLReader();
  ////  parser->setFeature (XMLString::transcode
  ////                            ("http://xml.org/sax/features/validation"),
  ////		      false);
  //  Create the handler object and install it as the document and error
  //  handler for the parser. Then parse the file and catch any exceptions
  //  that propogate out
  try {
    if (rootSimul) {
      handler = new SAXHandler(rootSimul);
    } else {
      handler = new SAXHandler();
    }
    parser->setContentHandler(handler);
    parser->setErrorHandler(handler);
  } catch (const XMLException& toCatch) {
    cerr << "\nAn error occured\n  Error: "
	 << SAXLocalStr(toCatch.getMessage())
	 << "\n" << endl;
    XMLPlatformUtils::Terminate();
    return;
  }
}
#else
void SAXSimulParser::init (const char* /*fileName*/, const char* /*rootSimul*/)
{
  AssertMsg (0, "SAX is not configured in");
}
#endif

SAXSimulParser::~SAXSimulParser()
{
#ifdef HAVE_XERCES
  // Deleting the parser must be done prior to calling Terminate.
  delete parser;
  delete [] file;
  // And call the termination method
  XMLPlatformUtils::Terminate();
  delete handler;
#endif
}

Simul SAXSimulParser::parseSimul()
{
#ifdef HAVE_XERCES
  if (!parser) return Simul();
  try {
    parser->parse(file);
  } catch (const XMLException& toCatch) {
    cerr << "\nAn error occured\n  Error: "
	 << SAXLocalStr(toCatch.getMessage())
	 << "\n" << endl;
    return Simul();
  }
  return handler->getRoot();
#else
  return Simul();
#endif
}

}
