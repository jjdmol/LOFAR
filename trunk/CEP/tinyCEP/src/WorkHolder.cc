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

#include <lofar_config.h>

#include <tinyCEP/WorkHolder.h>
#include <tinyCEP/Profiler.h>

#include TRANSPORTERINCLUDE

namespace LOFAR
{

map<string,WorkHolder::WHConstruct*>* WorkHolder::itsConstructMap = 0;

int WorkHolder::theirCurAppl=0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int WorkHolder::theirReadProfilerState=0;
int WorkHolder::theirProcessProfilerState=0;
int WorkHolder::theirWriteProfilerState=0;

//WorkHolder::WorkHolder[not-in-charge](int, int, 
//std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, 
//std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)'

WorkHolder::WorkHolder (int inputs, int outputs,
			const string& name,
			const string& type)
  :   itsProcessStep      (1),
      itsNinputs          (inputs), 
      itsNoutputs         (outputs),
      itsFirstProcessCall (true),
      itsName             (name),
      itsType             (type),
      itsNode             (0),
      itsAppl             (0)

{
//   TRACER2("WorkHolder constructor");
  itsDataManager = new TinyDataManager(inputs, outputs);
  itsCurRank = TRANSPORTER::getCurrentRank();
  if (theirReadProfilerState == 0) {
    theirReadProfilerState = Profiler::defineState("WH::Read","green");
  }
  if (theirProcessProfilerState == 0) {
    theirProcessProfilerState = Profiler::defineState("WH::Process","red");
  }
  if (theirWriteProfilerState == 0) {
    theirWriteProfilerState = Profiler::defineState("WH::Write","blue");
  }
}

WorkHolder::WorkHolder (const WorkHolder& that)
: itsDataManager      (0),
  itsCurRank          (that.itsCurRank),
  itsProcessStep      (that.itsProcessStep),
  itsNinputs          (that.itsNinputs), 
  itsNoutputs         (that.itsNoutputs),
  itsFirstProcessCall (that.itsFirstProcessCall),
  itsName             (that.itsName),
  itsType             (that.itsType),
  itsNode             (that.itsNode),
  itsAppl             (that.itsAppl)
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
    itsFirstProcessCall = that.itsFirstProcessCall;
    itsDataManager      = 0;
  }
  return *this;
}

WorkHolder* WorkHolder::baseMake()
{
  WorkHolder* whp = const_cast<WorkHolder*>(this)->make (getName());
  ASSERT (whp->getDataManager().getInputs() == getDataManager().getInputs());
  ASSERT (whp->getDataManager().getOutputs() == getDataManager().getOutputs());

  return whp;
}

void WorkHolder::dump()
{
  if (shouldProcess())
  {
    for (int i=0; i<itsNinputs; i++) {
      getDataManager().getInHolder(i)->dump();
    }
    for (int i=0; i<itsNoutputs; i++) {
      getDataManager().getOutHolder(i)->dump();
    }
  }
}

void WorkHolder::basePreprocess()
{
  if (shouldProcess()) 
  {
//     TRACER4("basePreprocess Step " << getName() << " on node/appl (" 
// 	    << getNode() << '/' << getAppl() << ')');
    getDataManager().preprocess();
    preprocess();
  }
}

void WorkHolder::preprocess()
{
}

void WorkHolder::baseProcess ()
{
 if (shouldProcess()) 
 {
//    TRACER4("WorkHolder::baseprocess()");
   Profiler::enterState (theirReadProfilerState);
   if (itsFirstProcessCall) {
     getDataManager().initializeInputs();
     itsFirstProcessCall = false;
   } else {     
     // the getDM::initializeInputs() method also performs 
     // the first read action.
     
     for (int input=0; input<itsNinputs; input++) {
       //       if (getDataManager().getGeneralInHolder(input)->doHandle()) {
       // temporary in-rate sollution
       if ( itsProcessStep % getDataManager().getInputRate(input) == 0 ) {
	 LOG_TRACE_COND_STR("WorkHolder " << getName() << " << Allowed input handling;  step = " 
			    << itsProcessStep << "   rate = " << getDataManager().getInputRate(input));
	 
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
	 getDataManager().clearReadyInFlag(input);  // remove the readyInFlag in DataManager
       } else {
	 LOG_TRACE_COND_STR("WorkHolder " << getName() << " << skipped input handling;  step = " 
			    << itsProcessStep << "   rate = " << getDataManager().getInputRate(input));
       }
       
     }
   }
   Profiler::leaveState (theirReadProfilerState);
 
   
   // Now we have the input data avialable
   // and it is time to do the real work; call the process()
   if ( (itsProcessStep % getDataManager().getProcessRate()) == 0) {
     Profiler::enterState (theirProcessProfilerState);
     process();
     Profiler::leaveState (theirProcessProfilerState);
   }
   
   Profiler::enterState (theirWriteProfilerState);
   for (int output=0; output<itsNoutputs; output++)	{
     
     //     if (getDataManager().getGeneralOutHolder(output)->doHandle()) {
     if ( itsProcessStep % getDataManager().getOutputRate(output) == 0 ) {
       
       if (getDataManager().hasOutputSelector() == false) {
	 getDataManager().getOutHolder(output);
       }
       if (getDataManager().doAutoTriggerOut(output)) { 
	 getDataManager().readyWithOutHolder(output); // Will cause writing of data
       }
       getDataManager().clearReadyOutFlag(output);  // remove the readyInFlag in DataManager
     } else {
       LOG_TRACE_COND_STR("WorkHolder" << getName() << " << skipped output handling;  step = " 
			  << itsProcessStep << "   rate = " << getDataManager().getOutputRate(output));
     }
     
   }
   Profiler::leaveState (theirWriteProfilerState);
    
   itsProcessStep++;
 }

 else {
//    TRACER4("WorkHolder " << getName() << " Not on right node/appl(" 
//            << getNode() << '/' << getAppl() << "); will skip Process"); 
 }  

}

void WorkHolder::basePostprocess()
{
  if (shouldProcess()) {
//     TRACER4("WorkHolder::basePostprocess " << getName() << " on node/appl (" 
// 	   << getNode() << '/' << getAppl() << ')');
    postprocess();
    for (int input=0; input<itsNinputs; input++)	{
      getDataManager().getInHolder(input)->basePostprocess();
    }
    for (int output=0; output<itsNoutputs; output++)	{
      getDataManager().getOutHolder(output)->basePostprocess();
    }
    getDataManager().postprocess();
  }
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
      ASSERTSTR (itsInMap.find(name) == itsInMap.end(),
		 "DataHolder name " << name <<
		 " already used in WorkHolder " << itsName);
      itsInMap[name] = i;
    }
  }
  if (itsOutMap.empty()) {
    for (int i=0; i<itsNoutputs; i++) {
      // Error if DataHolder name is already used.
      const string& name = getDataManager().getGeneralOutHolder(i)->getName();
      ASSERTSTR (itsOutMap.find(name) == itsOutMap.end(),
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

bool WorkHolder::doHandle() {
  return ( itsProcessStep % getDataManager().getProcessRate() == 0 );
}

void WorkHolder::runOnNode (int aNode, int applNr)
{ 
  itsNode = aNode;
  itsAppl = applNr;
}

void WorkHolder::setDataManager(TinyDataManager* dmptr)
{
  if (itsDataManager != 0)
  {
    delete itsDataManager;    
  }  
  itsDataManager = dmptr;
}


}
