//#  GPA_APC.cc: 
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
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

#include "GPA_APC.h"
#include "GPA_APCLoader.h"
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/XMLString.hpp>

static const char*              encodingName    = "LATIN1";
static XMLFormatter::UnRepFlags unRepFlags      = XMLFormatter::UnRep_CharRef;
static bool                  expandNamespaces= false ;
static bool                     doSchema        = true;
static bool                     schemaFullChecking = false;
static bool                     namespacePrefixes = false;
  
GPAAPC::GPAAPC()// :
  //_xmlHandler(encodingName, unRepFlags, expandNamespaces)
{
    try
    {
         XMLPlatformUtils::Initialize();
    }

    catch (const XMLException& toCatch)
    {
         cerr << "Error during initialization! :\n"
              << StrX(toCatch.getMessage()) << endl;
         exit(1);
    }

  _pXmlParser = XMLReaderFactory::createXMLReader();

  //
  // Set some features in the XMLReader
  //
  _pXmlParser->setFeature(XMLUni::fgXercesDynamic, true);
  _pXmlParser->setFeature(XMLUni::fgSAX2CoreValidation, true);   // optional
  _pXmlParser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);   // optional
  _pXmlParser->setFeature(XMLUni::fgXercesSchema, doSchema);
  _pXmlParser->setFeature(XMLUni::fgXercesSchemaFullChecking, schemaFullChecking);
  _pXmlParser->setFeature(XMLUni::fgSAX2CoreNameSpacePrefixes, namespacePrefixes);

}

GPAAPC::~GPAAPC()
{
  //
  // We can do away with the parser!
  //
  delete _pXmlParser;
  _pXmlParser = 0;
  XMLPlatformUtils::Terminate();
}

TPAResult GPAAPC::loadFile(const string& apcName)
{
  TPAResult result(PA_NO_ERROR);
  
  GPAAPCLoader xmlHandler(encodingName, unRepFlags, expandNamespaces);
  //
  // Tell the XMLReader who's the contenthandler...
  //
  _pXmlParser->setContentHandler( &xmlHandler );
  _pXmlParser->setErrorHandler( &xmlHandler );

  try
  {
    _pXmlParser->parse(apcName.c_str());
  }
  catch( const XMLException& e )
  {
    LOFAR_LOG_ERROR(PA_STDOUT_LOGGER, ( 
        "XMLException: %s\n",
        XMLString::transcode(e.getMessage())));
    result = PA_UNABLE_TO_LOAD_APC;  
  }
  catch( const SAXParseException& e )
  {
    LOFAR_LOG_ERROR(PA_STDOUT_LOGGER, ( 
        "SAXParseException: %s\n",
        XMLString::transcode(e.getMessage())));
    result = PA_UNABLE_TO_LOAD_APC;  
  }
/*  catch( const ExceptionObject& e )
  {
    LOFAR_LOG_ERROR(PA_STDOUT_LOGGER, ( 
        "%s/%s in %s @line %d, fn= %s\n",
        e.getType(), e.getText(), 
        e.getFile(), e.getLine(),
        e.getFunction()));
    result = PA_UNABLE_TO_LOAD_APC;  
  }
*/  catch (...)
  {
    LOFAR_LOG_ERROR(PA_STDOUT_LOGGER, ( 
        "Unexpected Exception\n"));
    result = PA_UNABLE_TO_LOAD_APC;  
  }
  
  return result;
}
