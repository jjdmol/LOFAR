//#  MiniDataManager.h: a BaseDataManager implementation for tinyCEP
//#
//#  Copyright (C) 2002-2004
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

#ifndef TINYDEP_MINIDATAMANAGER_H
#define TINYDEP_MINIDATAMANAGER_H

#include <lofar_config.h>

//# Includes
#include <Common/Debug.h>
#include <tinyCEP/BaseDataManager.h>
#include <Transport/DataHolder.h>

namespace LOFAR
{

  //# Forward Declarations
  class forward;


  // Description of class.
  class MiniDataManager : public BaseDataManager
  {
  public:
    MiniDataManager(int inputs, int outputs);

    virtual ~MiniDataManager();

    DataHolder* getInHolder(int dholder);
    DataHolder* getOutHolder(int dholder);

    void addInDataHolder(int channel, DataHolder* dhptr);
    void addOutDataHolder(int channel, DataHolder* dhptr);

    void preprocess();
/*     void process(); */
    void postprocess();

    void initializeInputs();

    DataHolder* getGeneralInHolder(int channel);
    DataHolder* getGeneralOutHolder(int channel);

    bool hasInputSelector();
    bool hasOutputSelector();

    void setDoAutoTriggerIn(bool trigger);
    void setDoAutoTriggerOut(bool trigger);

    bool doAutoTriggerIn(int channel) const;
    bool doAutoTriggerOut(int channel) const;

    void readyWithInHolder(int channel);
    void readyWithOutHolder(int channel);

    void setProcessRate(int rate);
    void setInputRate(int rate);
    void setOutputRate(int rate);

    int getProcessRate();
    int getInputRate();
    int getOutputRate();

    /// Get the number of inputs or outputs.
    int getInputs() const;
    int getOutputs() const;

  private:
    MiniDataManager (const MiniDataManager&);
    
    int itsNinputs;
    int itsNoutputs; 
    
    DataHolder** itsInDHs;
    DataHolder** itsOutDHs;
    
    void assertChannel(int channel, bool input);

    /// A static to keep track of the DataHolderID's
    static int DataHolderID;

    /// the DataManager also stores the input- output- and 
    /// processrates.
    int itsProcessRate;
    int itsInputRate;
    int itsOutputRate;

    bool itsDoAutoTriggerIn;
    bool itsDoAutoTriggerOut;
  };

inline int MiniDataManager::getInputs() const
{ return itsNinputs; }

inline int MiniDataManager::getOutputs() const 
{ return itsNoutputs; }

inline bool MiniDataManager::hasInputSelector() 
{ return false; }

inline bool MiniDataManager::hasOutputSelector()
{ return false; }

inline void MiniDataManager::setDoAutoTriggerIn(bool trigger) 
{ itsDoAutoTriggerIn = trigger; }

inline void MiniDataManager::setDoAutoTriggerOut(bool trigger) 
{ itsDoAutoTriggerOut = trigger; }

inline bool MiniDataManager::doAutoTriggerIn(int channel) const
{ return itsDoAutoTriggerIn; }

inline bool MiniDataManager::doAutoTriggerOut(int channel) const
{ return itsDoAutoTriggerOut; }

inline void MiniDataManager::setProcessRate(int rate)
{ itsProcessRate = rate; }

inline void MiniDataManager::setInputRate(int rate)
{ itsInputRate = rate; }

inline void MiniDataManager::setOutputRate(int rate)
{ itsOutputRate = rate; }

inline int MiniDataManager::getProcessRate()
{ return itsProcessRate; }

inline int MiniDataManager::getInputRate()
{ return itsInputRate; }

inline int MiniDataManager::getOutputRate()
{ return itsOutputRate; }

} // namespace LOFAR
#endif
