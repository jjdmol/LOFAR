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
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMText.hpp>

#include <SAL/GCF_PVBool.h>
#include <SAL/GCF_PVDouble.h>
#include <SAL/GCF_PVUnsigned.h>
#include <SAL/GCF_PVInteger.h>
#include <SAL/GCF_PVChar.h>
#include <SAL/GCF_PVString.h>

static void Trim_White_Spaces(string & string_to_trim)
{
  // Trim leading white spaces    
  size_t nowhite = string_to_trim.find_first_not_of( " \t\n\r\b\f\v" );
  string_to_trim.erase( 0, nowhite );
  // Trim trailing white spaces
  nowhite = string_to_trim.find_last_not_of( " \t\n\r\b\f\v" );
  string_to_trim.erase( nowhite + 1 );
}
  
GPAAPC::GPAAPC()
{
  AbstractDOMParser::ValSchemes valScheme = AbstractDOMParser::Val_Auto;
  bool                       doNamespaces       = false;
  bool                       doSchema           = false;
  bool                       schemaFullChecking = false;
  
  try
  {
    XMLPlatformUtils::Initialize();
  }
  catch (const XMLException& toCatch)
  {
    LOFAR_LOG_ERROR(PA_STDOUT_LOGGER, ( 
      "\n\tError during initialization! : '%s'\n\t",
      StrX(toCatch.getMessage()).localForm()));
  }

  // Instantiate the DOM parser.
  static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
  DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);
  _pXmlParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);

  _pXmlParser->setFeature(XMLUni::fgDOMNamespaces, doNamespaces);
  _pXmlParser->setFeature(XMLUni::fgXercesSchema, doSchema);
  _pXmlParser->setFeature(XMLUni::fgXercesSchemaFullChecking, schemaFullChecking);

  if (valScheme == AbstractDOMParser::Val_Auto)
  {
    _pXmlParser->setFeature(XMLUni::fgDOMValidateIfSchema, true);
  }
  else if (valScheme == AbstractDOMParser::Val_Never)
  {
    _pXmlParser->setFeature(XMLUni::fgDOMValidation, false);
  }
  else if (valScheme == AbstractDOMParser::Val_Always)
  {
    _pXmlParser->setFeature(XMLUni::fgDOMValidation, true);
  }

  // enable datatype normalization - default is off
  _pXmlParser->setFeature(XMLUni::fgDOMDatatypeNormalization, true);
}

GPAAPC::~GPAAPC()
{
  //
  // We can do away with the parser!
  //
  _pXmlParser->release();
  XMLPlatformUtils::Terminate();
}

TPAResult GPAAPC::load(const string apcName, const string scope)
{
  TPAResult result(PA_NO_ERROR);

  // And create our error handler and install it
  DOMCountErrorHandler errorHandler;
  _pXmlParser->setErrorHandler(&errorHandler);

  _scope = scope;
  for (list<TAPCProperty>::iterator iter = _properties.begin();
       iter != _properties.end(); ++iter)
  {
    if (iter->pValue)
      delete iter->pValue;
  }
  _properties.clear();
  Trim_White_Spaces(_scope);
  
  if (_scope.length() == 0)
    return PA_EMPTY_SCOPE;
    
  XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *doc = 0;

  try
  {
    // reset document pool
    _pXmlParser->resetDocumentPool();

    doc = _pXmlParser->parseURI(apcName.c_str());
  }
  catch (const XMLException& toCatch)
  {
    LOFAR_LOG_ERROR(PA_STDOUT_LOGGER, ( 
        "\n\tError during parsing: '%s'\n\tException message is:  \n\t%s\n\t",
        apcName.c_str(),
        StrX(toCatch.getMessage()).localForm()));
    result = PA_UNABLE_TO_LOAD_APC;  
  }
  catch (const DOMException& toCatch)
  {
    const unsigned int maxChars = 2047;
    XMLCh errText[maxChars + 1];

    if (DOMImplementation::loadDOMExceptionMsg(toCatch.code, errText, maxChars))
    {
      LOFAR_LOG_ERROR(PA_STDOUT_LOGGER, ( 
        "\n\tDOM Error during parsing: '%s'\n\tDOMException code is:  %s\n\t",
        "Message is: %s\n\t",
        apcName.c_str(),
        toCatch.code,
        StrX(errText).localForm()));
    }
    else
    {
      LOFAR_LOG_ERROR(PA_STDOUT_LOGGER, ( 
        "\n\tDOM Error during parsing: '%s'\n\tDOMException code is:  %s\n\t",
        apcName.c_str(),
        toCatch.code));
    }
    result = PA_UNABLE_TO_LOAD_APC;  
  }
  catch (...)
  {
    LOFAR_LOG_ERROR(PA_STDOUT_LOGGER, ( 
      "\n\tUnexpected exception during parsing: '%s'\n\t",
      apcName.c_str()));
    result = PA_UNABLE_TO_LOAD_APC;  
  }

  //
  //  Extract the DOM tree, get the list of all the elements and report the
  //  length as the count of elements.
  //
  if (!errorHandler.getSawErrors())
  {
    if (doc) 
    {
      DOMNode* pChild;
      DOMNode* pN = (DOMNode*)doc->getDocumentElement();
      if (pN)
      {
        // ignore root node -> type of APC
        for (pChild = pN->getFirstChild(); pChild != 0; 
             pChild = pChild->getNextSibling())
          makePropList(pChild, "");
      }     
    }
  }
  
  return result;
}

