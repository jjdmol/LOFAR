//  SAXHandler.cc:
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
//  $Log$
//  Revision 1.2  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.1  2002/03/15 13:28:08  gvd
//  Added construct function to WH classes (for XML parser)
//  Added getX functions to ParamBlock
//  Added SAX classes for XML parser
//  Improved testing scripts (added .run)
//
//
//////////////////////////////////////////////////////////////////////

#include "BaseSim/SAXHandler.h"

#ifdef HAVE_XERCES

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/framework/XMLAttr.hpp>
#include <xercesc/framework/XMLAttDef.hpp>
#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/internal/VecAttributesImpl.hpp>

#include "BaseSim/Transport.h"
#include "BaseSim/ParamBlock.h"
#include "BaseSim/WH_Empty.h"
#include "BaseSim/SAXSimulParser.h"
#include "Common/Debug.h"


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
  chOpenAngle, chQuestion, chLatin_x, chLatin_m, chLatin_l
  ,   chSpace, chLatin_v, chLatin_e, chLatin_r, chLatin_s, chLatin_i
  ,   chLatin_o, chLatin_n, chEqual, chDoubleQuote, chDigit_1, chPeriod
  ,   chDigit_0, chDoubleQuote, chSpace, chLatin_e, chLatin_n, chLatin_c
  ,   chLatin_o, chLatin_d, chLatin_i, chLatin_n, chLatin_g, chEqual
  ,   chDoubleQuote, chNull
};

static const XMLCh  gXMLDecl2[] =
{
  chDoubleQuote, chQuestion, chCloseAngle
  ,   chLF, chNull
};

// ---------------------------------------------------------------------------
// Elements and attributes defined in basesim.dtd
// ---------------------------------------------------------------------------

static XMLCh* ROOT_ELEMENT;
static XMLCh* SIMUL_ELEMENT;
static XMLCh* STEP_ELEMENT;
static XMLCh* WORKHOLDER_ELEMENT;
static XMLCh* DATAHOLDER_ELEMENT;
static XMLCh* CONNECT_ELEMENT;
static XMLCh* EXSTEP_ELEMENT;
static XMLCh* CLASS_ATT;
static XMLCh* DH_TYPE_ATT;
static XMLCh* NAME_ATT;
static XMLCh* EXNAME_ATT;
static XMLCh* NODE_ATT;
static XMLCh* SRC_ATT;
static XMLCh* DST_ATT;
static XMLCh* EMPTY_STRING;

// ---------------------------------------------------------------------------
//  SAXHandler: Constructors and Destructor
// ---------------------------------------------------------------------------
SAXHandler::SAXHandler()  
{
  ROOT_ELEMENT        = XMLString::transcode("basesim");
  SIMUL_ELEMENT       = XMLString::transcode("simul");
  STEP_ELEMENT        = XMLString::transcode("step");
  WORKHOLDER_ELEMENT  = XMLString::transcode("workholder");
  DATAHOLDER_ELEMENT  = XMLString::transcode("dataholder");
  CONNECT_ELEMENT     = XMLString::transcode("connect");
  EXSTEP_ELEMENT      = XMLString::transcode("exstep");
  CLASS_ATT           = XMLString::transcode("class"); 
  DH_TYPE_ATT         = XMLString::transcode("type");
  NAME_ATT            = XMLString::transcode("name");
  EXNAME_ATT          = XMLString::transcode("exname");
  NODE_ATT            = XMLString::transcode("node");
  SRC_ATT             = XMLString::transcode("src");
  DST_ATT             = XMLString::transcode("dest");
  EMPTY_STRING        = XMLString::transcode("");

  itsActiveFlag = true;
  itsFocusSimul = "<\">"; // 'impossible name'
}

SAXHandler::SAXHandler(const string& rootSimul)
{
  SAXHandler();
  itsActiveFlag = false;
  itsFocusSimul = rootSimul;
}

