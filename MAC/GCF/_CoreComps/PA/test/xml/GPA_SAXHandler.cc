//
//# $Id$
//
//# $Log$
//# Revision 1.6  2002/09/17 09:26:16  verkout
//# HV: Couple of parsing issues -
//# 1) if the parser encountered an element that it recognizes before AND
//#    after it was done parsing the rootelement, it would *replace* the
//#    element tree to the new element. This is now fixed. Once an element
//#    tree is successfully built, recognized elements are discarded.
//# 2) The default behaviour was that the rootelement of the tree was the
//#    first element the parser encountered and recognized. This might not
//#    necessarily be the rootelement you were hoping for :).
//#    It is now possible to specify which element you want for root-element.
//#    If you do not specify a root-element, the default behaviour applies
//#    (i.e. root-element=first recognized element)
//#
//# Revision 1.5  2002/07/16 10:42:58  loose
//# Added "SAXHandler: " in output of error handling methods.
//#
//# Revision 1.4  2002/07/15 12:01:22  verkout
//# HV: Undo (i.e. remove) printing of an obsolete error message.
//#
//# Revision 1.3  2002/07/10 10:46:08  verkout
//# HV: Merged my local addition into the new version
//#
//
#include <saxutil/SAXHandler.h>
#include <saxutil/SAXString.h>
#include <myutil/ExceptionObject.h>
#include <xercesc/sax2/Attributes.hpp>
#include <iostream>

using std::cerr;
using std::endl;
using std::string;
using std::pair;


//#----------------------------------------------------------------------------
//#                Constructors & destructors
//#----------------------------------------------------------------------------

SAXHandler::SAXHandler() : 
  itsErrorFlag(false),
  doneRootElement(false)
{
}


SAXHandler::SAXHandler( const creatormap_t& aCreatorMap, const string& rootelemname ) :
  itsErrorFlag(false),
  doneRootElement(false),
  itsRootElementName( rootelemname ),
  itsCreatorMap(aCreatorMap)
{
}


//#----------------------------------------------------------------------------
//#        Implementation of the SAX ContentHandler interface
//#----------------------------------------------------------------------------

void SAXHandler::endDocument()
{
}


void SAXHandler::endElement(const XMLCh *const uri,
			    const XMLCh *const localname,
			    const XMLCh *const qname)
{
  // Check if the element stack is not empty
  if (!itsElementStack.empty())
  {
    // Pop the current element of the stack.
    CountedPointer<SAXElement> ptr(itsElementStack.top());
	
    itsElementStack.pop();
    // If the stack is not empty, add the popped element to its parent,
    // which is now on top of the stack.
    if (!itsElementStack.empty())
    {
      // This should never fail, unless the programmer forgot to add the
      // necessary code in the overloaded addElement() method.
      if (!itsElementStack.top()->addElement(*ptr))
      {
	string err = "Don't know how to add " + ptr->type() + " to " 
	  + itsElementStack.top()->type();

	throw EXCEPTION(err.c_str());
      }
    }
    else
    {
      // apparently, the stack has become empty, which
      // basically means that we've seen the end of
      // our root element! So let's stop parsing!
      doneRootElement = true;
    }
  }
}


//
// If we start a new document, presumable we
// haven't done the root element yet, have we?  :)
//
void SAXHandler::startDocument()
{
  doneRootElement = false;
}


void SAXHandler::startElement(const XMLCh *const uri,
			      const XMLCh *const localname,
			      const XMLCh *const qname,
			      const Attributes &attrs)
{
  // if doneRootElement==true, we have done our stuff and there's
  // really no need in actually wasting more CPU cycles.
  // so let's bail out before we do anything else!
  if( doneRootElement )
  {
    return;
  }

  // if the stack.size()==0 we haven't started the rootelement yet.
  // we can save time by checking if the name of this element
  // matches the rootelementname (if specified). If it doesn't,
  // we can bail out immediately!
  SAXString        elemname( localname );
	
  if( itsElementStack.size()==0 && 
      itsRootElementName.size()!=0 &&
      itsRootElementName!=elemname )
  {
    //cout << "Start element: skipping non-root-element " << elemname << endl;
    return;
  }

  // See if we know what to do with this element.
  creatormap_t::iterator it = itsCreatorMap.find(elemname);
  
  // Hmm, obviously we don't
  if (it == itsCreatorMap.end())
  {
    // If element stack is not empty, we *should* know what to do with
    // this element. The programmer probably forgot to add it to the
    // creator map. Remind him/her subtly ;-)
    if (!itsElementStack.empty())
    {
      string err = "Don't know how to create " + SAXString(localname);
      throw EXCEPTION(err.c_str());
    }
    return;
  }

  // OK, so we do know what to do with this element.
  else
  {
    // Put all attributes in a (type,value) map.
    attributemap_t attribMap;
    for(unsigned i = 0; i < attrs.getLength(); ++i)
    {
      pair<string,string> aPair(SAXString(attrs.getLocalName(i)),
				SAXString(attrs.getValue(i)));
      attribMap.insert(aPair);
    }
    // Call the creator method (which is the value field in itsCreatorMap)
    // and store the returned SAXElement*.
    CountedPointer<SAXElement> ptr(it->second(attribMap));

    // If element stack is empty, we must save a copy in itsElementTree,
    // because from now on we're building up the element tree.
    if( itsElementStack.empty() )
    {
      itsElementTree = ptr;
    }
    // Place the stored SAXElement* onto the element stack.
    itsElementStack.push(ptr);
  }

  return;
}


//#----------------------------------------------------------------------------
//#        Implementation of the SAX ErrorHandler interface
//#----------------------------------------------------------------------------

void SAXHandler::error(const SAXParseException& e)
{
  itsErrorFlag = true;
  cerr << "\nSAXHandler: "
       << "Error at file " << e.getSystemId()
       << ", line " << e.getLineNumber()
       << ", char " << e.getColumnNumber()
       << "\n  Message: " << e.getMessage() << endl;
}


void SAXHandler::fatalError(const SAXParseException& e)
{
  itsErrorFlag = true;
  cerr << "\nSAXHandler: "
       << "Fatal Error at file " << e.getSystemId()
       << ", line " << e.getLineNumber()
       << ", char " << e.getColumnNumber()
       << "\n  Message: " << e.getMessage() << endl;
  throw e;
}


void SAXHandler::warning(const SAXParseException& e)
{
  cerr << "\nSAXHandler: "
       << "Warning at file " << e.getSystemId()
       << ", line " << e.getLineNumber()
       << ", char " << e.getColumnNumber()
       << "\n  Message: " << e.getMessage() << endl;
}


void SAXHandler::resetErrors()
{
  itsErrorFlag = false;
}


bool SAXHandler::sawErrors()
{
  return itsErrorFlag;
}


CountedPointer<SAXElement> SAXHandler::getElementTree()
{
  return itsElementTree;
}
