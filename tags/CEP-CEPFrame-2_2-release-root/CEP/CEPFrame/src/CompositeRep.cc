///  CompositeRep.cc:
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

#include <lofar_config.h>

#include <CEPFrame/CompositeRep.h>
#include <CEPFrame/Composite.h>
#include <CEPFrame/VirtualMachine.h>
#include <Transport/Connection.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_algorithm.h>    // for min,max
#include <unistd.h>

namespace LOFAR
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CompositeRep::CompositeRep (int nrInputs,
			    int nrOutputs,
			    const string& name,
			    bool addNameSuffix)
: BlockRep         (name, addNameSuffix),
  itsIsHighestLevel(true),
  itsNrInputs      (nrInputs),
  itsNrOutputs     (nrOutputs),
  itsNode          (0),
  itsAppl          (0)
{
  LOG_TRACE_FLOW("CompositeRep C'tor");
 
  itsInputs.resize(nrInputs);
  itsOutputs.resize(nrOutputs);
}

CompositeRep::~CompositeRep()
{
  for (list<Block*>::iterator iter=itsBlocks.begin();
       iter!=itsBlocks.end(); iter++) {
    delete *iter;
  }
  for (list<Connection*>::iterator it=itsConnections.begin();
       it!=itsConnections.end(); it++)
  {
    TransportHolder* th = (*it)->getTransportHolder();
    LOG_TRACE_FLOW_STR("Deleting connection " << (*it)->getTag());
    delete th;
    delete *it;
  }

  itsInputs.clear();
  itsOutputs.clear();
}

void CompositeRep::addBlock (const Block& aBlock)
{
  LOG_TRACE_FLOW_STR("Composite::addBlock " << aBlock.getName());
  // Error if the step is already used in a simul.
  ASSERTSTR (aBlock.getRep()->getParent() == 0,
 	     "Block " << aBlock.getName() 
 	     << " already used in another composite");
  // Make a copy of the Block object.
  // Note that the underlying BlockRep is shared (reference counted).
  Block* block = aBlock.clone();
  BlockRep* stepPtr = block->getRep();
  // Set the sequence number (which might change the name).
  stepPtr->setSeqNr (itsBlocks.size());
  // Put the copy of the step in list.
  itsBlocks.push_back (block);
  // Tell that the step has been added to a Composite.
  stepPtr->setParent (*this);
  if (stepPtr->isComposite()) {
    CompositeRep* simulPtr = dynamic_cast<CompositeRep*>(stepPtr);
    // update the ishighestlevel flag of the aBlock
    simulPtr->setNotHighestLevel();
  }
}

void CompositeRep::addConnection(Connection* conn)
{
  LOG_TRACE_FLOW_STR("Composite::addConnection " << conn->getTag());
  itsConnections.push_back(conn);
}

void CompositeRep::runOnNode (int aNode, int applNr)
{
  itsNode = aNode;
  itsAppl = applNr;
  for (list<Block*>::iterator iter=itsBlocks.begin();
       iter!=itsBlocks.end(); iter++) {
    (*iter)->runOnNode (aNode, applNr);
  }
}

void CompositeRep::setInput (int thisIndex, 
			     Block* aBlock, 
			     int thatIndex, 
			     int nrChan)
{
  DBGASSERTSTR (thisIndex >= 0,          "Channel index is too low");
  DBGASSERTSTR (thisIndex < getNrInputs(), "Channel index is too high");
  DBGASSERTSTR (nrChan <= aBlock->getNrInputs() && nrChan <= getNrInputs(), 
		"nrChan is too high");

  // Determine how many channels to loop
  if (nrChan < 0) {
    nrChan = min(aBlock->getNrInputs(), this->getNrInputs());
  }

  for (int i=0; i<nrChan; i++) 
  {
    int thisInx = thisIndex + i;  // Channel nr in this Composite
    int thatInx = thatIndex + i;  // Channel nr in aBlock

    DBGASSERTSTR(itsInputs[thisInx].block == 0 && itsInputs[thisInx].chanNr == -1,
		 "Input" << thisInx << " has already been set.");
    itsInputs[thisInx].block = aBlock;
    itsInputs[thisInx].chanNr = thatInx;
  }
}
  
void CompositeRep::setOutput (int thisIndex, 
			      Block* aBlock, 
			      int thatIndex, 
			      int nrChan)
{
  ASSERTSTR (thisIndex >= 0,          "Channel index is too low");
  ASSERTSTR (thisIndex < getNrOutputs(), "Channel index is too high");
  DBGASSERTSTR (nrChan <= aBlock->getNrOutputs() && nrChan <= getNrOutputs(), 
		"nrChan is too high");

  // Determine how many channels to loop
  if (nrChan < 0) {
    nrChan = min(aBlock->getNrOutputs(), this->getNrOutputs());
  }

  for (int i=0; i<nrChan; i++) 
  {
    int thisInx = thisIndex + i;  // DataHolder nr in this Composite
    int thatInx = thatIndex + i;  // DataHolder nr in aBlock

    DBGASSERTSTR(itsOutputs[thisInx].block == 0 && itsOutputs[thisInx].chanNr == -1,
		 "Output " << thisInx << " has already been set.");
    itsOutputs[thisInx].block = aBlock;
    itsOutputs[thisInx].chanNr = thatInx;
  }
}