SAXHandler::~SAXHandler()
{
  delete [] ROOT_ELEMENT;
  delete [] SIMUL_ELEMENT;
  delete [] STEP_ELEMENT;
  delete [] WORKHOLDER_ELEMENT;
  delete [] DATAHOLDER_ELEMENT;
  delete [] CONNECT_ELEMENT;
  delete [] EXSTEP_ELEMENT;
  delete [] CLASS_ATT;
  delete [] DH_TYPE_ATT;
  delete [] NAME_ATT;
  delete [] EXNAME_ATT;
  delete [] NODE_ATT;
  delete [] SRC_ATT;
  delete [] DST_ATT;
  delete [] EMPTY_STRING;
}

void SAXHandler::startDocument()
{

#ifdef HAVE_CORBA
  // Start Orb Environment
  AssertStr(BS_Corba::init(),
	    "Could not initialize CORBA environment");
#endif
  itsRootSimulWasSet = false;
}


void SAXHandler::endDocument()
{
}

WorkHolder* SAXHandler::instWorkHolderByName(const string& className,
					     size_t inputs, size_t outputs,
					     const string& whName)
{
  ParamBlock params;
  WorkHolder::WHConstruct* construct = WorkHolder::getConstruct (className);
  AssertMsg (construct != 0, "Workholder " << className << " is unknown");
  return construct (whName, inputs, outputs, params);
}


void SAXHandler::handleRoot (const Attributes& attributes, bool start)
{
  if (!itsActiveFlag) return;
}


void SAXHandler::handleSimul (const Attributes& attributes, bool start)
{
  if (start) {
    SAXLocalStr name1(attributes.getValue(NAME_ATT));
    if (itsFocusSimul == name1.localForm()) {
      itsActiveFlag = true;
    }
    if (!itsActiveFlag) return;
    
    // A Simul needs a WorkHolder in order to be instantiated. So we postpone
    // its creation until we encounter its WorkHolder
    itsContainerType = SIMUL_ELEMENT;
  } else {
    SAXLocalStr name2(attributes.getValue(NAME_ATT));
    if (itsFocusSimul == name2.localForm()) {
      itsActiveFlag = false;
    }
    if (!itsActiveFlag) return;
    itsParentSimul = itsParentSimul.getParent();
  }
}


void SAXHandler::handleStep (const Attributes& attributes, bool start)
{
  if (start) {
    SAXLocalStr name1(attributes.getValue(NAME_ATT));
    if (itsFocusSimul == name1.localForm()) {
      itsActiveFlag = true;
    }
    if (!itsActiveFlag) return;

    // A Simul needs a WorkHolder in order to be instantiated. So we postpone
    // its creation until we encounter its WorkHolder
    itsContainerType = STEP_ELEMENT;
  } else {
    SAXLocalStr name2(attributes.getValue(NAME_ATT));
    if (itsFocusSimul == name2.localForm()) {
      itsActiveFlag = false;
    }
    if (!itsActiveFlag) return;
  }
}


