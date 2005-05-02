//#  CompositeRep.cc:
//#
//#  Copyright (C) 2000, 2001
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <CEPFrame/CompositeRep.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/Composite.h>
#include <CEPFrame/WH_Empty.h>
#include <CEPFrame/VirtualMachine.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_algorithm.h>    // for min,max
#include <unistd.h>

#ifdef HAVE_CORBA
#include <Transport/Corba/BS_Corba.h>
#include <Transport/Corba/CorbaController.h>
#endif 

namespace LOFAR
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CompositeRep::CompositeRep (WorkHolder& worker, 
		    const string& name,
		    bool addNameSuffix, 
		    bool controllable,
		    bool monitor)
: StepRep          (worker, name, addNameSuffix, monitor),
  itsIsHighestLevel(true),
  itsController    (0)
{
  LOG_TRACE_FLOW("CompositeRep C'tor");
  if (controllable) {
    LOG_TRACE_RTTI_STR("Create controllable Composite " << name);
#ifdef HAVE_CORBA
    // Create a CorbaController object and connect it to the VirtualMachine.
    itsController = new CorbaController (BS_Corba::getPOA(),
					 BS_Corba::getPOAManager(),
					 name, 
					 &itsVM);
#else
    LOG_INFO("CORBA is not configured, so CorbaMonitor cannot be used in Composite ");
#endif 
  }
}

CompositeRep::~CompositeRep()
{
  for (list<Step*>::iterator iter=itsSteps.begin();
       iter!=itsSteps.end(); iter++) {
    delete *iter;
  }
#ifdef HAVE_CORBA
  delete itsController;
#endif
}

void CompositeRep::addStep (const Step& step)
{
  LOG_TRACE_FLOW_STR("Composite::addStep " << step.getName());
  // Error if the step is already used in a simul.
  ASSERTSTR (step.getRep()->getParent() == 0,
	     "Step " << step.getName() << " already used in another simul");
  // Make a copy of the Step object.
  // Note that the underlying StepRep is shared (reference counted).
  Step* aStep = step.clone();
  StepRep* stepPtr = aStep->getRep();
  // Set the sequence number (which might change the name).
  stepPtr->setSeqNr (itsSteps.size());
  // Error if the step name is already used.
  ASSERTSTR (itsNameMap.find(stepPtr->getName()) == itsNameMap.end(),
	     "Step name '%s' already used in this simul" <<
	     step.getName());
  // Put the copy of the step in list.
  itsSteps.push_back (aStep);
  // Add the name to the map.
  itsNameMap[stepPtr->getName()] = stepPtr;
  // Tell that the step has been added to a Composite.
  stepPtr->setParent (*this);
  if (stepPtr->isComposite()) {
    CompositeRep* simulPtr = dynamic_cast<CompositeRep*>(stepPtr);
    // update the ishighestlevel flag of the aStep
    simulPtr->setNotHighestLevel();
  }
}

void CompositeRep::runOnNode (int aNode, int applNr)
{
  StepRep::runOnNode (aNode, applNr);
  for (list<Step*>::iterator iter=itsSteps.begin();
       iter!=itsSteps.end(); iter++) {
    (*iter)->runOnNode (aNode, applNr);
  }
}


bool CompositeRep::connect (const string& sourceName, const string& targetName,
			    const TransportHolder& prototype, bool blockingComm)
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
  ASSERTSTR (sourceStep != targetStep,
	     "Attempt to connect Step/Composite " <<
	     sourceStep->getName() << " to itself");
  if (sourceStep == this) {
    if (sourceDH < 0  &&  targetDH < 0) {
      Step tgt(targetStep);
      return connect_thisIn_In (&tgt, 0, 0, 0, prototype, blockingComm);
    } else {
      return connectData (prototype,
			  sourceStep->getInData(max(0,sourceDH)),
			  targetStep->getInData(max(0,targetDH)),
			  blockingComm);
    }
  } else if (targetStep == this) {
    if (sourceDH < 0  &&  targetDH < 0) {
      Step src(sourceStep);
      connect_thisOut_Out (&src, 0, 0, 0, prototype, blockingComm);
    } else {
      return connectData (prototype,
			  sourceStep->getOutData(max(0,sourceDH)),
			  targetStep->getOutData(max(0,targetDH)),
			  blockingComm);
    }
  } else {
    // If no DataHolders given, connect all of them in the steps.
    // Otherwise connect given DataHolder. Take first if not given.
    if (sourceDH < 0  &&  targetDH < 0) {
      Step src(sourceStep);
      return targetStep->connectInput (&src, prototype, blockingComm);
    } else {
      return targetStep->connectRep (sourceStep, max(0,targetDH),
				     max(0,sourceDH), 1, prototype,
				     blockingComm);
    }
  }
  return false;
}

