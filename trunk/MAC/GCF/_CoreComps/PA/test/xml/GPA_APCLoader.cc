//#  GPA_APCLoader.cc: 
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


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include "GPA_APCLoader.h"


// ---------------------------------------------------------------------------
//  Local const data
//
//  Note: This is the 'safe' way to do these strings. If you compiler supports
//        L"" style strings, and portability is not a concern, you can use
//        those types constants directly.
// ---------------------------------------------------------------------------
static const XMLCh  gEndElement[] = { chOpenAngle, chForwardSlash, chNull };
static const XMLCh  gEndPI[] = { chQuestion, chCloseAngle, chNull };
static const XMLCh  gStartPI[] = { chOpenAngle, chQuestion, chNull };
static const XMLCh  gXMLDecl1[] =
{
  chOpenAngle, chQuestion, chLatin_x, chLatin_m, chLatin_l,   
  chSpace, chLatin_v, chLatin_e, chLatin_r, chLatin_s, chLatin_i,
  chLatin_o, chLatin_n, chEqual, chDoubleQuote, chDigit_1, chPeriod,   
  chDigit_0, chDoubleQuote, chSpace, chLatin_e, chLatin_n, chLatin_c,   
  chLatin_o, chLatin_d, chLatin_i, chLatin_n, chLatin_g, chEqual,   
  chDoubleQuote, chNull
};

static const XMLCh  gXMLDecl2[] =
{
  chDoubleQuote, chQuestion, chCloseAngle,   
  chLF, chNull
};


// ---------------------------------------------------------------------------
//  GPAAPCLoader: Constructors and Destructor
// ---------------------------------------------------------------------------
GPAAPCLoader::GPAAPCLoader( 
  const char* const encodingName, 
  const XMLFormatter::UnRepFlags unRepFlags, 
  const bool expandNamespaces) :
  _formatter(encodingName, 0, this, XMLFormatter::NoEscapes, unRepFlags),
	_expandNS(expandNamespaces)
{
}

GPAAPCLoader::~GPAAPCLoader()
{
}


// ---------------------------------------------------------------------------
//  GPAAPCLoader: Overrides of the output formatter target interface
// ---------------------------------------------------------------------------
void GPAAPCLoader::writeChars(const XMLByte* const toWrite)
{
}

void GPAAPCLoader::writeChars(const XMLByte* const toWrite,
                               const unsigned int count,
                               XMLFormatter* const formatter)
{
  // For this one, just dump them to the standard output
  // Surprisingly, Solaris was the only platform on which
  // required the char* cast to print out the string correctly.
  // Without the cast, it was printing the pointer value in hex.
  // Quite annoying, considering every other platform printed
  // the string with the explicit cast to char* below.
  cout.write((char *) toWrite, (int) count);
	cout.flush();
}


// ---------------------------------------------------------------------------
//  GPAAPCLoader: Overrides of the SAX ErrorHandler interface
// ---------------------------------------------------------------------------
void GPAAPCLoader::error(const SAXParseException& e)
{
  cerr << "\nError at file " << StrX(e.getSystemId())
   << ", line " << e.getLineNumber()
   << ", char " << e.getColumnNumber()
   << "\n  Message: " << StrX(e.getMessage()) << endl;
}

void GPAAPCLoader::fatalError(const SAXParseException& e)
{
  cerr << "\nFatal Error at file " << StrX(e.getSystemId())
   << ", line " << e.getLineNumber()
   << ", char " << e.getColumnNumber()
   << "\n  Message: " << StrX(e.getMessage()) << endl;
}

void GPAAPCLoader::warning(const SAXParseException& e)
{
  cerr << "\nWarning at file " << StrX(e.getSystemId())
   << ", line " << e.getLineNumber()
   << ", char " << e.getColumnNumber()
   << "\n  Message: " << StrX(e.getMessage()) << endl;
}


// ---------------------------------------------------------------------------
//  GPAAPCLoader: Overrides of the SAX DTDHandler interface
// ---------------------------------------------------------------------------
void GPAAPCLoader::unparsedEntityDecl(
  const XMLCh* const name, 
  const XMLCh* const publicId, 
  const XMLCh* const systemId, 
  const XMLCh* const notationName)
{
  // Not used at this time
}


void GPAAPCLoader::notationDecl(
  const XMLCh* const name, 
  const XMLCh* const publicId, 
  const XMLCh* const systemId)
{
    // Not used at this time
}


// ---------------------------------------------------------------------------
//  GPAAPCLoader: Overrides of the SAX DocumentHandler interface
// ---------------------------------------------------------------------------
void GPAAPCLoader::characters(const     XMLCh* const    chars
                                  , const   unsigned int    length)
{
  _formatter.formatBuf(chars, length, XMLFormatter::CharEscapes);
}


void GPAAPCLoader::endDocument()
{
}


void GPAAPCLoader::endElement(
  const XMLCh* const uri,
  const XMLCh* const localname,
  const XMLCh* const qname)
{
  // No escapes are legal here
  _formatter << XMLFormatter::NoEscapes << gEndElement ;
  if ( _expandNS )
  {
    if (XMLString::compareIString(uri,XMLUni::fgZeroLenString) != 0)
      _formatter  << uri << chColon;
    _formatter << localname << chCloseAngle;
  }
  else
    _formatter << qname << chCloseAngle;
}


void GPAAPCLoader::ignorableWhitespace( 
  const   XMLCh* const chars,
  const  unsigned int length)
{
  _formatter.formatBuf(chars, length, XMLFormatter::NoEscapes);
}


void GPAAPCLoader::processingInstruction(
  const XMLCh* const target, 
  const XMLCh* const data)
{
  _formatter << XMLFormatter::NoEscapes << gStartPI  << target;
  if (data)
    _formatter << chSpace << data;
  _formatter << XMLFormatter::NoEscapes << gEndPI;
}


void GPAAPCLoader::startDocument()
{
}


void GPAAPCLoader::startElement(
  const XMLCh* const  uri,
  const XMLCh* const  localname,
  const XMLCh* const  qname,
  const Attributes&		attributes)
{
  // The name has to be representable without any escapes
  _formatter  << XMLFormatter::NoEscapes << chOpenAngle ;
  if ( _expandNS )
	{
    if (XMLString::compareIString(uri,XMLUni::fgZeroLenString) != 0)
      _formatter  << uri << chColon;
    _formatter << localname ;
  }
  else
    _formatter << qname ;

  unsigned int len = attributes.getLength();
  for (unsigned int index = 0; index < len; index++)
  {
    //
    //  Again the name has to be completely representable. But the
    //  attribute can have refs and requires the attribute style
    //  escaping.
    //
    _formatter  << XMLFormatter::NoEscapes << chSpace ;
    if ( _expandNS )
    {
      if (XMLString::compareIString(attributes.getURI(index),XMLUni::fgZeroLenString) != 0)
		    _formatter  << attributes.getURI(index) << chColon;
      _formatter  << attributes.getLocalName(index) ;
    }
    else
      _formatter  << attributes.getQName(index) ;

    _formatter  << chEqual << chDoubleQuote
                << XMLFormatter::AttrEscapes
                << attributes.getValue(index)
                << XMLFormatter::NoEscapes
                << chDoubleQuote;
  }
  _formatter << chCloseAngle;
}
