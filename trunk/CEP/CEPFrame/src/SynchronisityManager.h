//# SynchronisityManager.h: 
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

#ifndef CEPFRAME_SYNCHRONISITYMANAGER_H
#define CEPFRAME_SYNCHRONISITYMANAGER_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Transport/DataHolder.h>
#include <CEPFrame/DHPoolManager.h>
#include <pthread.h>

namespace LOFAR
{

/**
  The main purpose of the SynchronisityManager class is to control asynchronous
  read/write actions. It contains a DHPoolManager for each input and output 
  channel. In case in- and output are shared, the in- and output point to the
  same DHPoolManager object. 
*/

//class DHPoolManager;

// Struct containing data that is shared between main program and thread
typedef struct thread_args{
  pthread_mutex_t mutex;
  DHPoolManager*   manager;
  bool            stopThread;
}thread_data;


class SynchronisityManager
{
public:
  /** The constructor with the number of input and output
      DataHolders as arguments.
  */
  SynchronisityManager (int inputs=0, int outputs=0);

  virtual ~SynchronisityManager();

  DHPoolManager* getInPoolManagerPtr(int channel);
  DHPoolManager* getOutPoolManagerPtr(int channel);

  // Use the same DHPoolManager for input and output
  void sharePoolManager(int channel);

  int getInputs() const;
  int getOutputs() const;

  // Starts a reader thread
  void readAsynchronous(int channel);
  // Starts a writer thread
  void writeAsynchronous(int channel);

  bool isOutSynchronous(int channel);
  void setOutSynchronous(int channel, bool synchronous);
  bool isInSynchronous(int channel);
  void setInSynchronous(int channel, bool synchronous);

  void preprocess();
  void postprocess();

private:
  // Start function for reader thread
static void* startReaderThread(void* thread_arg);
  // Start function for writer thread
static void* startWriterThread(void* thread_arg);

  int itsNinputs;
  int itsNoutputs;
  DHPoolManager** itsInManagers;
  DHPoolManager** itsOutManagers;
  bool* itsInSynchronisities;
  bool* itsOutSynchronisities;
  pthread_t* itsReaders;
  pthread_t* itsWriters;
  thread_data* itsReadersData;
  thread_data* itsWritersData;
};

inline int SynchronisityManager::getInputs() const
  { return itsNinputs; }

inline int SynchronisityManager::getOutputs() const
  { return itsNoutputs; }

inline DHPoolManager* SynchronisityManager::getInPoolManagerPtr(int channel)
  { return itsInManagers[channel]; }

inline DHPoolManager* SynchronisityManager::getOutPoolManagerPtr(int channel)
  { return itsOutManagers[channel]; }

inline bool SynchronisityManager::isOutSynchronous(int channel)
{
  return itsOutSynchronisities[channel];
}

inline bool SynchronisityManager::isInSynchronous(int channel)
{
  return itsInSynchronisities[channel];
}

}

#endif
