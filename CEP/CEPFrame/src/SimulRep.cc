//  SimulRep.cc:
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
//
//////////////////////////////////////////////////////////////////////

#include <Common/lofar_iostream.h>
#include <Common/lofar_algorithm.h>    // for min,max

#include <unistd.h>
#include "CEPFrame/SimulRep.h"
#include "CEPFrame/Step.h"
#include "CEPFrame/Simul.h"
#include "CEPFrame/TH_Mem.h"
#include "CEPFrame/Profiler.h"
#include "CEPFrame/WH_Empty.h"
#include "Common/Debug.h"
#include "CEPFrame/VirtualMachine.h"

#ifdef HAVE_CORBA
#include "CEPFrame/Corba/BS_Corba.h"
#include "CEPFrame/Corba/CorbaController.h"
#endif 

// Set static variables
int SimulRep::theirProcessProfilerState=0; 
int SimulRep::theirInReadProfilerState=0; 
int SimulRep::theirInWriteProfilerState=0; 
int SimulRep::theirOutReadProfilerState=0; 
int SimulRep::theirOutWriteProfilerState=0; 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SimulRep::SimulRep (const WorkHolder& worker, 
		    const string& name,
		    bool addNameSuffix, 
		    bool controllable,
		    bool monitor)
: StepRep          (worker, name, addNameSuffix, monitor),
  itsIsHighestLevel(true),
  itsController    (0)
{
  TRACER2("SimulRep C'tor");
  if (controllable) {
    TRACER2("Create controllable Simul " << name);
#ifdef HAVE_CORBA
    // Create a CorbaController object and connect it to the VirtualMachine.
    itsController = new CorbaController (BS_Corba::getPOA(),
					 BS_Corba::getPOAManager(),
					 name, 
					 &itsVM);
#else
    TRACER3("CORBA is not configured, so CorbaMonitor cannot be used in Simul ");
#endif 
  }

  if (theirProcessProfilerState == 0) {
    theirProcessProfilerState = Profiler::defineState("Simul_Process","grey");
  }
  if (theirInReadProfilerState == 0) {
    theirInReadProfilerState = Profiler::defineState("Simul_InRead","pink");
  }
  if (theirInWriteProfilerState == 0) {
    theirInWriteProfilerState = Profiler::defineState("Simul_InWrite","purple");
  }
  if (theirOutReadProfilerState == 0) {
    theirOutReadProfilerState = Profiler::defineState("Simul_OutRead","purple");
  }
  if (theirOutWriteProfilerState == 0) {
    theirOutWriteProfilerState = Profiler::defineState("Simul_OutWrite","red");
  }
}

SimulRep::~SimulRep()
{
  for (list<Step*>::iterator iter=itsSteps.begin();
       iter!=itsSteps.end(); iter++) {
    delete *iter;
  }
#ifdef HAVE_CORBA
  delete itsController;
#endif
}

void SimulRep::addStep (const Step& step)
{
  TRACER2("Simul::addStep " << step.getName());
  // Error if the step is already used in a simul.
  AssertStr (step.getRep()->getParent() == 0,
	     "Step " << step.getName() << " already used in another simul");
  // Make a copy of the Step object.
  // Note that the underlying StepRep is shared (reference counted).
  Step* aStep = step.clone();
  StepRep* stepPtr = aStep->getRep();
  // Set the sequence number (which might change the name).
  stepPtr->setSeqNr (itsSteps.size());
  // Error if the step name is already used.
  AssertStr (itsNameMap.find(stepPtr->getName()) == itsNameMap.end(),
	     "Step name '%s' already used in this simul" <<
	     step.getName());
  // Put the copy of the step in list.
  itsSteps.push_back (aStep);
  // Add the name to the map.
  itsNameMap[stepPtr->getName()] = stepPtr;
  // Tell that the step has been added to a Simul.
  stepPtr->setParent (*this);
  if (stepPtr->isSimul()) {
    SimulRep* simulPtr = dynamic_cast<SimulRep*>(stepPtr);
    // update the ishighestlevel flag of the aStep
    simulPtr->setNotHighestLevel();
  }
}

void SimulRep::runOnNode (int aNode, int applNr)
{
  StepRep::runOnNode (aNode, applNr);
  for (list<Step*>::iterator iter=itsSteps.begin();
       iter!=itsSteps.end(); iter++) {
    (*iter)->runOnNode (aNode, applNr);
  }
}


