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
// WorkHolder.cc: implementation of the WorkHolder class.
//
//////////////////////////////////////////////////////////////////////

#include "CEPFrame/WorkHolder.h"
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