TPAResult GPAAPC::makePropList(DOMNode* pN, string path)
{
  TPAResult result(PA_NO_ERROR);
  DOMNode* pChild;
  static bool propertyEntered = false;
  static bool macDefaultSet = false;
  static string macType = "";
  static string macDefault = "";
  static string propName = "";
  if (pN) 
  {
    if (pN->getNodeType() == DOMNode::ELEMENT_NODE)
    {
      char* name = XMLString::transcode(pN->getNodeName());
      if (strncmp(name, "MACType", 7) == 0)
      {
        result = getValue(pN, macType);
        propertyEntered = true;
        propName = path;
        XMLString::release(&name);
        return result;
      }
      else if (strncmp(name, "MACDefault", 10) == 0)
      {
        result = getValue(pN, macDefault);
        propertyEntered = true;
        macDefaultSet = true;
        propName = path;
        XMLString::release(&name);
        return result;
      }
      else
      {
        if (path.length() > 0)
          path += '_';
        path += name;
        XMLString::release(&name);
      }      
    }
    if (path != propName && propertyEntered)
    {
      Trim_White_Spaces(macType);
      if (macType.length() > 0)
      {
        TAPCProperty property;
        if (propName.length() > 0)
        {
          property.name = _scope + string("_") + propName;
        }
        else
        {
          property.name = _scope;
        }
        GCFPValue* pV(0);
        if ((result = createMACValueObject(macType, 
                                           macDefault, 
                                           macDefaultSet, 
                                           &pV)) 
                                           == PA_NO_ERROR)
        {
          property.pValue = pV;
          property.defaultSet = macDefaultSet;
          _properties.push_back(property);
        }
        propertyEntered = false;
        macDefaultSet = false;
      }
      else
      {
        LOFAR_LOG_ERROR(PA_STDOUT_LOGGER, (
          "No MACType specified on property: '%s'",
          propName.c_str()));
        result = PA_NO_TYPE_SPECIFIED_IN_APC;
      }
    }
    for (pChild = pN->getFirstChild(); pChild != 0; 
         pChild = pChild->getNextSibling())
      makePropList(pChild, path);
  }
  return result;
}

TPAResult GPAAPC::getValue(DOMNode* pN, string& value)
{
  TPAResult result(PA_NO_ERROR);  
  if (pN)
  {
    if (pN->getChildNodes()->getLength() == 1)
    {
      DOMNode* pChild;
      pChild = pN->getFirstChild();
      if (pChild->getNodeType() == DOMNode::TEXT_NODE) 
      {
        char* pValue = XMLString::transcode(pChild->getNodeValue());
        value = pValue;
        XMLString::release(&pValue);     
      }
    }
  }
  return result;  
}