void CompositeRep::setInputArray (Block* aBlock[],    // array of ptrs to Blocks
				  int    nrBlocks)    // nr of Blocks in aBlock[] array
{
  LOG_TRACE_FLOW_STR("setInputArray " 
		     << getName() << " "
		     << aBlock[0]->getName());
  DBGASSERT(aBlock!=NULL);

  if (nrBlocks < 0) {  // set nrBlocks automatically
    nrBlocks = getNrInputs();
  }
  int dhIndex=0;

  for (int item=0; item<nrBlocks; item++) {
    ASSERTSTR (getNrInputs() >= 
               dhIndex+aBlock[item]->getNrOutputs(),
               "setInputArray " << getName() << " - " << aBlock[item]->getName() <<
               "; not enough inputs");

    setInput(dhIndex, aBlock[item], 0, -1);
    dhIndex += aBlock[item]->getNrOutputs();
  }
  if (dhIndex != getNrInputs()) {
    LOG_WARN_STR("CompositeRep::setInputArray() - Warning:  " << 
		 getName() << " - " << aBlock[0]->getName() 
		 << ", unequal number of inputs and outputs");
  }
}

void CompositeRep::setOutputArray (Block* aBlock[],  // array of ptrs to Blocks
				   int    nrBlocks)  // nr of Blocks in aBlock[] array
{
  LOG_TRACE_FLOW_STR( "setOutputArray " 
		      << getName() << " "
		      << aBlock[0]->getName());
  DBGASSERT(aBlock==NULL);

  if (nrBlocks < 0) {  // set nrBlocks automatically
    nrBlocks = getNrOutputs();
  }
  int dhIndex=0;

  for (int item=0; item<nrBlocks; item++) {

    ASSERTSTR (getNrOutputs() >= 
               dhIndex+aBlock[item]->getNrInputs(),
               "setOutputArray " << getName() << " - " << aBlock[item]->getName() <<
               "; not enough inputs");

    setOutput(dhIndex, aBlock[item], 0, -1);

    dhIndex += aBlock[item]->getNrInputs();

  }
  if (dhIndex != getNrOutputs()) {
    LOG_WARN_STR("CompositeRep::setOutputArray() - Warning:   " 
		 << getName() << " - " << aBlock[0]->getName()
		 << ", unequal number of inputs and outputs");
  }
}

// void CompositeRep::replaceConnectionsWith(const TransportHolder& newTH, bool blockingComm)
// {
//   // Do a simplifyComm for this Composite.
//   BlockRep::replaceConnectionsWith(newTH, blockingComm);
//   // Do the same for all steps in this Composite.
//   for (list<Block*>::iterator iter=itsBlocks.begin();
//        iter!=itsBlocks.end(); iter++) {
//     (*iter)->replaceConnectionsWith(newTH, blockingComm);
//   }
// }

void CompositeRep::preprocess()
{
  LOG_TRACE_FLOW_STR("Composite " << getName() << " preprocess");
  list<Block*>::iterator iList;
  for (iList = itsBlocks.begin(); iList != itsBlocks.end(); ++iList) {
    (*iList)->preprocess();
  }
}

void CompositeRep::process()
{
  /// Process all contained steps

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
	
  // Process all substeps (and composites), even if this composite isn't running on this node 
  list<Block*>::iterator iList;
  for (iList = itsBlocks.begin(); iList != itsBlocks.end(); ++iList) {
    if ((*iList)->isComposite()) {
      LOG_TRACE_RTTI_STR("Processing Composite " << (*iList)->getName());  
    } else {            // not a Composite but a step
      LOG_TRACE_RTTI_STR("Processing step ID = " << (*iList)->getID());
    }
    (*iList)->process();
  }
 
}

void CompositeRep::postprocess()
{
  LOG_TRACE_FLOW_STR("Composite " << getName() << " postprocess");
  list<Block*>::iterator iList;
  for (iList = itsBlocks.begin(); iList != itsBlocks.end(); ++iList) {
    (*iList)->postprocess();
  }
}

void CompositeRep::dump() const
{
  LOG_TRACE_FLOW_STR("Composite " << getName() << " dump");
  list<Block*>::const_iterator iList;
  for (iList = itsBlocks.begin(); iList != itsBlocks.end(); ++iList) {
    (*iList)->dump();
  }
}

bool CompositeRep::isComposite() const
{
  return true;
}

