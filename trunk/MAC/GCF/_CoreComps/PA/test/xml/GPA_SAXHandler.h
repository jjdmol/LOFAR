//#  GPA_SAXHandler.h: 
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

#ifndef GPA_SAXHANDLER_H
#define GPA_SAXHANDLER_H

#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

//class VecAttributesImpl;

// This class provides the proper overloaded methods for handling the PCInt
// XML configuration files. It implements the SAX ContentHandler interface
// and the SAX ErrorHandler interface.
class SAXHandler : public DefaultHandler 
{
public:
  // -----------------------------------------------------------------------
  //  Constructors
  // -----------------------------------------------------------------------
  SAXHandler();

  virtual ~SAXHandler();


  // -----------------------------------------------------------------------
  //  Implementations of the SAX DocumentHandler interface
  // -----------------------------------------------------------------------
  virtual void endDocument();

  virtual void endElement (const XMLCh* const uri,
        const XMLCh* const localname,
        const XMLCh* const qname);

  virtual void startDocument();

  virtual void startElement (const XMLCh* const uri,
          const XMLCh* const localname,
          const XMLCh* const qname,
          const Attributes& attributes);

  // -----------------------------------------------------------------------
  //  Implementations of the SAX ErrorHandler interface
  // -----------------------------------------------------------------------

  virtual void warning (const SAXParseException &exception);

  virtual void error (const SAXParseException &exception);

  virtual void fatalError (const SAXParseException &exception);

  virtual void resetErrors();
  
private:
  
  void handleRoot (const Attributes& attributes, bool start);
  
  void handleSimul (const Attributes& attributes, bool start);
  
  void handleStep (const Attributes& attributes, bool start);
    
  void handleWorkHolder (const Attributes& attributes, bool start);
    
  void handleDataHolder (const Attributes& attributes, bool start);
    
  void handleConnect (const Attributes& attributes, bool start);
    
  void handleExStep (const Attributes& attributes, bool start);

  // -----------------------------------------------------------------------
  // Function to make a copy of an Attributes. Use this function until
  // Apache XML parser for C++ implements some clone() function in the
  // abstract Attributes class, or it implements some other convenient
  // method for copying an Attributes object
  // -----------------------------------------------------------------------
//  VecAttributesImpl* copyAttributes(const Attributes &att);

  bool                      itsActiveFlag;
  string                    itsFocusSimul;  // for partial parsing
  bool                      itsRootSimulWasSet;
  const XMLCh*              itsContainerType;
//  stack<VecAttributesImpl*> itsAttStack;
  vector<string>            itsInDHVector;
  vector<string>            itsOutDHVector;
};


// ---------------------------------------------------------------------------
// This is a simple class that lets us do easy (though not terribly efficient)
// transcoding of XMLCh data to local code page for display.
// ---------------------------------------------------------------------------
class SAXLocalStr
{
public:
  
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  SAXLocalStr (const XMLCh* const toTranscode)
    { if (toTranscode) {
        char* flocal = XMLString::transcode(toTranscode);
  itsLocalForm = flocal;
 delete [] flocal;
      }
    }
  
  const string& localForm() const
    { return itsLocalForm; }
  
private:
  string itsLocalForm;
};

inline ostream& operator<<(ostream& target, const SAXLocalStr& toDump)
{
  target << toDump.localForm();
  return target;
}
#endif