void SAXHandler::handleWorkHolder (const Attributes& attributes, bool start)
{
  int index;
  if (!itsActiveFlag) return;
  if (start) {
    // Before we can instantiate a WorkHolder we need to know how many
    // DataHolders it contains. We postpone the creation until we found
    // the matching end tag for this WorkHolder.
    itsInDHVector.clear();
    itsOutDHVector.clear();
  } else {
    const XMLCh* className = attributes.getValue(CLASS_ATT);
    const XMLCh* name      = attributes.getValue(NAME_ATT);
    string localClassName = SAXLocalStr(className).localForm();
    string localWHName = SAXLocalStr(name).localForm();
    WorkHolder *wh = instWorkHolderByName(localClassName,
					  itsInDHVector.size(),
					  itsOutDHVector.size(),
					  localWHName);
    itsStoredWorkHolder = wh;
    
    // We rename all DataHolders the WorkHolder created so they match
    // the names specified in the XML document
    index = 0;
    for (std::vector<string>::const_iterator it=itsInDHVector.begin(); 
	 it != itsInDHVector.end(); it++) {
      DataHolder* dh = wh->getInHolder(index++);
      AssertStr (dh,
		 "Invalid # of DataHolder in WorkHolder of class "
		 << localClassName);
      dh->setName(*it);
    }
    index = 0;
    for (std::vector<string>::const_iterator it=itsOutDHVector.begin(); 
	 it != itsOutDHVector.end(); it++) {
      DataHolder* dh = wh->getOutHolder(index++);
      AssertStr (dh,
		 "Invalid # of DataHolder in WorkHolder of class "
		 << localClassName);
      dh->setName(*it);
    }

    // We can now instantiate the Step/Simul this WorkHolder belongs to.
    Attributes* stepAtts = itsAttStack.top();
    SAXLocalStr sname(stepAtts->getValue(NAME_ATT));
    SAXLocalStr snode(stepAtts->getValue(NODE_ATT));

    if (sname.localForm() == "st1") {
      cout << "Found " << sname.localForm() << " outputting its dataholders: " << endl << endl;
      cout << "Input dataholders: " << endl;
      for (int i=0; i<wh->getInputs(); i++){
	cout << "    dataholder " << i << " : " << wh->getInHolder(i)->getName() << endl;
      }
      cout << "Output dataholders: " << endl;
      for (int i=0; i<wh->getOutputs(); i++){
	cout << "    dataholder " << i << " : " << wh->getOutHolder(i)->getName() << endl;
      }
    }

    if (XMLString::compareString(itsContainerType,SIMUL_ELEMENT) == 0) {
      Simul simul(*wh, sname.localForm(), false);

      if (!itsRootSimulWasSet) {
	itsRootSimul = simul;
      } else {
	itsParentSimul.addStep(simul);
      }
      itsParentSimul = simul;
      if (! snode.localForm().empty()) {
	simul.runOnNode (atoi(snode.localForm().c_str()));
      }
      itsRootSimulWasSet = true;
    } else {

      Step step (wh, sname.localForm(), false);
      itsParentSimul.addStep(step);
      if (! snode.localForm().empty()) {
	step.runOnNode(atoi(snode.localForm().c_str()));
      }
    }
  }
}


void SAXHandler::handleDataHolder (const Attributes& attributes, bool start)
{
  if (!itsActiveFlag) return;
  if (start) {
    SAXLocalStr name(attributes.getValue(NAME_ATT));
    SAXLocalStr dhtype(attributes.getValue(DH_TYPE_ATT));
    if (dhtype.localForm() == "in") {
      itsInDHVector.insert(itsInDHVector.end(),string(name.localForm()));    
    } else {
      itsOutDHVector.insert(itsOutDHVector.end(),string(name.localForm()));
    }
  }
}


void SAXHandler::handleConnect (const Attributes& attributes, bool start)
{
  if (!itsActiveFlag) return;
  if (start) {
    SAXLocalStr src(attributes.getValue(SRC_ATT));
    SAXLocalStr dst(attributes.getValue(DST_ATT));
    itsParentSimul.connect(src.localForm(),dst.localForm());
  } else {
    // REVISIT: implement
  }
}


void SAXHandler::handleExStep (const Attributes& attributes, bool start)
{
  if (!itsActiveFlag) return;
  if (start) {
    SAXLocalStr name(attributes.getValue(NAME_ATT));
    SAXLocalStr exname(attributes.getValue(EXNAME_ATT));
    SAXLocalStr src(attributes.getValue(SRC_ATT));
    SAXLocalStr node(attributes.getValue(NODE_ATT));

    SAXSimulParser parser(src.localForm(), exname.localForm());
    Step step = parser.parseSimul();
    step.setName (name.localForm());
    itsParentSimul.addStep (parser.parseSimul());
  } else {
    // REVISIT: implement
  }
}

void SAXHandler::startElement (const XMLCh* const uri,
			       const XMLCh* const localname,
			       const XMLCh* const qname,
			       const Attributes& attributes)
{
  itsAttStack.push(copyAttributes(attributes));
  if (XMLString::compareString(localname,ROOT_ELEMENT) == 0) {
    handleRoot(attributes,true);
  } else if (XMLString::compareString(localname,SIMUL_ELEMENT) == 0) {
    handleSimul(attributes,true);
  } else if (XMLString::compareString(localname,STEP_ELEMENT) == 0) {
    handleStep(attributes,true);
  } else if (XMLString::compareString(localname,WORKHOLDER_ELEMENT) == 0) {
    handleWorkHolder(attributes,true);
  } else if (XMLString::compareString(localname,DATAHOLDER_ELEMENT) == 0) {
    handleDataHolder(attributes,true);
  } else if (XMLString::compareString(localname,CONNECT_ELEMENT) == 0) {
    handleConnect(attributes,true);
  } else if (XMLString::compareString(localname,EXSTEP_ELEMENT) == 0) {
    handleExStep(attributes,true);
  }
}


