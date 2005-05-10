//#  WH_RSPInput.h: Catch RSP ethernet frames and synchronize RSP inputs 
//#
//#  Copyright (C) 2002-2005
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

#ifndef STATIONCORRELATOR_WH_RSPINPUT_H
#define STATIONCORRELATOR_WH_RSPINPUT_H


#include <Common/KeyValueMap.h>
#include <tinyCEP/WorkHolder.h>
#include <DH_RSPSync.h>

namespace LOFAR
{
  class WH_RSPInput: public WorkHolder
  {
  public:

    explicit WH_RSPInput(const string& name, 
                         const KeyValueMap kvm,
		         const bool isSyncMaster = false);
    virtual ~WH_RSPInput();
    
    static WorkHolder* construct(const string& name, 
                                 const KeyValueMap kvm,
				 const bool isSyncMaster = false);
    virtual WH_RSPInput* make(const string& name);

    virtual void process();

    /// set delay of this WorkHolder
    void setDelay(const DH_RSPSync::syncStamp_t newDelay);
    
    /// Show the work holder on stdout.
    virtual void dump();

  private:
    /// forbid copy constructor
    WH_RSPInput (const WH_RSPInput&);
    /// forbid assignment
    WH_RSPInput& operator= (const WH_RSPInput&);

    int itsNpackets;
    int itsPolarisations;
    int itsNbeamlets;
    int itsNCorrOutputs;
    int itsNRSPOutputs;
    int itsSzEPAheader;
    int itsSzEPApacket;

    KeyValueMap itsKVM;

    // for synchronisation
    bool itsIsSyncMaster; // Am I the one that sends the sync packets?
    DH_RSPSync::syncStamp_t itsNextStamp;
    bool itsReadNext; // Do we need to read at the beginning of the next process()?

    static ProfilingState theirOldDataState;
    static ProfilingState theirMissingDataState;
  };

} // namespace LOFAR

#endif