bool SimulRep::connect (const string& sourceName, const string& targetName,
			const TransportHolder& prototype)
{
  int sourceDH, targetDH;
  StepRep* sourceStep;
  StepRep* targetStep;
  if (! splitName (true, sourceName, sourceStep, sourceDH)) {
    return false;
  }
  if (! splitName (false, targetName, targetStep, targetDH)) {
    return false;
  }
  AssertStr (sourceStep != targetStep,
	     "Attempt to connect Step/Simul " <<
	     sourceStep->getName() << " to itself");
  if (sourceStep == this) {
    if (sourceDH < 0  &&  targetDH < 0) {
      Step tgt(targetStep);
      return connect_thisIn_In (&tgt, 0, 0, 0, prototype);
    } else {
      return connectData (prototype,
			  sourceStep->getInData(max(0,sourceDH)),
			  targetStep->getInData(max(0,targetDH)));
    }
  } else if (targetStep == this) {
    if (sourceDH < 0  &&  targetDH < 0) {
      Step src(sourceStep);
      connect_thisOut_Out (&src, 0, 0, 0, prototype);
    } else {
      return connectData (prototype,
			  sourceStep->getOutData(max(0,sourceDH)),
			  targetStep->getOutData(max(0,targetDH)));
    }
  } else {
    // If no DataHolders given, connect all of them in the steps.
    // Otherwise connect given DataHolder. Take first if not given.
    if (sourceDH < 0  &&  targetDH < 0) {
      Step src(sourceStep);
      return targetStep->connectInput (&src, prototype);
    } else {
      return targetStep->connectRep (sourceStep, max(0,targetDH),
				     max(0,sourceDH), 1, prototype);
    }
  }
  return false;
}

bool SimulRep::splitName (bool isSource, const string& name,
			  StepRep*& step, int& dhIndex)
{
  dhIndex = -1;
  step = 0;
  // Determine if we have to take the input or output DataHolder.
  bool takeOut = isSource;
  // Split the name at the .
  int i = name.find('.');
  // Find the step part and object. No step part means this Simul.
  if (i == 0) {
    step = this;
    takeOut = !isSource;
  } else {
    string stepName = name;
    if (i > 0) {
      stepName = name.substr (0, i);
    }
    AssertStr (itsNameMap.find(stepName) != itsNameMap.end(),
	       "Step name " << stepName << " is unknown");
    step = itsNameMap[stepName];
  }
  // Find the DataHolder name.
  // No name means all DataHolders in the Step.
  string dhName;
  if (i >= 0) {
    i++;
    if (i < int(name.length())) {
      dhName = name.substr (i, name.length()-i);
    }
  }
  // Find the DataHolder index in the WorkHolder of the Step.
  // It can be an input or an output DataHolder depending on:
  // - is it a source or target
  // - is it this Simul or a Step in the Simul.
  if (! dhName.empty()) {
    string type = "Output";
    if (takeOut) {
      dhIndex = step->getWorker()->getOutChannel(dhName);
    } else {
      type = "Input";
      dhIndex = step->getWorker()->getInChannel(dhName);
    }
    AssertStr (dhIndex >= 0,
	       type << " DataHolder " << dhName <<
	       " unknown in step/simul " << step->getName());
  }
  return true;
}


bool SimulRep::connect_thisOut_Out (Step* aStep,          
				    int   thisChannelOffset,
				    int   thatChannelOffset,
				    int   skip,
				    const TransportHolder& prototype)
{
  if (aStep == NULL) {
    return false;
  }
  // determine how much channels to loop
  int loopSize = min(aStep->getWorker()->getOutputs(),
		     this->getWorker()->getOutputs());
  for (int channel=0; channel < loopSize ; channel += skip+1) {
    int thischannel = channel+thisChannelOffset;   // channel nr in this Step
    int thatchannel = channel+thatChannelOffset;   // channel nr in aStep
    
    connectData (prototype,
		 aStep->getOutData(thatchannel),
		 this->getOutData(thischannel));

    TRACER2("connect_OutOut; Connect " << getName() << "(ID = "
	   << getID() << " ) channel " << thischannel << "to : ("
	   << aStep->getRep()->getID() << ") OutTransport ID = " 
	   << aStep->getOutTransport(thatchannel).getItsID() << " ");  
  }
  return true;
}

bool SimulRep::connect_thisIn_In (Step* aStep,          
				  int   thisChannelOffset,
				  int   thatChannelOffset,
				  int   skip,
				  const TransportHolder& prototype)
{
  if (aStep == NULL) {
    return false;
  }

  // determine how much channels to loop
  int loopSize = min(aStep->getWorker()->getInputs(),
		     this->getWorker()->getInputs());
  for (int channel=0; channel < loopSize ; channel += skip+1) {
    int thischannel = channel+thisChannelOffset;  // channel nr in this Step
    int thatchannel = channel+thatChannelOffset;  // channel nr in aStep
    connectData (prototype,
		 this->getInData(thischannel),
		 aStep->getInData(thatchannel));

    TRACER2( "connect_thisIn_In; Connect " << getName() << "(ID = " << getID() 
	   << " ) channel " << thischannel << " : InTransport InID = " 
	   << aStep->getInTransport(thatchannel).getItsID() << " ");  
  }
  return true;
}