void SAXHandler::endElement (const XMLCh* const uri,
			     const XMLCh* const localname,
			     const XMLCh* const qname)
{
  Attributes* attributes = itsAttStack.top();
  itsAttStack.pop();
  if (XMLString::compareString(localname,ROOT_ELEMENT) == 0) {
    handleRoot(*attributes,false);
  } else if (XMLString::compareString(localname,SIMUL_ELEMENT) == 0) {
    handleSimul(*attributes,false);
  } else if (XMLString::compareString(localname,STEP_ELEMENT) == 0) {
    handleStep(*attributes,false);
  } else if (XMLString::compareString(localname,WORKHOLDER_ELEMENT) == 0) {
    handleWorkHolder(*attributes,false);
  } else if (XMLString::compareString(localname,DATAHOLDER_ELEMENT) == 0) {
    handleDataHolder(*attributes,false);
  } else if (XMLString::compareString(localname,CONNECT_ELEMENT) == 0) {
    handleConnect(*attributes,false);
  } else if (XMLString::compareString(localname,EXSTEP_ELEMENT) == 0) {
    handleExStep(*attributes,false);
  }
  delete attributes;
}

VecAttributesImpl* SAXHandler::copyAttributes(const Attributes &att)
{
  const XMLCh* name;
  const XMLCh* value;
  char* type_s;
  XMLAttDef::AttTypes type;
  RefVectorOf<XMLAttr>* ref = 
    new RefVectorOf<XMLAttr>(att.getLength(),true); // (adopts elements)
  for (unsigned int i=0; i<att.getLength(); i++) {
    name = att.getLocalName(i);
    value = att.getValue(i);
    type_s = XMLString::transcode(att.getType(i));
    if (strcmp(type_s,"CDATA") == 0) {
      type = XMLAttDef::CData;
    } else
    if (strcmp(type_s,"ID") == 0) {
      type = XMLAttDef::ID;
    } else
    if (strcmp(type_s,"IDREF") == 0) {
      type = XMLAttDef::IDRef;
    } else
    if (strcmp(type_s,"NMTOKEN") == 0) {
      type = XMLAttDef::NmToken;
    } else
    if (strcmp(type_s,"NMTOKENS") == 0) {
      type = XMLAttDef::NmTokens;
    } else
    if (strcmp(type_s,"ENTITY") == 0) {
      type = XMLAttDef::Entity;
    } else
    if (strcmp(type_s,"ENTITIES") == 0) {
      type = XMLAttDef::Entities;
    } else
    if (strcmp(type_s,"NOTATION") == 0) {
      type = XMLAttDef::Notation;
    }
    delete [] type_s;
    ref->addElement (new XMLAttr(0, name, EMPTY_STRING, value, type));
  }
  VecAttributesImpl* vecAttr = new VecAttributesImpl();
  vecAttr->setVector(ref,att.getLength(), 0, true);
  return vecAttr;
}

void SAXHandler::warning (const SAXParseException &exception)
{
  SAXLocalStr message(exception.getMessage());
  cout << "Warning: " << message
       << "(line " << exception.getLineNumber() << ")" << endl;
}

void SAXHandler::error (const SAXParseException &exception)
{
  SAXLocalStr message(exception.getMessage());
  cout << "Error: " << message 
       << "(line " << exception.getLineNumber() << ")" << endl;
}

void SAXHandler::fatalError (const SAXParseException &exception)
{
  SAXLocalStr message(exception.getMessage());
  cout << "Error (fatal): " << message 
       << "(line " << exception.getLineNumber() << ")" << endl;
}

void SAXHandler::resetErrors()
{}

#endif
