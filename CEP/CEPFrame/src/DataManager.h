// DataManager.h: Base class for the data managers
//
//  Copyright (C) 2000-2002
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

#ifndef CEPFRAME_DATAMANAGER_H
#define CEPFRAME_DATAMANAGER_H

/*  #ifdef HAVE_CONFIG_H */
/*  #include <config.h> */
/*  #endif */

#include "CEPFrame/DataHolder.h"

typedef struct 
{
  DataHolder* currentDH;
  int         id;             // id of dataholder in DHPoolManager
} DH_info;

/**
  Class DataManager is the base class for all data managers
  in the CEPFrame environment. It main purpose is to offer a common interface
  to a class like WorkHolder. Apart from that it also offers some common
  functionality to the classes derived from it.
*/

class SynchronisityManager;
class Selector;

class DataManager
{
public:
  /** The constructor with the number of input and output
      DataHolders as arguments.
  */
  DataManager (int inputs=0, int outputs=0);

  virtual ~DataManager();
  
  DataHolder* getInHolder(int channel);
  DataHolder* getOutHolder(int channel);
  void readyWithInHolder(int channel);
  void readyWithOutHolder(int channel);
  
  void addInDataHolder(int channel, DataHolder* dhptr, bool synchronous=true, 
		       bool shareIO=false);
  void addOutDataHolder(int channel, DataHolder* dhptr, bool synchronous=true, 
			bool shareIO=false);

  void preprocess();
  void postprocess();

  const string& getInHolderName(int channel) const;
  const string& getOutHolderName(int channel) const; 

  DataHolder* getGeneralInHolder(int channel); // Use these functions only for
                                               // acquiring of general properties
  DataHolder* getGeneralOutHolder(int channel); 

  /// Get the number of inputs or outputs.
  int getInputs() const;
  int getOutputs() const;

  void initializeInputs();  ///Reads all inputs

  // Is data transport of input channel synchronous?
  bool isInSynchronous(int channel);

  // Is data transport of output channel synchronous?
  bool isOutSynchronous(int channel);

  // The following methods are used for selecting one of the inputs/outputs
  void setInputSelector(Selector* selector);  // Set an input selector
  void setOutputSelector(Selector* selector);  // Set an output selector
  DataHolder* selectInHolder();   // Select an input
  DataHolder* selectOutHolder();  // Select an output

  // Check if the datamanager has a selector
  bool hasInputSelector();
  bool hasOutputSelector();

private:
  /// Copy constructor (copy semantics).
  DataManager (const DataManager&);

  int itsNinputs;
  int itsNoutputs;
  SynchronisityManager* itsSynMan;
  DH_info* itsInDHs;        // Last requested inholders info
  DH_info* itsOutDHs;       // Last requested outholders info

  Selector* itsInputSelector;  // Input selection mechanism
  Selector* itsOutputSelector; // Output selection mechanism
};

inline int DataManager::getInputs() const
  { return itsNinputs; }

inline int DataManager::getOutputs() const
  { return itsNoutputs; }


#endif