bool SimulRep::connectInputToArray (Step* aStep[],   // pointer to  array of ptrs to Steps
				 int    nrItems, // nr of Steps in aStep[] array
				 int    skip,     // skip in inputs in aStep 
				 int    offset,   // start with this input nr in aStep
				 const TransportHolder& prototype)
{

  TRACER2("connectInputToArray " 
	 << getName() << " "
	 << aStep[0]->getName());
  if (aStep==NULL) return false;
  int channelOffset=0;
  for (int item=offset; item<nrItems; item ++) {
    AssertStr(getWorker()->getInputs()  
	    >=  channelOffset + aStep[item]->getWorker()->getInputs() - skip,
		 "Step::connectInputToArray not enough inputs");
    connect_thisIn_In (aStep[item],
		       channelOffset, // offset in this
		       offset,
		       skip,         // offset in aStep
		       prototype);

    channelOffset += aStep[item]->getWorker()->getInputs() - skip;
  }
  return true;
  
}

bool SimulRep::connectOutputToArray (Step* aStep[],  // array of ptrs to Steps
				  int    nrItems,
				  int    skip,    // skip in inputs in aStep 
				  int    offset,  // start with this input nr in aStep
				  const TransportHolder& prototype)
{ // nr of Steps in aStep[] array
  
  TRACER2( "connectOutputToArray " 
	 << getName() << " "
	 << aStep[0]->getName());
  if (aStep==NULL) return false;
  int channelOffset=0;
  for (int item=0; item<nrItems; item++) {
    AssertStr (getWorker()->getOutputs() >=
	       channelOffset + aStep[item]->getWorker()->getOutputs(),
	       "Step::connectOutputToArray not enough outputs");
    connect_thisOut_Out (aStep[item],
			 channelOffset, // offset in this
			 offset,
			 skip,
			 prototype);
    channelOffset += aStep[item]->getWorker()->getOutputs() - skip;
  }
  return true;
}


bool SimulRep::checkConnections (ostream& os, const StepRep* parent)
{
  // Check if the Simul DataHolders are connected.
  bool result = StepRep::checkConnections (os, parent);
  // Check all steps in the simul.
  for (list<Step*>::iterator iter=itsSteps.begin();
       iter!=itsSteps.end(); iter++) {
    result &= (*iter)->checkConnections (os, this);
  }
  return result;
}


void SimulRep::shortcutConnections()
{
  // Make a shortcut (if possible) for the DataHolders of this Simul.
  for (int ch=0; ch<getWorker()->getInputs(); ch++) {
    doShortcut (getInTransport(ch));
  }
  for (int ch=0; ch<getWorker()->getOutputs(); ch++) {
    doShortcut (getOutTransport(ch));
  }
  // Do the same for all simuls in this Simul.
  for (list<Step*>::iterator iter=itsSteps.begin();
       iter!=itsSteps.end(); iter++) {
    (*iter)->shortcutConnections();
  }
}

void SimulRep::doShortcut (Transport& tp)
{
  // The Simul can be shortcut if it has a source and target.
  DataHolder* src = tp.getSourceAddr();
  DataHolder* dst = tp.getTargetAddr();
  if (src && dst) {
    Transport& srctp = src->getTransport();
    Transport& dsttp = dst->getTransport();
    DbgAssert (srctp.getTargetAddr() == tp.getDataHolder());
    DbgAssert (dsttp.getSourceAddr() == tp.getDataHolder());
    // There are two connections which might be replaced by a single one.
    // That can be done if both connections have the same type or
    // if one of the connections is a TH_Mem one.
    // Also if both run on the same node, a single TH_Mem connection
    // can be used.
    bool replaced = false;
    if (srctp.getNode() == dsttp.getNode()) {
      srctp.makeTransportHolder (TH_Mem());
      dsttp.makeTransportHolder (TH_Mem());
      replaced = true;
    } else {
      // Shortcut is possible if they use the same Transport mechanism
      // or if one of them uses TH_Mem.
      // In the latter case we have to replace one of the mechanisms.
      if (srctp.getTransportHolder()->getType() ==
          dsttp.getTransportHolder()->getType()) {
	replaced = true;
      } else if (srctp.getTransportHolder()->getType() == "TH_Mem") {
	srctp.makeTransportHolder (*(dsttp.getTransportHolder()));
	replaced = true;
      } else if (dsttp.getTransportHolder()->getType() == "TH_Mem") {
	dsttp.makeTransportHolder (*(srctp.getTransportHolder()));
	replaced = true;
      }
    }
    // If replaced, set correct source and target, etc..
    if (replaced) {
      srctp.setTargetAddr (dsttp.getDataHolder());
      dsttp.setSourceAddr (srctp.getDataHolder());
      dsttp.setReadTag (tp.getReadTag());
      // Clear transport for the Simul.
      tp.setReadTag (-1);
      tp.setWriteTag (-1);
      tp.setSourceAddr (0);
      tp.setTargetAddr (0);
    }
  }
}


