//#  GPA_APCLoader.h: 
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

#ifndef GPA_APCLOADER_H
#define GPA_APCLOADER_H
#include <iostream.h>
#include    <xercesc/sax2/DefaultHandler.hpp>
#include    <xercesc/framework/XMLFormatter.hpp>

XERCES_CPP_NAMESPACE_USE

class GPAAPCLoader : public DefaultHandler, private XMLFormatTarget
{
  public:
    // -----------------------------------------------------------------------
    //  Constructors
    // -----------------------------------------------------------------------
    GPAAPCLoader
    (
      const char* const               encodingName, 
      const XMLFormatter::UnRepFlags  unRepFlags, 
      const bool                      expandNamespaces
    );
    ~GPAAPCLoader();


    // -----------------------------------------------------------------------
    //  Implementations of the format target interface
    // -----------------------------------------------------------------------
    void writeChars
    (
      const   XMLByte* const  toWrite
    );

    void writeChars
    (
      const XMLByte* const  toWrite, 
      const unsigned int    count, 
      XMLFormatter* const   formatter
    );


    // -----------------------------------------------------------------------
    //  Implementations of the SAX DocumentHandler interface
    // -----------------------------------------------------------------------
    void endDocument();

    void endElement
    ( 
      const XMLCh* const uri,
			const XMLCh* const localname,
			const XMLCh* const qname
    );

    void characters
    (
      const XMLCh* const chars, 
      const unsigned int length
    );

    void ignorableWhitespace
    (
      const XMLCh* const  chars, 
      const unsigned int  length
    );

    void processingInstruction
    (
      const XMLCh* const  target, 
      const XMLCh* const  data
    );

    void startDocument();

    void startElement
    (	
      const   XMLCh* const  uri,
		  const   XMLCh* const  localname,
			const   XMLCh* const  qname,
			const   Attributes&		attributes
    );



    // -----------------------------------------------------------------------
    //  Implementations of the SAX ErrorHandler interface
    // -----------------------------------------------------------------------
    void warning(const SAXParseException& exception);
    void error(const SAXParseException& exception);
    void fatalError(const SAXParseException& exception);



    // -----------------------------------------------------------------------
    //  Implementation of the SAX DTDHandler interface
    // -----------------------------------------------------------------------
    void notationDecl
    (
      const XMLCh* const    name, 
      const XMLCh* const    publicId, 
      const XMLCh* const    systemId
    );

    void unparsedEntityDecl
    (
        const XMLCh* const    name, 
        const XMLCh* const    publicId, 
        const XMLCh* const    systemId, 
        const XMLCh* const    notationName
    );

  private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  _formatter
    //      This is the formatter object that is used to output the data
    //      to the target. It is set up to format to the standard output
    //      stream.
    // -----------------------------------------------------------------------
    XMLFormatter _formatter;
    bool			   _expandNS;
};

// ---------------------------------------------------------------------------
//  This is a simple class that lets us do easy (though not terribly efficient)
//  trancoding of XMLCh data to local code page for display.
// ---------------------------------------------------------------------------
class StrX
{
  public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    StrX(const XMLCh* const toTranscode)
    {
      // Call the private transcoding method
      _localForm = XMLString::transcode(toTranscode);
    }

    ~StrX()
    {
      XMLString::release(&_localForm);
    }

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const char* localForm() const
    {
      return _localForm;
    }

  private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fLocalForm
    //      This is the local code page form of the string.
    // -----------------------------------------------------------------------
    char*   _localForm;
};

inline ostream& operator<<(ostream& target, const StrX& toDump)
{
    target << toDump.localForm();
    return target;
}

#endif
