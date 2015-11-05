//#  TBBObservation.h: class/struct that holds the Observation information
//# 
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
//#  $Id: TBBObservation.h 10634 2007-11-01 11:32:07Z overeem $

#ifndef TBBOBSERVATION_H
#define TBBOBSERVATION_H

// \file
// class/struct that holds the Observation information

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/ParameterSet.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_bitset.h>
#include <Common/LofarConstants.h>

namespace LOFAR {
  namespace StationCU {

// \addtogroup APLCommon
// @{

//# Forward Declarations
//class forward;


// The TBBObservation class is a public struct that can be used as base-class
// for holding TBBObservation related information.
// It can be instantiated with a parset containing Observation information.
class TBBObservation
{
public:
    TBBObservation();
    ~TBBObservation() { };
    
    explicit    TBBObservation (ParameterSet*        aParSet);
    
    typedef bitset<MAX_RCUS> RCUset_t;

    class cSettings {
    public:
        cSettings() {};
        ~cSettings() {};
        uint16 filter0[4];
        uint16 filter1[4];
        uint16 triggerLevel;
        uint16 filter;
        uint16 startLevel;
        uint16 stopLevel;
        uint16 detectWindow;
        uint16 triggerMode;
        uint16 operatingMode;         // transient or subbands
        
        RCUset_t      RCUset;
        vector<int32> SubbandList;
    };
    //# Datamembers
    vector<cSettings> TbbSettings;
    RCUset_t allRCUset;
    bool vhecrTaskEnabled;
    int clockFreq;
    double sampleTime;
    double NSEC2SAMPLE;
    
    bool isLoaded();
    //# print function for operator<<
    ostream&    print(ostream&    os) const;

private:
    bool itsSettingsLoaded;
    uint16 _windowNr(const string&        wdwName);

};

inline bool TBBObservation::isLoaded() { return(itsSettingsLoaded); }
//#
//# operator<<
//#
inline ostream& operator<< (ostream& os, const TBBObservation& aTBBObservation)
{    
    return (aTBBObservation.print(os));
}

  } // namespace StationCU
} // namespace LOFAR

#endif
