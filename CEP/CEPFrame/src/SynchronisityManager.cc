//  SynchronisityManager.cc:
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
// SynchronisityManager.cc: implementation of the SynchronisityManager class.
//
//////////////////////////////////////////////////////////////////////

#include "CEPFrame/SynchronisityManager.h"
#include "CEPFrame/DHPoolManager.h"
#include "CEPFrame/CycBufferManager.h"
#include <Common/Debug.h>
#include <unistd.h>

namespace LOFAR
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SynchronisityManager::SynchronisityManager (int inputs, int outputs)
  : itsNinputs(inputs),
    itsNoutputs(outputs)
{
  TRACER2("SynchronisityManager constructor");
  itsInManagers = new DHPoolManager*[inputs];
  itsOutManagers = new DHPoolManager*[outputs];
  itsInSynchronisities = new bool[inputs];
  itsOutSynchronisities = new bool[outputs];
  itsReaders = new pthread_t[inputs];
  itsReadersData = new thread_data[inputs];
  itsWriters = new pthread_t[outputs];
  itsWritersData = new thread_data[outputs];

  for (int i = 0; i < inputs; i++)
  {
    itsInManagers[i] = new DHPoolManager();
    itsInSynchronisities[i] = true;
    int status = pthread_mutex_init(&itsReadersData[i].mutex, NULL);
    DbgAssertStr(status == 0, "Init reader mutex failed");
    itsReadersData[i].manager = 0;
    itsReadersData[i].stopThread = true;
  }

  for (int j = 0; j < outputs; j++)
  {
    itsOutManagers[j] = new DHPoolManager();
    itsOutSynchronisities[j] = true;
    int status = pthread_mutex_init(&itsWritersData[j].mutex, NULL);
    DbgAssertStr(status == 0, "Init writer mutex failed");
    itsWritersData[j].manager = 0;
    itsWritersData[j].stopThread = true;
  }
}

SynchronisityManager::~SynchronisityManager()
{
  // Termination of all active reader and writer threads

  TRACER4("SynchronisityManager postprocess");
  for (int i = 0; i < itsNinputs; i++)
  {
    pthread_mutex_lock(&itsReadersData[i].mutex);
    if (itsReadersData[i].stopThread == false)
    {
      itsReadersData[i].stopThread = true;  // Causes reader threads to exit
    }
    pthread_mutex_unlock(&itsReadersData[i].mutex);
//      void* thread_res;
//      pthread_join(itsReaders[i], &thread_res);
  }
  for (int j = 0; j < itsNoutputs; j++)
  {
    pthread_mutex_lock(&itsWritersData[j].mutex);
    if (itsWritersData[j].stopThread == false)
    {
      itsWritersData[j].stopThread = true; // Causes writer threads to exit
    }
    pthread_mutex_unlock(&itsWritersData[j].mutex);
//      void* thread_res;
//      pthread_join(itsWriters[j], &thread_res);
  }

  for (int k = 0; k < itsNinputs; k++)
  {
    delete itsInManagers[k];
  }
  for (int m = 0; m < itsNoutputs; m++)
  {
    if (itsOutManagers[m]->getSharing() == false)
    {
      delete itsOutManagers[m];
    }
  }

  delete [] itsInManagers;
  delete [] itsOutManagers;
  delete [] itsInSynchronisities;
  delete [] itsOutSynchronisities;
  delete [] itsReaders;
  delete [] itsWriters;
  delete [] itsReadersData;
  delete [] itsWritersData;
}

void SynchronisityManager::preprocess()
{
  for (int i = 0; i < itsNinputs; i++)
  {
    DbgAssertStr(itsInManagers[i]!=0, "No DHPoolManager constructed");
    itsInManagers[i]->preprocess();
  }

  for (int j = 0; j < itsNoutputs; j++)
  {
    DbgAssertStr(itsOutManagers[j]!=0, "No DHPoolManager constructed");
    itsOutManagers[j]->preprocess();
  }
}

void SynchronisityManager::postprocess()
{
}

