//  WorkHolder.cc:
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
//  Revision 1.21  2002/06/10 09:44:15  diepen
//
//  %[BugId: 37]%
//  setRead and setWrite have been replaced by setReadDelay.
//
//  Revision 1.20  2002/05/16 15:17:55  schaaf
//  modified TRACER levels and output
//
//  Revision 1.19  2002/05/14 07:54:47  gvd
//  Moved virtual functions to .cc file
//  Removed INCLUDES from ShMem Makefile.am
//  Add LOFAR_DEPEND to test/Makefile.am
//
//  Revision 1.18  2002/05/08 14:20:51  wierenga
//  Keep compiler happy (avoid warning).
//
//  Revision 1.17  2002/05/03 11:21:32  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.16  2002/05/02 12:16:24  schaaf
//  Added method getMonitorValue
//
//  Revision 1.15  2002/03/15 13:28:09  gvd
//  Added construct function to WH classes (for XML parser)
//  Added getX functions to ParamBlock
//  Added SAX classes for XML parser
//  Improved testing scripts (added .run)
//
//  Revision 1.14  2002/03/04 12:54:01  gvd
//  Let WorkHolder copy the name of DataHolders; done by creating baseMake
//
//  Revision 1.13  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.12  2001/12/07 13:58:20  gvd
//  Changes to make connect by name possible
//  Avoid leaks in firewall
//  Replace resolveComm by a new simplifyConnections
//
//  Revision 1.11  2001/10/05 11:50:37  gvd
//  Added getType function
//
//  Revision 1.10  2001/09/24 14:04:09  gvd
//  Added preprocess and postprocess functions
//
//  Revision 1.9  2001/09/21 12:19:02  gvd
//  Added make functions to WH classes to fix memory leaks
//
//  Revision 1.8  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.7  2001/02/05 14:53:05  loose
//  Added GPL headers
//

// WorkHolder.cc: implementation of the WorkHolder class.
//
//////////////////////////////////////////////////////////////////////

#include "BaseSim/WorkHolder.h"
#include "Common/Debug.h"


map<string,WorkHolder::WHConstruct*>* WorkHolder::itsConstructMap = 0;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WorkHolder::WorkHolder (int inputs, int outputs,
			const string& name,
			const string& type)
: itsNinputs     (inputs), 
  itsNoutputs    (outputs),
  itsName        (name),
  itsType        (type),
  itsProcMode    (Process)
{}

WorkHolder::WorkHolder (const WorkHolder& that)
: itsNinputs     (that.itsNinputs), 
  itsNoutputs    (that.itsNoutputs),
  itsName        (that.itsName),
  itsType        (that.itsType),
  itsProcMode    (that.itsProcMode)
{}

WorkHolder::~WorkHolder()
{
}

WorkHolder& WorkHolder::operator= (const WorkHolder& that)
{
  if (this != &that) {
    itsNinputs     = that.itsNinputs; 
    itsNoutputs    = that.itsNoutputs;
    itsName        = that.itsName;
    itsType        = that.itsType;
    itsProcMode    = that.itsProcMode;
  }
  return *this;
}

WorkHolder* WorkHolder::baseMake() const
{
  WorkHolder* whp = make (getName());
  Assert (whp->getInputs() == getInputs());
  Assert (whp->getOutputs() == getOutputs());
  for (int i=0; i<getInputs(); i++) {
    whp->getInHolder(i)->setName (getConstInHolder(i)->getName());
  }
  for (int i=0; i<getOutputs(); i++) {
    whp->getOutHolder(i)->setName (getConstOutHolder(i)->getName());
  }
  return whp;
}


void WorkHolder::dump() const
{
  for (int i=0; i<itsNinputs; i++) {
    getConstInHolder(i)->dump();
  }
  for (int i=0; i<itsNoutputs; i++) {
    getConstOutHolder(i)->dump();
  }
}

void WorkHolder::basePreprocess()
{
  preprocess();
  for (int input=0; input<itsNinputs; input++)	{
    getInHolder(input)->basePreprocess();
  }
  for (int output=0; output<itsNoutputs; output++)	{
    getOutHolder(output)->basePreprocess();
  }
}

void WorkHolder::preprocess()
{}

void WorkHolder::baseProcess ()
{
  TRACER4("WorkHolder::baseprocess()");
  for (int input=0; input<itsNinputs; input++) {
    if (getInHolder(input)->doHandle()) {
      getInHolder(input)->read ();
    }
  }        
  
  switch (WorkHolder::getProcMode()) {
  case Process:
    process();
    break;
  case Zeroes:
    // initialize all output
    for (int output=0; output<itsNoutputs; output++) {
      getOutHolder(output)->setZeroes();
    }
    break;
  case Ones :
    // initialize all output
    for (int output=0; output<itsNoutputs; output++) {
      getOutHolder(output)->setOnes();
    }
    break;
  default :
    break;
  }

  for (int output=0; output<itsNoutputs; output++)	{
    if (getOutHolder(output)->doHandle()) {
      getOutHolder(output)->write ();
    }
  }     
}

void WorkHolder::basePostprocess()
{
  postprocess();
  for (int input=0; input<itsNinputs; input++)	{
    getInHolder(input)->basePostprocess();
  }
  for (int output=0; output<itsNoutputs; output++)	{
    getOutHolder(output)->basePostprocess();
  }
}

void WorkHolder::postprocess()
{}


int WorkHolder::getInChannel (const string& name) const
{
  if (itsInMap.empty()) {
    fillMaps();
  }
  if (itsInMap.find(name) == itsInMap.end()) {
    return -1;
  }
  return itsInMap[name];
}

int WorkHolder::getOutChannel (const string& name) const
{
  if (itsOutMap.empty()) {
    fillMaps();
  }
  if (itsOutMap.find(name) == itsOutMap.end()) {
    return -1;
  }
  return itsOutMap[name];
}

void WorkHolder::fillMaps() const
{
  if (itsInMap.empty()) {
    for (int i=0; i<itsNinputs; i++) {
      // Error if DataHolder name is already used.
      const string& name = getConstInHolder(i)->getName();
      AssertStr (itsInMap.find(name) == itsInMap.end(),
		 "DataHolder name " << name <<
		 " already used in WorkHolder " << itsName);
      itsInMap[name] = i;
    }
  }
  if (itsOutMap.empty()) {
    for (int i=0; i<itsNoutputs; i++) {
      // Error if DataHolder name is already used.
      const string& name = getConstOutHolder(i)->getName();
      AssertStr (itsOutMap.find(name) == itsOutMap.end(),
		 "DataHolder name " << name <<
		 " already used in WorkHolder " << itsName);
      itsOutMap[name] = i;
    }
  }
}


void WorkHolder::registerConstruct (const string& name,
				    WorkHolder::WHConstruct* construct)
{
  if (!itsConstructMap) {
    itsConstructMap = new map<string,WHConstruct*>;
  }
  (*itsConstructMap)[name] = construct;
}

WorkHolder::WHConstruct* WorkHolder::getConstruct (const string& name)
{
  if (itsConstructMap) {
    if (itsConstructMap->find(name) != itsConstructMap->end()) {
      return (*itsConstructMap)[name];
    }
  }
  return 0;
}

int WorkHolder::getMonitorValue (const char*)
{
  return 0;
}
