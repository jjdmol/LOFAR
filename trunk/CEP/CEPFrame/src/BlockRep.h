//# BlockRep.h: Base class of a application building block
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

#ifndef LOFAR_CEPFRAME_BLOCKREP_H
#define LOFAR_CEPFRAME_BLOCKREP_H

// \file BlockRep.h
// Base class of application building block, actual implementation

//#include <Transport/TransportHolder.h>
#include <stdlib.h>
//#include <Common/lofar_string.h>
#include <tinyCEP/WorkHolder.h>
#include <CEPFrame/DataManager.h>

namespace LOFAR
{

// \addtogroup CEPFrame
// @{

class Block;
class TransportHolder;
class CompositeRep;

/** The Block class is the basic building block for applications.
    The actual simulation work is performed in the process() method
    (which calls the Workholder::baseProcess() method)
*/

class BlockRep
{
public:
  /** Normally used basic constructor
      a pointer to a workholder (containing the dataholders) is passed 
  */
  BlockRep (const string& name,
	    bool addNameSuffix);

  virtual ~BlockRep();

  /// Increment reference count.
  void incrRefCount();

  /// Decremented reference count and return decremented count.
  int decrRefCount();

  /** The preprocess method is called before process.
      It can be used to initialize WorkHolders, etc.
  */
  virtual void preprocess() = 0;;

  /** The process method is the basical simulation step.
      It will first read the data for the in DataHolders in the workholder.
      Then it will call Workholder->process().
      Finally it will write the data from the DataHolders
      (e.g to another MPI process).
  */
  virtual void process() = 0;

  /** The postprocess method is called after process.
      It can be used to clean up WorkHolders, etc.
  */
  virtual void postprocess() = 0;

  /// Dump information to user
  virtual void dump() const = 0;

  /// This methods distinguishes between BlockRep and CompositeRep class.
  virtual bool isComposite() const;

  /// Get DataManager handling a specific in/output channel
  virtual DataManager& getInDataManager(int channel) = 0;
  virtual DataManager& getOutDataManager(int channel) = 0;

  /// Get the channel number in the actual DataManager of an input/output channel.
  virtual int getInChannelNumber(int channel) = 0;
  virtual int getOutChannelNumber(int channel) = 0;

  /// Get the number of input/output channels
  virtual int getNrInputs() const = 0;
  virtual int getNrOutputs() const = 0;

/*   /// Get i-th input DataHolder. */
/*   DataHolder& getInData (int dhIndex); */
/*   /// Get i-th output DataHolder. */
/*   DataHolder& getOutData (int dhIndex); */

  /// Get ID of the BlockRep.
  int getID() const;  

  /// Get the node number for this BlockRep.
  virtual int getNode() const = 0; 

  /// Get the application number for this BlockRep.
  virtual int getAppl() const = 0; 

  /** Set Node numbers for both In and OutData
      Set application number as well.
  */
  virtual void runOnNode(int aNode, int applNr) = 0;

  /// return this BlockRep's name
  const string& getName() const;

  /// set the Block's name
  void setName (const string& name);

  /** Get the parent simul object.
      0 means that the Block is not used in a Composite.
  */
  CompositeRep* getParent() const;

  /// Set the parent simul object.
  void setParent (CompositeRep& parent);

  /** Get the sequence number of the Block in its parent Composite. 
      -1 means thar the Block is not used in a Composite.
  */
  int getSeqNr() const;

  /** Set the sequence number.
      If needed, it is also added as a suffix to the name.
  */
  void setSeqNr (int seqNr);

  /// Basic helper function for ConnectXXXX methods.
  bool connectRep (int thisDHIndex,
		   BlockRep* aBlock, 
		   int thatDHIndex,
		   int nrDH,
		   TransportHolder* prototype,
		   bool blockingComm);
  
  /**
     Connect the output DataHolders of aBlock to the input DHs of the current
     Block. This is the normal way of connecting two Blocks to each other.
  */
  bool connectInput (Block* aBlock,
		     TransportHolder* prototype,
		     bool blockingComm);

  /**
     Connect all output DataHolders in the array of Blocks (aBlock[]) to
     the input DataHolders of the current step. This is the normal way
     of connecting the output of multiple Blocks to a Composite object.
     If the nrBlocks argument is -1, it is assumed that
     each of the Blocks in the array has only one input DataHolder
     which will be connected to the output DataHolders of the Composite.
  */
  bool connectInputArray (Block* aBlock[], // pointer to array of ptrs to Blocks
			  int   nrBlocks, // nr of Blocks in aBlock[] array
			  TransportHolder* prototype,
			  bool blockingComm);