TPAResult GPAAPC::createMACValueObject(
  const string& macType, 
  const string& valueData, 
  bool defaultSet,
  GCFPValue** pReturnValue)
{
  TPAResult result(PA_NO_ERROR);
  *pReturnValue = 0;
  
  if (macType == "MACBool")
  {
    GCFPVBool* pValue = new GCFPVBool();
    *pReturnValue = pValue;
    if (!defaultSet) return result;
    if (valueData.length() > 0)
    {
      char* validPos(0);
      long int value = strtol(valueData.c_str(), &validPos, 10);
      if (*validPos == '\0')
      {
        pValue->setValue(value != 0);
      }
      else if (validPos == valueData.c_str())
      {
        if ((strncasecmp(valueData.c_str(), "false", 5) == 0) || 
            (strncasecmp(valueData.c_str(), "no", 2) == 0) ||
            (strncasecmp(valueData.c_str(), "off", 3) == 0))
        {
          pValue->setValue(false);          
        }
        else 
        if ((strncasecmp(valueData.c_str(), "true", 5) == 0) || 
            (strncasecmp(valueData.c_str(), "yes", 2) == 0) ||
            (strncasecmp(valueData.c_str(), "on", 3) == 0))
        {
          pValue->setValue(true);
        }
      }
    }
  }
  else if (macType == "MACChar")
  {
    GCFPVChar* pValue = new GCFPVChar();
    *pReturnValue = pValue;
    if (!defaultSet)  return result;
    if (valueData.length() == 1)
    {
      pValue->setValue(valueData[0]);
    }
  }
  else if (macType == "MACUnsigned")
  {
    GCFPVUnsigned* pValue = new GCFPVUnsigned();
    *pReturnValue = pValue;
    if (!defaultSet)  return result;
    if (valueData.length() == 1)
    {
      char* validPos(0);
      long int value = strtol(valueData.c_str(), &validPos, 10);
      if (*validPos == '\0')
      {
        if (value >= 0)
          pValue->setValue(value);
      }
    }
  }
  else if (macType == "MACInteger")
  {
    GCFPVInteger* pValue = new GCFPVInteger();
    *pReturnValue = pValue;
    if (!defaultSet)  return result;
    if (valueData.length() == 1)
    {
      char* validPos(0);
      long int value = strtol(valueData.c_str(), &validPos, 10);
      if (*validPos == '\0')
      {
        pValue->setValue(value);
      }
    }
  }
  else if (macType == "MACDouble")
  {
    GCFPVDouble* pValue = new GCFPVDouble();
    *pReturnValue = pValue;
    if (!defaultSet)  return result;    
    if (valueData.length() > 0)
    {
      char* validPos(0);
      double value = strtod(valueData.c_str(), &validPos);
      if (*validPos == '\0')
      {
        pValue->setValue(value);
      }
    }
  }
  else if (macType == "MACString")
  {
    GCFPVString* pValue = new GCFPVString();
    *pReturnValue = pValue;
    if (!defaultSet)  return result;    
    if (valueData.length() > 0)
    {
      pValue->setValue(valueData);
    }
  }
/*  else if (macType == "BIT32_VAL")
  {
    *pMacValue = new GCFPVBit32(((Bit32Var *)&variable)->getValue());
  }
  else if (macType == "REF_VAL")
  {
    *pMacValue = new GCFPVRef(((TextVar *)&variable)->getValue());
  }
  else if (macType == "BLOB_VAL")
  {
    *pMacValue = new GCFPVBlob(((BlobVar *)&variable)->getValue());
  }
  else if (macType == "DATETIME_VAL")
  {
    *pMacValue = new GCFPVDateTime(((TimeVar *)&variable)->getValue());
  }*/
  else 
  {
    result = PA_MACTYPE_UNKNOWN;
  }
  return result;
}


DOMCountErrorHandler::DOMCountErrorHandler() :
  _sawErrors(false)
{
}

DOMCountErrorHandler::~DOMCountErrorHandler()
{
}

// ---------------------------------------------------------------------------
//  DOMCountHandlers: Overrides of the DOM ErrorHandler interface
// ---------------------------------------------------------------------------
bool DOMCountErrorHandler::handleError(const DOMError& domError)
{
  _sawErrors = true;
  if (domError.getSeverity() == DOMError::DOM_SEVERITY_WARNING)
  {
    LOFAR_LOG_WARN(PA_STDOUT_LOGGER, ( 
      "\nWarning at file %s, line %s, char %s\n Message: %s\n\t",
      StrX(domError.getLocation()->getURI()).localForm(),
      domError.getLocation()->getLineNumber(),
      domError.getLocation()->getColumnNumber(),
      StrX(domError.getMessage()).localForm()));
  }
  else if (domError.getSeverity() == DOMError::DOM_SEVERITY_ERROR)
  {
    LOFAR_LOG_ERROR(PA_STDOUT_LOGGER, ( 
      "\nError at file %s, line %s, char %s\n Message: %s\n\t",
      StrX(domError.getLocation()->getURI()).localForm(),
      domError.getLocation()->getLineNumber(),
      domError.getLocation()->getColumnNumber(),
      StrX(domError.getMessage()).localForm()));
  }
  else
  {
    LOFAR_LOG_FATAL(PA_STDOUT_LOGGER, ( 
      "\nFatal Error at file %s, line %s, char %s\n Message: %s\n\t",
      StrX(domError.getLocation()->getURI()).localForm(),
      domError.getLocation()->getLineNumber(),
      domError.getLocation()->getColumnNumber(),
      StrX(domError.getMessage()).localForm()));
  }
  return true;
}

void DOMCountErrorHandler::resetErrors()
{
  _sawErrors = false;
}
