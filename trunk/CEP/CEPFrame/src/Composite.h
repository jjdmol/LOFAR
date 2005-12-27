//# Composite.h: Class to hold a collection of Block objects
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

#ifndef LOFAR_CEPFRAME_COMPOSITE_H
#define LOFAR_CEPFRAME_COMPOSITE_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
// \file
// Class to hold a collection of Block objects

#include <CEPFrame/Block.h>
#include <CEPFrame/CompositeRep.h>
//#include <Common/lofar_list.h>

namespace LOFAR
{
// \addtogroup CEPFrame
// @{

class NetworkBuilder;
class VirtualMachine;

/** The Composite class is related to the Block class following a Composite 
    pattern.
    Therefore, a Composite is a collection of Blocks and/or Composites
    The actual simulation work is performed in the process() method
    (which calls its Blocks process() method)
*/

class Composite: public Block
{
public:

  /** Create the Composite with a number inputs and outputs.
      The Composite must get a unique name. To make that process easy,
      by default the suffix _n is added to the name when the Composite is
      added to a Composite (where n is the sequence number starting with 0).
  */
  explicit Composite (int nrInputs,
		      int nrOutputs,
		      const string& name = "aComposite", 
		      bool addNameSuffix = true);

  /// Construct the object using a builder object.
  explicit Composite (int nrInputs,
		      int nrOutputs,
		      NetworkBuilder& builder,
		      const string& name = "aComposite",
		      bool addNameSuffix = true);

  /** Default constructor is only useful for a Composite class member
      (as used in class ApplicationHolder).
      It does not create a useful Composite object.
  */
  Composite();

  /// Copy constructor (reference semantics).
  Composite (const Composite&);

  /// Construct from an existing CompositeRep (meant for internal use).
  explicit Composite (CompositeRep*);

  virtual ~Composite();
  
  /// Assignment (reference semantics).
  Composite& operator= (const Composite&);

  /// Make a correct copy (reference semantics).
  virtual Composite* clone() const;

  /** Add a Block to this Composite
      An exception is thrown if the name of the step (possibly after
      adding the suffix (see constructor)) is not unique.
      Also an exception is thrown if the step has already been added to
      another simul.
  */
  void addBlock (const Block* aBlock)
    { itsComposite->addBlock (*aBlock); }
  void addBlock (const Block& aBlock)
    { itsComposite->addBlock (aBlock); }

  /// Get all Blocks in the Composite.
/*   const list<Block*>& getBlocks() const */
/*     { return itsComposite->getBlocks(); } */

  /// Is this Composite is not part of another Composite?
  bool isHighestLevel() const
    { return itsComposite->isHighestLevel(); }
  /// Clear the highest level flag (thus it is part of another Composite).
  void setNotHighestLevel()
    { itsComposite->setNotHighestLevel(); }

  /// Is the Block a Composite?
  bool isComposite() const; 

  /// Connect this Composite to an array of Composites.
  bool connectInputArray (Composite* aComposite[],   // pointer to  array of ptrs to Blocks
			  int    nrItems, // nr of Blocks in aBlock[] array
			  TransportHolder* prototype,
			  bool blockingComm=true);

  /**
     Set nrChan input channels of aBlock as input channels of this Composite.
     Start at the given indices. 
     If nrChan=-1 is given, it will be set to the 
     minimum number of input channels in aBlock and this Composite.
     This is needed in order to let the framework transport the data read by 
     the Composite directly to the DataHolder of the Block in the Composite.
  */
  void setInput (int thisIndex, 
		 Block* aBlock, 
		 int thatIndex, 
		 int nrChan)
    { return itsComposite->setInput (thisIndex, aBlock, thatIndex, nrChan); }
  
  /**
     Set nrChan output channels of aBlock as output channels of this Composite.
     Start at the given indices.
     If nrChan=-1 is given, it will be set to the minimum number of output
     channels in aBlock and this Composite.
     This is needed in order to let the framework transport the data from 
     the last Block in the Composite to the output of the Composite.
  */
  void setOutput (int thisIndex, 
		  Block* aBlock, 
		  int thatIndex, 
		  int nrChan)
    { return itsComposite->setOutput (thisIndex, aBlock, thatIndex, nrChan); }

  /**
     Set input channels in the aBlock[] array as input channels of 
     this composite.
     This is needed in order to let the framework transport
     the data read by the Composite directly to the DataHolders of the first Blocks
     in the Composite.
     If the nrBlocks argument is -1, it is assumed that
     each of the Blocks in the array has only one output channel
     which will be connected to the input channel of the Composite.
  */
  void setInputArray (Block* aBlock[],  // pointer to  array of ptrs to Blocks
		      int    nrBlocks)  // nr of Blocks in aBlock[] array
    { return itsComposite->setInputArray (aBlock, nrBlocks); }

  /**
     Set output channels in the aBlock[] array as output channels of 
     this composite.
     This is needed in order to let the framework transport
     the data from the last Blocks in the Composite to the output of the Composite.
     If the nrBlocks argument is -1, it is assumed that
     each of the Blocks in the array has only one input channel
     which will be connected to the output channel of the Composite.
   */
  void setOutputArray (Block* aBlock[],  // pointer to  array of ptrs to Blocks
		       int    nrBlocks)  // nr of Blocks in aBlock[] array
    { return itsComposite->setOutputArray (aBlock, nrBlocks); }

  /// Obtain a pointer to the VirtualMachine object
  VirtualMachine& getVM()
    { return itsComposite->getVM(); }

private:
  // Pointer to the actual simul object.
  CompositeRep* itsComposite;
};

inline bool Composite::isComposite() const
  { return true; }

// @}

}
  
#endif
