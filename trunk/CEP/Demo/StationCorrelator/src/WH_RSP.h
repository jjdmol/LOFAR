//#  WH_RSP.h: Analyse RSP ethernet frames and store datablocks, blockID, 
//#            stationID and isValid flag in DH_StationData
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

#ifndef STATIONCORRELATOR_WH_RSP_H
#define STATIONCORRELATOR_WH_RSP_H


#include <Common/KeyValueMap.h>
#include <tinyCEP/WorkHolder.h>
#include <DH_RSPSync.h>

namespace LOFAR
{
  class WH_RSP: public WorkHolder
  {
  public:

    explicit WH_RSP(const string& name, 
                    const KeyValueMap kvm,
		    const bool isSyncMaster = false);
    virtual ~WH_RSP();
    
    static WorkHolder* construct(const string& name, 
                                 const KeyValueMap kvm,
				 const bool isSyncMaster = false);
    virtual WH_RSP* make(const string& name);

    virtual void process();

    /// set delay of this WorkHolder
    void setDelay(const DH_RSPSync::syncStamp_t newDelay);
    
    /// Show the work holder on stdout.
    virtual void dump();

  private:
    /// forbid copy constructor
    WH_RSP (const WH_RSP&);
    /// forbid assignment
    WH_RSP& operator= (const WH_RSP&);

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
    DH_RSPSync::syncStamp_t itsDelay;    
    bool itsReadNext; // Do we need to read at the beginning of the next process()?
  };

  inline void WH_RSP::setDelay(const DH_RSPSync::syncStamp_t newDelay)
    { itsDelay = newDelay; }

} // namespace LOFAR

#endif