void SimulRep::simplifyConnections()
{
  // Do a simplifyComm for this Simul.
  StepRep::simplifyConnections();
  // Do the same for all steps in this Simul.
  for (list<Step*>::iterator iter=itsSteps.begin();
       iter!=itsSteps.end(); iter++) {
    (*iter)->simplifyConnections();
  }
}

void SimulRep::optimizeConnectionsWith(const TransportHolder& newTH)
{
  // Do a simplifyComm for this Simul.
  StepRep::optimizeConnectionsWith(newTH);
  // Do the same for all steps in this Simul.
  for (list<Step*>::iterator iter=itsSteps.begin();
       iter!=itsSteps.end(); iter++) {
    (*iter)->optimizeConnectionsWith(newTH);
  }
}

void SimulRep::preprocess()
{
  TRACER4("Simul " << getName() << " preprocess");
  StepRep::preprocess();
  list<Step*>::iterator iList;
  for (iList = itsSteps.begin(); iList != itsSteps.end(); ++iList) {
    (*iList)->preprocess();
  }
}

void SimulRep::process()
{
  /// 1) Read InData
  /// 2) process all steps
  /// 3) Write OutData

  // The VirtualMachine controls the state of this SimulRep.
  // The process() method will only do something while in the "running" mode.
  // The "aborting" state will cause an exit(0)
  // (which may be changed in later versions).
  while (itsVM.getState() != VirtualMachine::running) {
    if (itsVM.getState() == VirtualMachine::aborting) {
      exit(0); 
    }
    usleep(100); 
  }

  if (isHighestLevel()) {
    incrementEventCount();
  }

  bool onRightNode = true; // true if Node of this process == current rank

  if (getNode() < 0) {
    TRACER4("Simul::Process Node<0 " << getName());	  
    onRightNode = false;     
  }
	
  if (! shouldProcess()) {
    onRightNode = false;     
    TRACER4("Not on right node/appl; will skip Read & Write, and proceed substeps.");
  }

  if (onRightNode) {
    // Simuls run on ALL nodes.
    // Only read and write of the simul is on your own node
    for (int ch=0; ch < getWorker()->getInputs(); ch++) {
      // Read the source of inTransport
      Profiler::enterState (theirInReadProfilerState);
      getInTransport(ch).read();
      Profiler::leaveState (theirInReadProfilerState);
      // Write the InTransport to the first step of this simul
      Profiler::enterState (theirInWriteProfilerState);
      getInTransport(ch).write();
      Profiler::leaveState (theirInWriteProfilerState);
    }  
  }

  // Process all substeps (and simuls), even if this simul isn't running on this node 
  list<Step*>::iterator iList;
  for (iList = itsSteps.begin(); iList != itsSteps.end(); ++iList) {
    if ((*iList)->isSimul()) {
      TRACER4("Processing Simul " << (*iList)->getName());  
    } else {            // not a Simul but a step
      TRACER4("Processing step ID = " << (*iList)->getID());
    }
    (*iList)->process();
  }
  
  if (onRightNode) {
    for (int ch=0; ch < getWorker()->getOutputs(); ch++) {
      // Fill the outdata buffer by reading from last step
      Profiler::enterState (theirOutReadProfilerState);
      getOutTransport(ch).read();
      Profiler::leaveState (theirOutReadProfilerState);
      // Write the Outtransport
      Profiler::enterState (theirOutWriteProfilerState);
      getOutTransport(ch).write();
      Profiler::leaveState (theirOutWriteProfilerState);
    }  
  }
}

void SimulRep::postprocess()
{
  TRACER4("Simul " << getName() << " postprocess");
  StepRep::postprocess();
  list<Step*>::iterator iList;
  for (iList = itsSteps.begin(); iList != itsSteps.end(); ++iList) {
    (*iList)->postprocess();
  }
}

void SimulRep::dump() const
{
  TRACER4("Simul " << getName() << " dump");
  StepRep::dump();
  list<Step*>::const_iterator iList;
  for (iList = itsSteps.begin(); iList != itsSteps.end(); ++iList) {
    (*iList)->dump();
  }
}

bool SimulRep::isSimul() const
{
  return true;
}

bool SimulRep::setDHFile (const string& dhName, const string& fileName)
{
  int inxDH;
  StepRep* step;
  // Search in the output data holders.
  if (! splitName (true, dhName, step, inxDH)) {
    return false;
  }
  return step->getOutData(inxDH).setOutFile (fileName);
}