void* SynchronisityManager::startReaderThread(void* thread_arg)
{
  TRACER2("In reader thread ID " << pthread_self());
  thread_data* data = (thread_data*)thread_arg;
  DHPoolManager* manager = data->manager;
  
  while (1)
  {
    pthread_mutex_lock(&data->mutex);           /// Check stop condition
    if (data->stopThread == true)
    {
      pthread_mutex_unlock(&data->mutex);
      TRACER2("Reader thread " << pthread_self() << " exiting");
      pthread_exit(NULL);
    }
    pthread_mutex_unlock(&data->mutex);
    int id;
    TRACER2("Thread " << pthread_self() << " attempting to read");

    DataHolder* dh = manager->getWriteLockedDH(&id);

    while (dh->read() == false)
    {
      pthread_mutex_lock(&data->mutex);         /// Check stop condition
      if (data->stopThread == true)
      {
	manager->writeUnlock(id);
	pthread_mutex_unlock(&data->mutex);
	TRACER2("Reader thread " << pthread_self() << " exiting");
	pthread_exit(NULL);
      }
      pthread_mutex_unlock(&data->mutex);
    }
    
    manager->writeUnlock(id);
  }
}

void* SynchronisityManager::startWriterThread(void* thread_arg)
{
  TRACER2("In writer thread ID " << pthread_self());
  thread_data* data = (thread_data*)thread_arg;

  DHPoolManager* manager = data->manager;

  while (1)
  {
    pthread_mutex_lock(&data->mutex);
    if (data->stopThread == true)               /// Check stop condition
    {
      pthread_mutex_unlock(&data->mutex);
      TRACER2("Writer thread " << pthread_self() << " exiting");
      pthread_exit(NULL);
    }
    pthread_mutex_unlock(&data->mutex);
    int id;
    TRACER2("Thread " << pthread_self() << " attempting to write");
    manager->getReadLockedDH(&id)->write();
    manager->readUnlock(id);
  
  }
}

void SynchronisityManager::setOutSynchronous(int channel, bool synchronous)
{
  if (synchronous != itsOutSynchronisities[channel])
  {
    itsOutSynchronisities[channel] = synchronous;
    delete itsOutManagers[channel];
    if (synchronous == true)
    {
      itsOutManagers[channel] = new DHPoolManager();
      pthread_mutex_lock(&itsWritersData[channel].mutex); // Stop writer thread
      itsWritersData[channel].stopThread = true;
      pthread_mutex_unlock(&itsWritersData[channel].mutex);
      if (itsWriters[channel] != 0)
      {
	pthread_join(itsWriters[channel], NULL);
      }
    }
    else
    {
      itsOutManagers[channel] = new CycBufferManager();
      // Initialization???
    }
  }
}

void SynchronisityManager::setInSynchronous(int channel, bool synchronous)
{
  if (synchronous != itsInSynchronisities[channel])
  {
    itsInSynchronisities[channel] = synchronous;
    delete itsInManagers[channel];
    if (synchronous == true)
    {
      itsInManagers[channel] = new DHPoolManager();
      pthread_mutex_lock(&itsReadersData[channel].mutex); // Stop reader thread
      itsReadersData[channel].stopThread = true;
      pthread_mutex_unlock(&itsReadersData[channel].mutex);
      if (itsReaders[channel] != 0)
      {
	pthread_join(itsReaders[channel], NULL);
      }
    }
    else
    {
      itsInManagers[channel] = new CycBufferManager();
      // Initialization???
    }
  }
}

void SynchronisityManager::readAsynchronous(int channel)
{
  pthread_mutex_lock(&itsReadersData[channel].mutex);

  if (itsReadersData[channel].stopThread == true)
  {                                            // Thread creation
    itsReadersData[channel].manager = itsInManagers[channel];
    itsReadersData[channel].stopThread = false;
    pthread_create(&itsReaders[channel], NULL, startReaderThread, &itsReadersData[channel]);
    TRACER4("Reader thread " << itsReaders[channel] << " created");
  }

  pthread_mutex_unlock(&itsReadersData[channel].mutex);
}

void SynchronisityManager::writeAsynchronous(int channel)
{
  pthread_mutex_lock(&itsWritersData[channel].mutex);

  if (itsWritersData[channel].stopThread == true)
  {                                              // Thread creation
    itsWritersData[channel].manager = itsOutManagers[channel];
    itsWritersData[channel].stopThread = false;
    TRACER4("Writer thread creation");
    pthread_create(&itsWriters[channel], NULL, startWriterThread, &itsWritersData[channel]);
  }

  pthread_mutex_unlock(&itsWritersData[channel].mutex);
}

void SynchronisityManager::sharePoolManager(int channel)
{
  if (itsOutManagers[channel] != 0)        // Sets output DHPoolManager pointer to 
  {                                        // input DHPoolManager
    delete itsOutManagers[channel];
  }
  itsOutManagers[channel] = itsInManagers[channel];
  itsOutManagers[channel]->setSharing(true);
  itsOutSynchronisities[channel] = itsInSynchronisities[channel];
}


}
