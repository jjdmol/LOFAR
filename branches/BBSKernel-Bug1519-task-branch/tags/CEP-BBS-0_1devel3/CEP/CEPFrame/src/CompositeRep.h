//# CompositeRep.h: Class to hold a collection of Block objects
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

#ifndef LOFAR_CEPFRAME_COMPOSITEREP_H
#define LOFAR_CEPFRAME_COMPOSITEREP_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
// \file
// Class to hold a collection of Block objects

#include <stdlib.h>
#include <Common/lofar_string.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_map.h>

#include <Common/lofar_list.h>
#include <CEPFrame/BlockRep.h>
#include <CEPFrame/VirtualMachine.h>

namespace LOFAR
{
// \addtogroup CEPFrame
// @{

class Composite;
class Connection;

/** The Composite class is related to the Block class following a Composite pattern.
    Therefore, a Composite is a collection of Blocks and/or Composites
    In the constructor the actual workholder is defined. 
    The actual simulation work is performed in the process() method
    (which calls the Workholder::process() method)
*/

class CompositeRep: public BlockRep
{
public:

  /** Normally used basic constructor.
      A pointer to a workholder (containing the dataholders) is passed.
  */
  CompositeRep (int nrInputs,
		int nrOutputs,
		const string& name,
		bool addnameSuffix);

  virtual ~CompositeRep();
  
  
  /** Set Node numbers for both In and OutData
      Set application number as well.
      Do it recursively for all Blocks in this Composite.
  */
  virtual void runOnNode (int aNode, int applNr);

  /** The preprocess method is called before process().
      It can be used to initialize WorkHolders, etc.
  */
  virtual void preprocess();

  /** The process method is the basical simulation step.
      It will first read the data for the in DataHolders in the workholder
      Then it will call the process() method for all the Blocks in the list
      Finally it will write the data from the DataHolders
      (e.g to another MPI process)
   */
  virtual void process();

  /** The postprocess method is called after process.
      It can be used to clean up WorkHolders, etc.
  */
  virtual void postprocess();

  /// Dump information to the user
  virtual void dump() const;

  /// Get number of inputs / outputs
  int getNrInputs() const;
  int getNrOutputs() const;

  /** SetRate methods:
      These methods set the rate at which input/output dataholders are
      read/written.
      The rate is the fraction of the events that is to be read/written.
      The dhIndex argument selects only the given input/output
      DataHolder; dhIndex=-1 sets all input/output DataHolders.
  */
  void setProcessRate (int rate);
  void setInRate (int rate, int dhIndex);
  void setOutRate (int rate, int dhIndex);

  /// Add a Block to this Composite
  void addBlock (const Block& aBlock);

  /// Get all Blocks in the Composite.
  //  const list<Block*>& getBlocks() const;

  /// Distinguish between step and simul
  bool isComposite() const;

  /// Mark if this Composite is not part of another Composite
  bool isHighestLevel() const;
  /// lower the highest level flag
  void setNotHighestLevel();

  /**
     Set nrChan input channels of aBlock as input channels of this CompositeRep.
     Start at the given indices. 
     If nrChan=-1 is given, it will be set to the 
     minimum number of input channels in aBlock and this CompositeRep.
     This is needed in order to let the framework transport the data read by 
     the Composite directly to the DataHolder of the Block in the Composite.
  */
  void setInput (int thisIndex, 
		 Block* aBlock, 
		 int thatIndex, 
		 int nrChan);
  
  /**
     Set nrChan output channels of aBlock as output channels of this CompositeRep.
     Start at the given indices.
     If nrChan=-1 is given, it will be set to the minimum number of output
     channels in aBlock and this CompositeRep.
     This is needed in order to let the framework transport the data from 
     the last Block in the Composite to the output of the Composite.
  */
  void setOutput (int thisIndex, 
		  Block* aBlock, 
		  int thatIndex, 
		  int nrChan);

  /**
     Set input channels in the aBlock[] array as input channels of 
     this CompositeRep.
     This is needed in order to let the framework transport
     the data read by the Composite directly to the DataHolders of the first Blocks
     in the Composite.
     If the nrBlocks argument is -1, it is assumed that
     each of the Blocks in the array has only one output channel
     which will be connected to the input channel of the Composite.
  */
  void setInputArray (Block* aBlock[],  // pointer to  array of ptrs to Blocks
		      int    nrBlocks);  // nr of Blocks in aBlock[] array

  /**
     Set output channels in the aBlock[] array as output channels of 
     this CompositeRep.
     This is needed in order to let the framework transport
     the data from the last Block in the Composite to the output of the Composite.
     If the nrBlocks argument is -1, it is assumed that
     each of the Blocks in the array has only one input channel
     which will be connected to the output channel of the Composite.
   */
  void setOutputArray (Block* aBlock[],  // pointer to  array of ptrs to Blocks
		       int    nrBlocks);  // nr of Blocks in aBlock[] array

  /**
     Get a pointer to the Virtual Machine controlling this CompositeRep. The Virtual 
     Machine is created in the contructor.
   */
  VirtualMachine& getVM();

  void addConnection(Connection* conn);

  /// Get DataManager handling a specific in/output channel
  DataManager& getInDataManager(int channel);
  DataManager& getOutDataManager(int channel);

  /// Get the channel number in the actual DataManager of an input/output channel.
  /// For a Step these will be identical.
  int getInChannelNumber(int channel);
  int getOutChannelNumber(int channel);

  /// Get the node number.
  int getNode() const;

  /// Get the application number.
  int getAppl() const; 

 private:

  // Struct to indicate channels
  class ChannelInfo{
  public: 
    ChannelInfo() : block(0), chanNr(-1) {}
    Block* block;
    int    chanNr;
  };

  /// true = this Composite is the top Composite
  bool itsIsHighestLevel;
  /// List of Blocks contained in the Composite
  list<Block*> itsBlocks;
  /// List of Connections contained in the Composite
  list<Connection*> itsConnections;

  /// The VirtualMachine object.
  VirtualMachine itsVM;

  int itsNrInputs;
  int itsNrOutputs;

  int itsNode;
  int itsAppl;
  
  vector<ChannelInfo> itsInputs;   // Mapping of input channels to containing Blocks
  vector<ChannelInfo> itsOutputs;  // Mapping of outputs channels to containing Blocks
};

  
inline bool CompositeRep::isHighestLevel() const
  { return itsIsHighestLevel; }
inline void CompositeRep::setNotHighestLevel()
  { itsIsHighestLevel = false; }
/* inline const list<Block*>& CompositeRep::getBlocks() const */
/*  { return itsBlocks; } */
inline VirtualMachine& CompositeRep::getVM()
 { return itsVM; }

inline int CompositeRep::getNode() const
  { return itsNode; }

inline int CompositeRep::getNrInputs() const
  { return itsNrInputs; }

inline int CompositeRep::getNrOutputs() const
  { return itsNrOutputs; } 

// @}

}

#endif
