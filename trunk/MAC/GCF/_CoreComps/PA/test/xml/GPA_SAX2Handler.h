/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 1999-2000 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Xerces" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache\@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation, and was
 * originally based on software copyright (c) 1999, International
 * Business Machines, Inc., http://www.ibm.com .  For more information
 * on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/*
 * $Log$
 * Revision 1.5  2002/11/05 21:46:20  tng
 * Explicit code using namespace in application.
 *
 * Revision 1.4  2002/02/01 22:38:52  peiyongz
 * sane_include
 *
 * Revision 1.3  2001/08/02 17:10:29  tng
 * Allow DOMCount/SAXCount/IDOMCount/SAX2Count to take a file that has a list of xml file as input.
 *
 * Revision 1.2  2000/08/09 22:40:15  jpolast
 * updates for changes to sax2 core functionality.
 *
 * Revision 1.1  2000/08/08 17:17:21  jpolast
 * initial checkin of SAX2Count
 *
 *
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/internal/VecAttributesImpl.hpp>
#include <Common/lofar_vector.h>
#include <Common/lofar_stack.h>
#include <Common/lofar_string.h>

XERCES_CPP_NAMESPACE_USE

class GPASAX2Handler : public DefaultHandler
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    GPASAX2Handler();
    ~GPASAX2Handler();

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
  VecAttributesImpl* copyAttributes(const Attributes &att);

  bool                      itsActiveFlag;
  string                    itsFocusSimul;  // for partial parsing
  bool                      itsRootSimulWasSet;
  const XMLCh*              itsContainerType;
  stack<VecAttributesImpl*> itsAttStack;
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
