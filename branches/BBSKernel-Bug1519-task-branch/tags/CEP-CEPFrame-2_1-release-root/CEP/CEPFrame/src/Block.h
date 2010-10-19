//# Block.h: Base class of application blocks
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_CEPFRAME_BLOCK_H
#define LOFAR_CEPFRAME_BLOCK_H

// \file Block.h
// Base class of application blocks

//# Includes
#include <CEPFrame/BlockRep.h>
#include <Common/lofar_string.h>

namespace LOFAR
{

  // \addtogroup CEPFrame
  // @{

//# Forward Declarations
class Composite;
class DataManager;
class WorkHolder;
class TransportHolder;

/** The Block class is the base class for application building blocks. 
    It offers a common interface and common functionality for Steps and 
    Composites.
    The actual simulation work is performed in the process() method

    Note that the actual Block data is contained in the reference counted
    class BlockRep. In this way a copy of a Block is a cheap operation.
*/

class Block
{
public:
  /** Build the Block. The Block must get a unique name. To make that 
      process easy, by default the suffix _n is added to the name when 
      the Block is added to a Composite (where n is the sequence number 
      starting with 0).
  */

  /// Copy constructor (reference semantics).
  Block (const Block&);

  /// Construct a Block from a BlockRep (meant for internal use).
  explicit Block (BlockRep*);

  virtual ~Block();

  /// Assignment (reference semantics).
  Block& operator= (const Block&);

  /// Make a correct copy (reference semantics).
  virtual Block* clone() const = 0;

  /** The preprocess method is called before process.
  */
  void preprocess()
   { itsRep->preprocess(); }

  /** The process method is the basical simulation step.
      It will first read the data for the input DataHolders in the workholder.
      Then it will call Workholder->process().
      Finally it will write the data from the output DataHolders
      (e.g to another MPI process).
  */
  void process()
    { itsRep->process(); }

  /** The postprocess method is called after process.
  */
  void postprocess()
    { itsRep->postprocess(); }

  /// Dump information to user
  virtual void dump() const
  { itsRep->dump(); }

  /// Is the Block a Composite?
  virtual bool isComposite() const = 0;

  /// Get DataManager handling a specific in/output channel
  DataManager& getInDataManager(int channel);
  DataManager& getOutDataManager(int channel);

  /// Get the number of input/output channels
  int getNrInputs() const
  { return itsRep->getNrInputs(); }
  int getNrOutputs() const
  { return itsRep->getNrOutputs(); }


/*   /// Get WorkHolder. */
/*   virtual WorkHolder* getWorker() = 0;  //>>>>Necessary??? */

/*   /// Get i-th input DataHolder. */
/*   DataHolder& getInData (int dhIndex) */
/*     { return itsRep->getInData (dhIndex); } */
/*   /// Get i-th output DataHolder. */
/*   DataHolder& getOutData (int dhIndex) */
/*     { return itsRep->getOutData (dhIndex); } */

  /// Get ID of the Block.
  //  It will be obtained from the zeroth InData object in the Workholder
  int getID() const
    { return itsRep->getID(); }

  /// Get the node number for this Block.
  int getNode() const
    { return itsRep->getNode(); }

  /// Get the application number for this Block.
  int getAppl() const
    { return itsRep->getAppl(); }

  /**
     Set Node numbers for both In and OutData.
     Set application number as well.
     This is done recursively, thus executed for all Blocks in a Composite.
  */
  void runOnNode (int aNode, int applNr=0)
    { itsRep->runOnNode (aNode, applNr); }

  /// return this Block's name
  const string& getName() const
    { return itsRep->getName(); }

  /// set the Block's name
  void setName (const string& name)
    { itsRep->setName(name); }

  /// Get the parent Composite object.
  Composite getParent() const;

  /** Basic connect function.
      Connect nrDH output DataHolders of aBlock to input DataHolders of
      this Block. Start at the given indices.
      If nrDH=-1 is given, it will be set to the mininum number of
      DataHolders in input and output.
  */
  virtual bool connect (int thisDHIndex,
			Block* aBlock,
			int thatDHIndex,
			int nrDH,
			TransportHolder* prototype,
			bool blockingComm = true)
    { return itsRep->connectRep (thisDHIndex, aBlock->itsRep,
				 thatDHIndex, nrDH, prototype,
				 blockingComm); }
  
