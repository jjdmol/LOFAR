//#  TinyDataManager.h: a DataHolder manager for tinyCEP
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

#ifndef TINYDEP_TINYDATAMANAGER_H
#define TINYDEP_TINYDATAMANAGER_H

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Transport/DataHolder.h>

namespace LOFAR
{
  //# Forward Declarations
  class Selector;

  // Description of class.
  class TinyDataManager
  {
  public:
    TinyDataManager(int inputs, int outputs);

    virtual ~TinyDataManager();

    virtual DataHolder* getInHolder(int dholder);
    virtual DataHolder* getOutHolder(int dholder);

    // Add a DataHolder to the DataManager. The DataManager will then take care 
    // of DataHolder reading, writing and clean-up.
    virtual void addInDataHolder(int channel, DataHolder* dhptr);
    virtual void addOutDataHolder(int channel, DataHolder* dhptr);

    virtual void preprocess();
    virtual void postprocess();

    virtual void initializeInputs();

    virtual DataHolder* getGeneralInHolder(int channel);
    virtual DataHolder* getGeneralOutHolder(int channel);

   /**  accessor functions to the auto trigger flags.
       This flag determines if automated triggering of read/write
       sequences is wanted. 
   **/
    virtual bool doAutoTriggerIn(int channel) const;
    virtual bool doAutoTriggerOut(int channel) const;
    void setAutoTriggerIn(int channel, 
			  bool newflag) const;
    void setAutoTriggerOut(int channel, 
			   bool newflag) const;

    virtual void readyWithInHolder(int channel);
    virtual void readyWithOutHolder(int channel);

    int getInputs() const;
    int getOutputs() const;

    void setProcessRate(int rate);
    void setInputRate(int rate, int dhIndex=-1);
    void setOutputRate(int rate, int dhIndex=-1);

    int getProcessRate();
    int getInputRate(int dhIndex=-1);
    int getOutputRate(int dhIndex=-1);

    // The following methods are used for selecting one of the inputs/outputs
    void setInputSelector(Selector* selector);  // Set an input selector
    void setOutputSelector(Selector* selector);  // Set an output selector

    DataHolder* selectInHolder();   // Select an input
    DataHolder* selectOutHolder();  // Select an output

    // Check if this TinyDataManager has a selector
    bool hasInputSelector();
    bool hasOutputSelector();

    void setReadyInFlag(int channel);
    void setReadyOutFlag(int channel);
    bool getReadyInFlag(int channel);
    bool getReadyOutFlag(int channel);
    void clearReadyInFlag(int channel);
    void clearReadyOutFlag(int channel);
  protected:  
    int itsNinputs;
    int itsNoutputs;

    TinyDataManager (const TinyDataManager&);
 
    DataHolder** itsInDHs;
    DataHolder** itsOutDHs;

    /// the DataManager also stores the input- output- and 
    /// processrates.
    int itsProcessRate;
    int* itsInputRates;
    int* itsOutputRates;

  /**  The auto trigger flags.
       This flag determines if automated triggering of read/write
       sequences is wanted. 
   **/
    bool*     itsDoAutoTriggerIn;
    bool*     itsDoAutoTriggerOut;
 
    Selector* itsInputSelector;  // Input selection mechanism
    Selector* itsOutputSelector; // Output selection mechanism
 
  private:  
    void assertChannel(int channel, bool input);

    /// A static to keep track of the DataHolderID's
    static int DataHolderID;
    bool* itsReadyInFlag;
    bool* itsReadyOutFlag;
  };

inline int TinyDataManager::getInputs() const { 
  return itsNinputs; }

inline int TinyDataManager::getOutputs() const { 
  return itsNoutputs; }

inline void TinyDataManager::setProcessRate(int rate)
{ itsProcessRate = rate; }

inline int TinyDataManager::getProcessRate()
{ return itsProcessRate; }

inline void TinyDataManager::setReadyInFlag(int channel)
{ itsReadyInFlag[channel] = true;}
inline void TinyDataManager::setReadyOutFlag(int channel)
{ itsReadyOutFlag[channel] = true;}
inline bool TinyDataManager::getReadyInFlag(int channel)
{ return itsReadyInFlag[channel];}
inline bool TinyDataManager::getReadyOutFlag(int channel)
{ return itsReadyOutFlag[channel];}
inline void TinyDataManager::clearReadyInFlag(int channel)
{ itsReadyInFlag[channel] = false;}
inline void TinyDataManager::clearReadyOutFlag(int channel)
{ itsReadyOutFlag[channel] = false;}



} // namespace LOFAR
#endif
