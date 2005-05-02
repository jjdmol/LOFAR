//# DataManager.h: Data manager which incorporates asynchronous behaviour
//#
//# Copyright (C) 2000-2002
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

#ifndef CEPFRAME_DATAMANAGER_H
#define CEPFRAME_DATAMANAGER_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <tinyCEP/TinyDataManager.h>
#include <Transport/DataHolder.h>

namespace LOFAR
{

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

class DataManager : public TinyDataManager
{
public:
  /** The constructor with the number of input and output
      DataHolders as arguments.
  */
  DataManager (int inputs=0, int outputs=0);
  DataManager (const TinyDataManager&);
  virtual ~DataManager();
  
  DataHolder* getInHolder(int channel);
  DataHolder* getOutHolder(int channel);
  void readyWithInHolder(int channel);
  void readyWithOutHolder(int channel);
  
  void preprocess();
  void postprocess();

  const string& getInHolderName(int channel) const;
  const string& getOutHolderName(int channel) const; 

  DataHolder* getGeneralInHolder(int channel) const;//Use these functions only 
                                                    // for acquiring of general
                                                    // properties
  DataHolder* getGeneralOutHolder(int channel) const; 

  void initializeInputs();  ///Reads all inputs

  // N.B.The following methods should only be used when defining an application,
  // not during processing.

  // Set properties of a communication channel: synchronisity and sharing of DataHolders
  // by input and output
  void setInBufferingProperties(int channel, bool synchronous, 
				bool shareDHs=false) const;
  void setOutBufferingProperties(int channel, bool synchronous, 
				 bool shareDHs=false) const;

  // Is data transport of input channel synchronous?
  bool isInSynchronous(int channel);
  // Is data transport of output channel synchronous?
  bool isOutSynchronous(int channel);

private:
  /// Copy constructor (copy semantics).
  DataManager (const DataManager&);

  SynchronisityManager* itsSynMan;
  DH_info* itsInDHsinfo;        // Last requested inholders information
  DH_info* itsOutDHsinfo;       // Last requested outholders information

};

} // namespace
#endif
