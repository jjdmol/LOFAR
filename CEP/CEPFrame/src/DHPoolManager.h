//# DHPoolManager.h: Base class for the dataholder pool managers
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

#ifndef CEPFRAME_DHPOOLMANAGER_H
#define CEPFRAME_DHPOOLMANAGER_H

/*  #include <lofar_config.h> */

#include "Transport/DataHolder.h"

namespace LOFAR
{

/**
 The main purpose of this class is to control access to a DataHolder
 (in a pool with size = 1). It offers a common interface for all access to
 DataHolders and is a base class for the CycBufferManager class.
*/
class DHPoolManager
{
public:
  /** The constructor.
  */
  DHPoolManager();

  virtual ~DHPoolManager();

  void setDataHolder(DataHolder* dhptr);

  virtual DataHolder* getWriteLockedDH(int* id);
  virtual DataHolder* getReadLockedDH(int* id);
  virtual DataHolder* getRWLockedDH(int* id);
  
  virtual void writeUnlock(int) {};
  virtual void readUnlock(int) {};
  virtual void preprocess();
  
  virtual int getSize();
  
  bool getSharing();
  void setSharing(bool share);

  DataHolder* getGeneralDataHolder();

 protected:
  DataHolder* itsDataHolder;

 private:
  bool        itsShared;

};

inline void DHPoolManager::setDataHolder(DataHolder* dhptr)
{
  itsDataHolder = dhptr;
}

inline DataHolder* DHPoolManager::getGeneralDataHolder()
{
  return itsDataHolder;
}

inline void DHPoolManager::setSharing(bool share)
{
  itsShared = share;
}

inline bool DHPoolManager::getSharing()
{
  return itsShared;
}

}

#endif
