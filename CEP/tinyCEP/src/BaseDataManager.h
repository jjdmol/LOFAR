//#  BaseDataManager.h: A DataManager baseclass
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef TINYCEP_BASEDATAMANAGER_H
#define TINYCEP_BASEDATAMANAGER_H

#include <lofar_config.h>

//# Includes
#include <Transport/DataHolder.h>

namespace LOFAR
{

// Description of class.
  class BaseDataManager
  {
  public:
    BaseDataManager(int inputs, int outputs);
    BaseDataManager();
    virtual ~BaseDataManager();
    
    virtual DataHolder* getInHolder(int dholder);
    virtual DataHolder* getOutHolder(int dholder);

    virtual void addInDataHolder(int channel, DataHolder* dhptr);
    virtual void addOutDataHolder(int channel, DataHolder* dhptr);
    
    /// Get the number of inputs or outputs.
    int getInputs() const;
    int getOutputs() const;

  protected:

    int itsNinputs;
    int itsNoutputs;

    
  };

inline int BaseDataManager::getInputs() const { 
  return itsNinputs; }

inline int BaseDataManager::getOutputs() const { 
  return itsNoutputs; }


} // namespace LOFAR

#endif
