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

#include <tinyCEP/WorkHolder.h>
#include <Common/Debug.h>


namespace LOFAR
{

map<string,WorkHolder::WHConstruct*>* WorkHolder::itsConstructMap = 0;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WorkHolder::WorkHolder (int inputs, int outputs,
			const string& name,
			const string& type)
: itsNinputs          (inputs), 
  itsNoutputs         (outputs),
  itsName             (name),
  itsType             (type),
  itsFirstProcessCall (true)
{
  TRACER2("WorkHolder constructor");
  itsDataManager = new BaseDataManager(inputs, outputs);
}

WorkHolder::WorkHolder (const WorkHolder& that)
: itsNinputs          (that.itsNinputs), 
  itsNoutputs         (that.itsNoutputs),
  itsName             (that.itsName),
  itsType             (that.itsType),
  itsDataManager      (0),
  itsFirstProcessCall (that.itsFirstProcessCall)
{}

WorkHolder::~WorkHolder()
{
  delete itsDataManager;
}

WorkHolder& WorkHolder::operator= (const WorkHolder& that)
{
  if (this != &that) {
    itsNinputs          = that.itsNinputs; 
    itsNoutputs         = that.itsNoutputs;
    itsName             = that.itsName;
    itsType             = that.itsType;
    itsDataManager      = 0;
    itsFirstProcessCall = that.itsFirstProcessCall;
  }
  return *this;
}

WorkHolder* WorkHolder::baseMake()
{
  WorkHolder* whp = const_cast<WorkHolder*>(this)->make (getName());
  Assert (whp->getDataManager().getInputs() == getDataManager().getInputs());
  Assert (whp->getDataManager().getOutputs() == getDataManager().getOutputs());

  return whp;
}


void WorkHolder::dump()
{
  for (int i=0; i<itsNinputs; i++) {
   getDataManager().getInHolder(i)->dump();
  }
  for (int i=0; i<itsNoutputs; i++) {
    getDataManager().getOutHolder(i)->dump();
  }
}

void WorkHolder::basePreprocess()
{
  getDataManager().preprocess();
//   getParamManager().preprocess();
  preprocess();
}

void WorkHolder::preprocess()
{
}

void WorkHolder::baseProcess ()
{
  TRACER4("WorkHolder::baseprocess()");
  if (itsFirstProcessCall)
  {
    getDataManager().initializeInputs();
    itsFirstProcessCall = false;
  }
  else     // the getDM::initializeInputs() method also performs 
           // the first read action.
   {

    for (int input=0; input<itsNinputs; input++) {
//       if (getDataManager().getGeneralInHolder(input)->doHandle()) {
	
	// for selector type handle locking
	if (getDataManager().hasInputSelector() == false) {
	  // wait for unlocking if needed
	  getDataManager().getInHolder(input);
	}

	if (getDataManager().doAutoTriggerIn(input)) { 
	  // signal the DM that we're done with the input channel.
	  // The DM will initiate the read sequence now.
	  getDataManager().readyWithInHolder(input); 
	  
	}
//       }
    }
  } 

  // Now we have the input data avialable
  // and it is time to do the real work; call the process()
  process();
  

  for (int output=0; output<itsNoutputs; output++)	{
//     if (getDataManager().getGeneralOutHolder(output)->doHandle()) {
      if (getDataManager().hasOutputSelector() == false) {
	getDataManager().getOutHolder(output);
      }
      if (getDataManager().doAutoTriggerOut(output)) { 
	getDataManager().readyWithOutHolder(output); // Will cause writing of data
      }
//     }
  } 
}

void WorkHolder::basePostprocess()
{
  TRACER4("WorkHolder::basePostprocess()");
  postprocess();
  for (int input=0; input<itsNinputs; input++)	{
    getDataManager().getInHolder(input)->basePostprocess();
  }
  for (int output=0; output<itsNoutputs; output++)	{
    getDataManager().getOutHolder(output)->basePostprocess();
  }
  getDataManager().postprocess();
//   getParamManager().postprocess();
}

void WorkHolder::postprocess()
{}


int WorkHolder::getInChannel (const string& name)
{
  if (itsInMap.empty()) {
    fillMaps();
  }
  if (itsInMap.find(name) == itsInMap.end()) {
    return -1;
  }
  return itsInMap[name];
}

int WorkHolder::getOutChannel (const string& name)
{
  if (itsOutMap.empty()) {
    fillMaps();
  }
  if (itsOutMap.find(name) == itsOutMap.end()) {
    return -1;
  }
  return itsOutMap[name];
}

void WorkHolder::fillMaps()
{
  if (itsInMap.empty()) {
    for (int i=0; i<itsNinputs; i++) {
      // Error if DataHolder name is already used.
      const string& name = getDataManager().getGeneralInHolder(i)->getName();
      AssertStr (itsInMap.find(name) == itsInMap.end(),
		 "DataHolder name " << name <<
		 " already used in WorkHolder " << itsName);
      itsInMap[name] = i;
    }
  }
  if (itsOutMap.empty()) {
    for (int i=0; i<itsNoutputs; i++) {
      // Error if DataHolder name is already used.
      const string& name = getDataManager().getGeneralOutHolder(i)->getName();
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

}
