//  BlockRep.cc:
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

#include <CEPFrame/BlockRep.h>
#include <CEPFrame/CompositeRep.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/DataManager.h>
#include <Transport/BaseSim.h>
#include TRANSPORTERINCLUDE
#include <Transport/TH_Mem.h>
#include <Transport/Connection.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_algorithm.h>    // for min,max

#include <sstream>
using namespace std;

namespace LOFAR
{

// this will give all instances of Block the same event in the Profiling output
unsigned int BlockRep::theirNextID=0;
unsigned int BlockRep::theirEventCnt=0;


BlockRep::BlockRep (const string& name,
		    bool addNameSuffix)
: itsRefCount   (1),
  itsParent     (0),
  itsID         (theirNextID),
  itsAddSuffix  (addNameSuffix),
  itsSeqNr      (-1),
  itsName       (name)
{
  setID();
  LOG_TRACE_RTTI_STR("Create Block " << name << " ID = " << getID());
}

BlockRep::~BlockRep() 
{}


bool BlockRep::isComposite() const
{
  return false;
}

void BlockRep::setSeqNr (int seqNr)
{
  itsSeqNr = seqNr;
  if (itsAddSuffix) {
    ostringstream os;
    os << '_' << itsSeqNr;
    itsName += os.str();
  }
}
       
bool BlockRep::connectData (const string& name,
			   TransportHolder* prototype,
			   DataManager& sourceDM, int sourceChannel, 
			   DataManager& targetDM, int targetChannel,
			   bool blockingComm)
{
  Connection* conn = new Connection(name, sourceDM.getGeneralOutHolder(sourceChannel), 
				    targetDM.getGeneralInHolder(targetChannel), 
				    prototype, blockingComm);
  sourceDM.setOutConnection(sourceChannel, conn);
  targetDM.setInConnection(targetChannel, conn);
  if (getParent() != 0)
  {
    getParent()->addConnection(conn);
  }
  else if (isComposite())
  {
    ((CompositeRep*)(this))->addConnection(conn);
  }
  else
  {
    THROW(LOFAR::Exception, "This Block " + getName() + " has not yet been added to a Composite. Do this before connecting.");
  }
  return true;
}


bool BlockRep::connectRep (int   thisDHIndex,
			   BlockRep* aBlock,
			   int   thatDHIndex,
			   int   nrDH,
			   TransportHolder* prototype,
			   bool blockingComm)
{
  // determine how much DataHolders to loop
  if (nrDH < 0) {
    nrDH = min(aBlock->getNrOutputs(),
	       this->getNrInputs());
  }
  bool result=true;
  bool firstConnect = true;
  TransportHolder* pTH = 0;
  for (int i=0; i<nrDH; i++) {
    int thisInx = i + thisDHIndex;  // DataHolder nr in this Block
    int thatInx = i + thatDHIndex;  // DataHolder nr in aBlock

    if (firstConnect)   // The first channel uses the prototype TransportHolder in 
    {                   // its connection. The others use a clone.
      pTH = prototype;
      firstConnect = false;
    }
    else
    {
      if (prototype->isClonable())
      {
	pTH = prototype->clone();
      }
      else
      {
	THROW(LOFAR::Exception, "TransportHolder type " + prototype->getType()
	      + " cannot be used in a connect method on multiple channels, " 
	      + "because it cannot be cloned. Try an individual connect() per channel, " 
	      + "each with its own instance of " + prototype->getType());
      }
    }

    result &= connectData (getName(), pTH,
                           aBlock->getOutDataManager(thatInx), 
			   aBlock->getOutChannelNumber(thatInx),
                           getInDataManager(thisInx), 
			   getInChannelNumber(thisInx),
                           blockingComm);

    LOG_TRACE_RTTI_STR( "BlockRep::connect " << getName().c_str() << " (ID = "
			<< getID() << ") DataHolder " << thisInx << " to "
			<< aBlock->getName().c_str() 
			<< " (ID = " << aBlock->getID() << ") DataHolder " 
			<< thatInx);
  }
  return result;
}

bool BlockRep::connectInput (Block* aBlock,
			     TransportHolder* prototype,
			     bool blockingComm)
{
  return connectRep (0, aBlock->getRep(), 0, -1, prototype, blockingComm);
}

bool BlockRep::connectInputArray (Block* aBlock[],
				 int   nrBlocks,
				 TransportHolder* prototype,
				 bool blockingComm)
{
  if (aBlock==NULL) return false;
  if (nrBlocks < 0) {  // set nrBlocks automatically
    nrBlocks = getNrInputs();
  }
  int dhIndex=0;
  bool firstConnect = true;
  TransportHolder* pTH = 0;
  for (int item=0; item<nrBlocks; item++) {
    ASSERTSTR (getNrInputs() >= 
	       dhIndex+aBlock[item]->getNrOutputs(),
	       "connect " << getName() << " - " << aBlock[item]->getName() <<
	       "; not enough inputs");

    if (firstConnect)   // The first Block in the array  uses the prototype 
    {                   // TransportHolder in its connection. The others use a clone.
      pTH = prototype;
      firstConnect = false;
    }
    else
    {
      if (prototype->isClonable())
      {
	pTH = prototype->clone();
      }
      else
      {
	THROW(LOFAR::Exception, "TransportHolder type " + prototype->getType()
	      + " cannot be used in a connect method on multiple steps, " 
	      + "because it cannot be cloned. Try an individual connect() per channel, " 
	      + "each with its own instance of " + prototype->getType());
      }
    }

    connectRep (dhIndex, aBlock[item]->getRep(), 0, -1, pTH,
		blockingComm);
    dhIndex += aBlock[item]->getNrOutputs();
  }
  if (dhIndex != getNrInputs()) {
    LOG_WARN_STR( "BlockRep::connectInputArray() - Warning:  " 
		  << getName() << " - " << aBlock[0]->getName() 
		  << ", unequal number of inputs and outputs");
  }
  return true;
}


bool BlockRep::connectOutputArray (Block* aBlock[],
				  int   nrBlocks,
				  TransportHolder* prototype,
				  bool blockingComm)
{
  if (aBlock == NULL) {
    return false;
  }
  if (nrBlocks < 0) {  // set nrBlocks automatically
    nrBlocks = getNrOutputs();
  }
  int dhIndex=0;
  bool firstConnect = true;
  TransportHolder* pTH = 0;
  for (int item=0; item<nrBlocks; item++) {

    ASSERTSTR (getNrOutputs() >= 
	       dhIndex+aBlock[item]->getNrInputs(),
	       "connect " << getName() << " - " << aBlock[item]->getName() <<
	       "; not enough inputs");
    if (firstConnect)   // The first Block in the array  uses the prototype 
    {                   // TransportHolder in its connection. The others use a clone.
      pTH = prototype;
      firstConnect = false;
    }
    else
    {
      if (prototype->isClonable())
      {
	pTH = prototype->clone();
      }
      else
      {
	THROW(LOFAR::Exception, "TransportHolder type " + prototype->getType()
	      + " cannot be used in a connect method on multiple steps, " 
	      + "because it cannot be cloned. Try an individual connect() per channel, " 
	      + "each with its own instance of " + prototype->getType());
      }
    }

    aBlock[item]->getRep()->connectRep (0, this, dhIndex, -1, pTH,
					blockingComm);
    dhIndex += aBlock[item]->getNrInputs();

  }
  if (dhIndex != getNrOutputs()) {
    LOG_WARN_STR("BlockRep::connectOutputArray() - Warning:  " 
		 << getName() << " - " << aBlock[0]->getName()
		 << ", unequal number of inputs and outputs");
  }
  return true;
}

// void BlockRep::replaceConnectionsWith(const TransportHolder& newTH,
// 				     bool blockingComm)
// {
//   LOG_TRACE_RTTI_STR("replaceConnectionsWith  " << newTH.getType());
//   for (int ch=0; ch<itsDataManager->getInputs(); ch++)
//   {
//     DataHolder* dh = itsDataManager->getGeneralInHolder(ch);
//     Transporter& transp = dh->getTransporter();
//     DataHolder* thatDH = transp.getSourceDataHolder();
//     if (thatDH)
//     {
//       LOG_TRACE_RTTI_STR("replace " << transp.getTransportHolder()->getType()
// 		<< " with " << newTH.getType());
//       dh->connectTo(*thatDH, newTH, blockingComm);
//     }
//   }
// }

}
