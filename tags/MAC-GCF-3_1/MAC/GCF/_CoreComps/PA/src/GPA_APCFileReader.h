//#  GPA_APC.h: helper class for loading XML files with APC into RAM
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

#ifndef GPA_APCFILEREADER_H
#define GPA_APCFILEREADER_H

#include <GPA_Defines.h>

#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMBuilder.hpp>
#include <Common/lofar_list.h>
#include <Common/lofar_iostream.h>

/**
  This is a helper class for loading XML files with APC into RAM
*/
class GCFPValue;

XERCES_CPP_NAMESPACE_USE

class GPAAPCFileReader
{
  public:
    GPAAPCFileReader();
    virtual ~GPAAPCFileReader();

    inline const list<TAPCProperty>& getProperties() const {return _properties;}
    TPAResult readFile(const string apcName, 
                      const string scope);

  private: //helper methods
    TPAResult makePropList(DOMNode* pN, 
                           string path);
    void getValue(DOMNode* pN, 
                  string& value);
    TPAResult createMACValueObject(const string& macType, 
                                   const string& valueData, 
                                   bool defaultSet,
                                   GCFPValue** pReturnValue);
    
  private:
    
    DOMBuilder*         _pXmlParser;
    list<TAPCProperty>  _properties;
    string              _scope;
    string              _apcRootPath;
};

// ---------------------------------------------------------------------------
//  Simple error handler deriviative to install on parser
// ---------------------------------------------------------------------------
class DOMCountErrorHandler : public DOMErrorHandler
{
  public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    DOMCountErrorHandler();
    ~DOMCountErrorHandler();


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    bool getSawErrors() const;


    // -----------------------------------------------------------------------
    //  Implementation of the DOM ErrorHandler interface
    // -----------------------------------------------------------------------
    bool handleError(const DOMError& domError);
    void resetErrors();


  private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DOMCountErrorHandler(const DOMCountErrorHandler&);
    void operator=(const DOMCountErrorHandler&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  _sawErrors
    //      This is set if we get any errors, and is queryable via a getter
    //      method. Its used by the main code to suppress output if there are
    //      errors.
    // -----------------------------------------------------------------------
    bool    _sawErrors;
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
    //  _localForm
    //      This is the local code page form of the string.
    // -----------------------------------------------------------------------
    char*   _localForm;
};

inline ostream& operator<<(ostream& target, const StrX& toDump)
{
  target << toDump.localForm();
  return target;
}

inline bool DOMCountErrorHandler::getSawErrors() const
{
  return _sawErrors;
}
#endif