  /**
     Connect all input DataHolders in the array of Blocks (aBlock[]) to
     the output DataHolders of the current step. This is the normal
     way of connecting the output of a Composite object to Blocks.
     If the nrBlocks argument is -1, it is assumed that
     each of the Blocks in the array has only one output DataHolder
     which will be connected to the input DataHolders of the Composite.
  */
  bool connectOutputArray (Block* aBlock[], // pointer to array of ptrs to Blocks
			   int   nrBlocks, // nr of Blocks in aBlock[] array
			   TransportHolder* prototype,
			   bool blockingComm);


 /** Optimize connections by replacing a TransportHolder with
      a possibly more efficient TransportHolder (newTH)
  */
/*   virtual void replaceConnectionsWith(const TransportHolder& newTH, */
/* 				      bool blockingComm); */
 
  // Set properties of a communication channel: synchronisity and sharing of DataHolders
  // by input and output
  void setInBufferingProperties(int channel, bool synchronous, 
                                bool shareDHs=false);
  void setOutBufferingProperties(int channel, bool synchronous, 
                                 bool shareDHs=false);

  /** SetRate methods:
      These methods set the rate at which input/output dataholders are
      read/written.
      The rate is the fraction of the events that is to be read/written.
      The dhIndex argument selects only the given input/output
      DataHolder; dhIndex=-1 sets all input/output DataHolders.
  */
  virtual void setProcessRate (int rate) = 0;
  virtual void setInRate (int rate, int dhIndex) = 0;
  virtual void setOutRate (int rate, int dhIndex) = 0;

  /// Get the application number of the current run.
  static int getCurAppl();

   /// Get the event count.
  static unsigned int getEventCount();

  /// Clear the event count.
  static void clearEventCount();

  /// Set the current application number.
  static void setCurAppl (int applNr);

protected:
  /// Increment theirEventCnt.
  static void incrementEventCount();

  /// Set ID of the BlockRep
  void setID();

  /// Connect 2 channels with the given transport prototype.
  bool connectData (const string& name,
		    TransportHolder* prototype,
		    DataManager& sourceDM, int sourceChannel, 
		    DataManager& targetDM, int targetChannel,
		    bool blockingComm);

private:

  int         itsRefCount;

  CompositeRep*   itsParent;        // The parent Composite.
  // This will give all instances of Block the same event in the
  static unsigned int theirNextID;
  static unsigned int theirEventCnt;
  int                 itsID;         // the ID of the step

  bool                itsAddSuffix;  // Add the seqnr as the name suffix?

  int                 itsSeqNr;      // Sequence number in the Composite. 
                                     // Used to know the Block order.
  string              itsName;       // Name of the Block object.
  int                 itsNode;

};

////////////////////////////////////////////////////////////////////////////

inline void BlockRep::incrRefCount()
  { ++itsRefCount; }

inline int BlockRep::decrRefCount()
  { return --itsRefCount; }

/* inline DataHolder& BlockRep::getInData (int dhIndex) */
/*   { return *itsWorker->getDataManager().getGeneralInHolder(dhIndex); } */

/* inline DataHolder& BlockRep::getOutData (int dhIndex) */
/*   { return *itsWorker->getDataManager().getGeneralOutHolder(dhIndex); } */

inline const string& BlockRep::getName() const
  { return itsName; } 

inline void BlockRep::setName (const string& name)
  { itsName = name; }

inline int BlockRep::getID() const
  { return itsID; }

inline int BlockRep::getCurAppl()
  { return WorkHolder::getCurAppl(); } 

inline void BlockRep::setCurAppl (int applNr)
  { WorkHolder::setCurAppl(applNr); } 

inline unsigned int BlockRep::getEventCount()
  { return theirEventCnt; }

inline void BlockRep::clearEventCount()
  { theirEventCnt = 0; }

inline void BlockRep::incrementEventCount()
  { theirEventCnt++; } 

inline CompositeRep* BlockRep::getParent() const
  { return itsParent; }

inline void BlockRep::setParent (CompositeRep& parent)
  { itsParent = &parent; }

inline int BlockRep::getSeqNr() const
  { return itsSeqNr; }

inline void BlockRep::setInBufferingProperties(int channel, bool synchronous, 
				       bool shareDHs)
  { getInDataManager(channel).setInBufferingProperties(channel, synchronous, shareDHs);}

inline void BlockRep::setOutBufferingProperties(int channel, bool synchronous, 
					bool shareDHs)
  { getOutDataManager(channel).setOutBufferingProperties(channel, synchronous, shareDHs);}

inline void BlockRep::setID()
  { itsID = theirNextID++; }

// @}

}

#endif