bool CompositeRep::splitName (bool isSource, const string& name,
			  StepRep*& step, int& dhIndex)
{
  dhIndex = -1;
  step = 0;
  // Determine if we have to take the input or output DataHolder.
  bool takeOut = isSource;
  // Split the name at the .
  int i = name.find('.');
  // Find the step part and object. No step part means this Composite.
  if (i == 0) {
    step = this;
    takeOut = !isSource;
  } else {
    string stepName = name;
    if (i > 0) {
      stepName = name.substr (0, i);
    }
    ASSERTSTR (itsNameMap.find(stepName) != itsNameMap.end(),
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
  // - is it this Composite or a Step in the Composite.
  if (! dhName.empty()) {
    string type = "Output";
    if (takeOut) {
      dhIndex = step->getWorker()->getOutChannel(dhName);
    } else {
      type = "Input";
      dhIndex = step->getWorker()->getInChannel(dhName);
    }
    ASSERTSTR (dhIndex >= 0,
	       type << " DataHolder " << dhName <<
	       " unknown in step/simul " << step->getName());
  }
  return true;
}


bool CompositeRep::connect_thisOut_Out (Step* aStep,          
				    int   thisChannelOffset,
				    int   thatChannelOffset,
				    int   skip,
				    const TransportHolder& prototype,
				    bool blockingComm)
{
  if (aStep == NULL) {
    return false;
  }
  // determine how much channels to loop
  int loopSize = min(aStep->getWorker()->getDataManager().getOutputs(),
		     this->getWorker()->getDataManager().getOutputs());
  for (int channel=0; channel < loopSize ; channel += skip+1) {
    int thischannel = channel+thisChannelOffset;   // channel nr in this Step
    int thatchannel = channel+thatChannelOffset;   // channel nr in aStep
    
    connectData (prototype,
		 aStep->getOutData(thatchannel),
		 this->getOutData(thischannel),
		 blockingComm);

    LOG_TRACE_RTTI_STR("connect_OutOut; Connect " << getName() << "(ID = "
	   << getID() << " ) channel " << thischannel << "to : ("
	   << aStep->getRep()->getID() << ") OutTransport ID = " 
	   << aStep->getOutData(thatchannel).getID() << " ");  
  }
  return true;
}

bool CompositeRep::connect_thisIn_In (Step* aStep,          
				  int   thisChannelOffset,
				  int   thatChannelOffset,
				  int   skip,
				  const TransportHolder& prototype,
				  bool blockingComm)
{
  if (aStep == NULL) {
    return false;
  }

  // determine how much channels to loop
  int loopSize = min(aStep->getWorker()->getDataManager().getInputs(),
		     this->getWorker()->getDataManager().getInputs());
  for (int channel=0; channel < loopSize ; channel += skip+1) {
    int thischannel = channel+thisChannelOffset;  // channel nr in this Step
    int thatchannel = channel+thatChannelOffset;  // channel nr in aStep
    connectData (prototype,
		 this->getInData(thischannel),
		 aStep->getInData(thatchannel),
		 blockingComm);

    LOG_TRACE_RTTI_STR( "connect_thisIn_In; Connect " << getName() 
			<< "(ID = " << getID() 
	   << " ) channel " << thischannel << " : InTransport InID = " 
	   << aStep->getInData(thatchannel).getID() << " ");  
  }
  return true;
}


bool CompositeRep::connectInputToArray (Step* aStep[],   // pointer to  array of ptrs to Steps
				 int    nrItems, // nr of Steps in aStep[] array
				 int    skip,     // skip in inputs in aStep 
				 int    offset,   // start with this input nr in aStep
				 const TransportHolder& prototype,
				 bool blockingComm)
{

  LOG_TRACE_FLOW_STR("connectInputToArray " 
		     << getName() << " "
		     << aStep[0]->getName());
  if (aStep==NULL) return false;
  int channelOffset=0;
  for (int item=offset; item<nrItems; item ++) {
    ASSERTSTR(getWorker()->getDataManager().getInputs()  
	    >=  channelOffset + aStep[item]->getWorker()->getDataManager().getInputs() - skip,
		 "Step::connectInputToArray not enough inputs");
    connect_thisIn_In (aStep[item],
		       channelOffset, // offset in this
		       offset,
		       skip,         // offset in aStep
		       prototype,
		       blockingComm);

    channelOffset += aStep[item]->getWorker()->getDataManager().getInputs() - skip;
  }
  return true;
  
}

bool CompositeRep::connectOutputToArray (Step* aStep[],  // array of ptrs to Steps
				  int    nrItems,
				  int    skip,    // skip in inputs in aStep 
				  int    offset,  // start with this input nr in aStep
				  const TransportHolder& prototype,
				  bool blockingComm)
{ // nr of Steps in aStep[] array
  
  LOG_TRACE_FLOW_STR( "connectOutputToArray " 
		      << getName() << " "
		      << aStep[0]->getName());
  if (aStep==NULL) return false;
  int channelOffset=0;
  for (int item=0; item<nrItems; item++) {
    ASSERTSTR (getWorker()->getDataManager().getOutputs() >=
	       channelOffset + aStep[item]->getWorker()->getDataManager().getOutputs(),
	       "Step::connectOutputToArray not enough outputs");
    connect_thisOut_Out (aStep[item],
			 channelOffset, // offset in this
			 offset,
			 skip,
			 prototype,
			 blockingComm);
    channelOffset += aStep[item]->getWorker()->getDataManager().getOutputs() - skip;
  }
  return true;
}

void CompositeRep::replaceConnectionsWith(const TransportHolder& newTH, bool blockingComm)
{
  // Do a simplifyComm for this Composite.
  StepRep::replaceConnectionsWith(newTH, blockingComm);
  // Do the same for all steps in this Composite.
  for (list<Step*>::iterator iter=itsSteps.begin();
       iter!=itsSteps.end(); iter++) {
    (*iter)->replaceConnectionsWith(newTH, blockingComm);
  }
}

void CompositeRep::preprocess()
{
  LOG_TRACE_FLOW_STR("Composite " << getName() << " preprocess");
  StepRep::preprocess();
  list<Step*>::iterator iList;
  for (iList = itsSteps.begin(); iList != itsSteps.end(); ++iList) {
    (*iList)->preprocess();
  }
}

void CompositeRep::process()
{
  /// 1) Read InData
  /// 2) process all steps
  /// 3) Write OutData

  // The VirtualMachine controls the state of this CompositeRep.
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
    LOG_TRACE_RTTI_STR("Composite::Process Node<0 " << getName());	  
    onRightNode = false;     
  }
	
  if (! (getWorker()->shouldProcess())) {
    onRightNode = false;     
    LOG_TRACE_RTTI("Not on right node/appl; will skip Read & Write, and proceed substeps.");
  }

  if (onRightNode) {
    // Composites run on ALL nodes.
    // Only read and write of the simul is on your own node
    for (int ch=0; ch < getWorker()->getDataManager().getInputs(); ch++) {
      // Read the source of inTransport
      getWorker()->getDataManager().getInHolder(ch);
      getWorker()->getDataManager().readyWithInHolder(ch);
      // Write the InTransport to the first step of this simul
      (getWorker()->getDataManager().getInHolder(ch))->write();
    }  
  }

  // Process all substeps (and simuls), even if this simul isn't running on this node 
  list<Step*>::iterator iList;
  for (iList = itsSteps.begin(); iList != itsSteps.end(); ++iList) {
    if ((*iList)->isComposite()) {
      LOG_TRACE_RTTI_STR("Processing Composite " << (*iList)->getName());  
    } else {            // not a Composite but a step
      LOG_TRACE_RTTI_STR("Processing step ID = " << (*iList)->getID());
    }
    (*iList)->process();
  }
  
  if (onRightNode) {
    for (int ch=0; ch < getWorker()->getDataManager().getOutputs(); ch++) {
      // Fill the outdata buffer by reading from last step
      getWorker()->getDataManager().getOutHolder(ch)->read();
      // Write the Outtransport
      getWorker()->getDataManager().readyWithOutHolder(ch);
    }  
  }
}

void CompositeRep::postprocess()
{
  LOG_TRACE_FLOW_STR("Composite " << getName() << " postprocess");
  StepRep::postprocess();
  list<Step*>::iterator iList;
  for (iList = itsSteps.begin(); iList != itsSteps.end(); ++iList) {
    (*iList)->postprocess();
  }
}

void CompositeRep::dump() const
{
  LOG_TRACE_FLOW_STR("Composite " << getName() << " dump");
  StepRep::dump();
  list<Step*>::const_iterator iList;
  for (iList = itsSteps.begin(); iList != itsSteps.end(); ++iList) {
    (*iList)->dump();
  }
}

bool CompositeRep::isComposite() const
{
  return true;
}


}