  /**
     Connect all output DataHolders of aBlock to the input DataHolders of
     this Block. The steps must have the same number of DataHolders.
     This is the normal way of connecting two Blocks to each other.
  */
  virtual bool connectInput (Block* aBlock,
		     TransportHolder* prototype,
		     bool blockingComm = true)
    { return itsRep->connectInput (aBlock, prototype, blockingComm); }

  /**
     Connect all output DataHolders in the array of Blocks (aBlock[]) to
     the input DataHolders of the current step. This is the normal way
     of connecting the output of multiple Blocks to a Composite object.
     If the nrBlocks argument is -1, it is assumed that
     each of the Blocks in the array has only one input DataHolder
     which will be connected to the output DataHolders of the Composite.
  */
  virtual bool connectInputArray (Block* aBlock[],   // pointer to array of ptrs to Blocks
			  int   nrBlocks,   // nr of Blocks in aBlock[] array
			  TransportHolder* prototype,
			  bool blockingComm = true)
    { return itsRep->connectInputArray (aBlock, nrBlocks, prototype,
					blockingComm); }

  /**
     Connect all input DataHolders in the array of Blocks (aBlock[]) to
     the output DataHolders of the current step. This is the normal
     way of connecting the output of a Composite object to Blocks.
     If the nrBlocks argument is -1, it is assumed that
     each of the Blocks in the array has only one output DataHolder
     which will be connected to the input DataHolders of the Composite.
  */
  virtual bool connectOutputArray (Block* aBlock[],   // pointer to array of ptrs to Blocks
			   int   nrBlocks,   // nr of Blocks in aBlock[] array
			   TransportHolder* prototype,
			   bool blockingComm=true)
    { return itsRep->connectOutputArray (aBlock, nrBlocks, prototype,
					 blockingComm); }

/*   /\** Replace the connections with using a specified TransportHolder. */
/*   *\/ */
/*   void replaceConnectionsWith(const TransportHolder& newTH,  */
/* 			      bool blockingComm=true) */
/*     { return itsRep->replaceConnectionsWith(newTH, blockingComm); } */

  /** Set properties of a communication channel: synchronisity and sharing of DataHolders
      by input and output.
  */
  virtual void setInBufferingProperties(int channel, bool synchronous, 
                                bool shareDHs=false)
    { return itsRep->setInBufferingProperties(channel, synchronous, shareDHs);}

  virtual void setOutBufferingProperties(int channel, bool synchronous, 
                                 bool shareDHs=false)
    { return itsRep->setOutBufferingProperties(channel, synchronous, shareDHs);}

  /** SetRate methods:
      These methods set the rate at which data is processed and input/output 
      dataholders are read/written.
      The rate is the fraction of the events that is to be read/written.
      The dhIndex argument selects only the given input/output
      DataHolder; dhIndex=-1 sets all input/output DataHolders.
  */
  void setProcessRate (int rate=1)
    { return itsRep->setProcessRate (rate); }
  void setInRate (int rate=1, int dhIndex=-1)
    { return itsRep->setInRate (rate, dhIndex); }
  void setOutRate (int rate=1, int dhIndex=-1)
    { return itsRep->setOutRate (rate, dhIndex); }

   /// Get the event count.
  static unsigned int getEventCount()
    { return BlockRep::getEventCount(); }

  /// Clear the event count.
  static void clearEventCount()
    { BlockRep::clearEventCount(); }

  /// Get the current application number.
  static int getCurAppl()
    { return BlockRep::getCurAppl(); } 

  /// Set the current application number.
  static void setCurAppl (int applNr)
    { BlockRep::setCurAppl (applNr); } 

protected:
  /// Default constructor for derived class.
  Block()
    : itsRep(0) {}

  /// Get the internal rep object (to be used by BlockRep and CompositeRep).
  friend class BlockRep;
  friend class CompositeRep;
  BlockRep* getRep()
    { return itsRep; }
  const BlockRep* getRep() const
    { return itsRep; }

  BlockRep* itsRep;

};

inline DataManager& Block::getInDataManager(int channel)  
  { return itsRep->getInDataManager(channel); }

inline DataManager& Block::getOutDataManager(int channel)
  { return itsRep->getOutDataManager(channel); }

// @}
}

#endif