DataManager& CompositeRep::getInDataManager(int channel)
{
  DBGASSERTSTR (channel >= 0,        "Channel index is too low");
  DBGASSERTSTR (channel < getNrInputs(), "Channel index is too high");
  Block* blockPtr = itsInputs[channel].block;
  ASSERTSTR(blockPtr!=0, "Input channel " << channel 
	    << " has not been set in this Composite.")
  int index = itsInputs[channel].chanNr;
  ASSERTSTR(index>=0,"Input channel " << channel 
	    << " has not been correctly set in this Composite.")  
  return blockPtr->getInDataManager(index);
}

DataManager& CompositeRep::getOutDataManager(int channel)
{
  DBGASSERTSTR (channel >= 0,        "Channel index is too low");
  DBGASSERTSTR (channel < getNrOutputs(), "Channel index is too high");
  Block* blockPtr = itsOutputs[channel].block;
  ASSERTSTR(blockPtr!=0, "Output channel " << channel 
	    << " has not been set in this Composite.")
  int index = itsOutputs[channel].chanNr;
  ASSERTSTR(index>=0,"Output channel " << channel 
	    << " has not been correctly set in this Composite.")  
  return blockPtr->getOutDataManager(index);
}

int CompositeRep::getInChannelNumber(int channel)
{
  DBGASSERTSTR (channel >= 0,        "Channel index is too low");
  DBGASSERTSTR (channel < getNrInputs(), "Channel index is too high");
  Block* blockPtr = itsInputs[channel].block;
  ASSERTSTR(blockPtr!=0, "Input channel " << channel 
	    << " has not been set in this Composite.");
  int index = itsInputs[channel].chanNr;
  ASSERTSTR(index>=0,"Input channel " << channel 
	    << " has not been correctly set in this Composite.");
  return blockPtr->getRep()->getInChannelNumber(index);
}

int CompositeRep::getOutChannelNumber(int channel)
{
  DBGASSERTSTR (channel >= 0,        "Channel index is too low");
  DBGASSERTSTR (channel < getNrOutputs(), "Channel index is too high");
  Block* blockPtr = itsOutputs[channel].block;
  ASSERTSTR(blockPtr!=0, "Output channel " << channel 
	    << " has not been set in this Composite.");
  int index = itsOutputs[channel].chanNr;
  ASSERTSTR(index>=0,"Output channel " << channel 
	    << " has not been correctly set in this Composite.");  
  return blockPtr->getRep()->getOutChannelNumber(index);
}

int CompositeRep::getAppl() const
{
  return itsAppl;
}

void CompositeRep::setProcessRate (int rate)
{
  list<Block*>::iterator iter;
  for (iter = itsBlocks.begin(); iter != itsBlocks.end(); ++iter)
  {
    (*iter)->setProcessRate(rate);
  }
}

void CompositeRep::setInRate(int rate, int dhIndex)
{
  if (dhIndex == -1)
  {
    for (int i=0; i<getNrInputs(); i++)
    {
      Block* blockPtr = itsInputs[i].block;
      ASSERTSTR(blockPtr!=0, "Input channel " << i 
		<< " has not been set in this Composite.");
      int index = itsInputs[i].chanNr;
      ASSERTSTR(index>=0,"Input channel " << dhIndex 
		<< " has not been correctly set in this Composite."); 
      blockPtr->getRep()->setInRate(rate, index);
    }
  }
  else
  {
    DBGASSERTSTR (dhIndex >= 0,        "Channel index is too low");
    DBGASSERTSTR (dhIndex < getNrInputs(), "Channel index is too high");
    Block* blockPtr = itsInputs[dhIndex].block;
    ASSERTSTR(blockPtr!=0, "Input channel " << dhIndex 
	      << " has not been set in this Composite.");
    int index = itsInputs[dhIndex].chanNr;
    ASSERTSTR(index>=0,"Input channel " << dhIndex 
	      << " has not been correctly set in this Composite."); 
    blockPtr->getRep()->setInRate(rate, index);
  }
}

void CompositeRep::setOutRate (int rate, int dhIndex)
{
  if (dhIndex == -1)
  {
    for (int i=0; i<getNrInputs(); i++)
    {
      Block* blockPtr = itsInputs[i].block;
      ASSERTSTR(blockPtr!=0, "Input channel " << i 
		<< " has not been set in this Composite.");
      int index = itsInputs[i].chanNr;
      ASSERTSTR(index>=0,"Input channel " << dhIndex 
		<< " has not been correctly set in this Composite."); 
      blockPtr->getRep()->setInRate(rate, index);
    }
  }
  else
  {
    DBGASSERTSTR (dhIndex >= 0,        "Channel index is too low");
    DBGASSERTSTR (dhIndex < getNrInputs(), "Channel index is too high");
    Block* blockPtr = itsInputs[dhIndex].block;
    ASSERTSTR(blockPtr!=0, "Input channel " << dhIndex 
	      << " has not been set in this Composite.");
    int index = itsInputs[dhIndex].chanNr;
    ASSERTSTR(index>=0,"Input channel " << dhIndex 
	      << " has not been correctly set in this Composite."); 
    blockPtr->getRep()->setInRate(rate, index);
  }
}

}
