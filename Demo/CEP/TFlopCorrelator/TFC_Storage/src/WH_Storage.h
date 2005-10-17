//#  WH_Storage.h: 
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

#ifndef TFLOPCORRELATOR_WH_STORAGE_H
#define TFLOPCORRELATOR_WH_STORAGE_H


#include <Common/KeyValueMap.h>
#include <tinyCEP/WorkHolder.h>
#ifdef USE_MAC_PI
#include <GCF/PALlight/CEPPropertySet.h>
#include <GCF/GCF_PVDynArr.h>
#endif

namespace LOFAR
{

class MSWriter;

class WH_Storage: public WorkHolder
  {
  public:

    explicit WH_Storage(const string& name, 
			const ACC::APS::ParameterSet& pset);
    virtual ~WH_Storage();
    
    static WorkHolder* construct(const string& name, 
                                 const ACC::APS::ParameterSet& pset);
    virtual WH_Storage* make(const string& name);

    void preprocess();

    virtual void process();

  private:
    /// forbid copy constructor
    WH_Storage (const WH_Storage&);
    /// forbid assignment
    WH_Storage& operator= (const WH_Storage&);

    const ACC::APS::ParameterSet itsPS;
    int itsNstations;
    int itsNChannels;
    int itsNpolSquared;

    MSWriter* itsWriter;

    vector<int> itsBandIds;       // MS ID s of frequency bands
    int itsFieldId;
    int itsCounter;

#ifdef USE_MAC_PI
    bool itsWriteToMAC;
    GCF::CEPPMLlight::CEPPropertySet* itsPropertySet;
    GCF::Common::GCFPValueArray itsVArray; 
#endif
  };
} // namespace LOFAR

#endif
