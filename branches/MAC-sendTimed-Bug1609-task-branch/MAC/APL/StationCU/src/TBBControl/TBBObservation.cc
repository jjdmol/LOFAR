//#  TBBObservation.cc: one line description
//#
//#  Copyright (C) 2006
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
//#  $Id: TBBObservation.cc 9881 2007-01-24 12:39:40Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_datetime.h>
#include <Common/StringUtil.h>

#include "TBBObservation.h"

using namespace LOFAR;
using namespace StationCU;

TBBObservation::TBBObservation() :
    TbbSettings()
{
}

TBBObservation::TBBObservation(ParameterSet* aParSet)
{
    itsSettingsLoaded = false;
    clockFreq = 200;
    sampleTime = 5.0;
    string prefix;
    // analyse ParameterSet.
    
    
    prefix = aParSet->locateModule("Observation") + "Observation.TBB";
    LOG_DEBUG_STR("'TBB' located at: " << prefix);
    
    vhecrTaskEnabled = aParSet->getBool(prefix+"vhecrTaskEnabled", true);
        
    prefix = aParSet->locateModule("Observation") + "Observation.TBB.TBBsetting";
    LOG_DEBUG_STR("'TBB.TBBsetting' located at: " << prefix);

    int setNr = 0;
    string setnr(formatString("[%d].", setNr));

    cSettings tbbsettings;

    // look if the TBB settings are defined
    while (aParSet->isDefined(prefix+setnr+"baselevel")) {
        LOG_DEBUG_STR("Reading parameterSet " << setNr);
        itsSettingsLoaded = true;

        tbbsettings.triggerLevel  = aParSet->getUint16(prefix+setnr+"baselevel");
        tbbsettings.filter        = aParSet->getUint16(prefix+setnr+"filter");

        // new settings for two filterbanks
        tbbsettings.filter0[0]    = aParSet->getUint16(prefix+setnr+"filter0_coeff0");
        tbbsettings.filter0[1]    = aParSet->getUint16(prefix+setnr+"filter0_coeff1");
        tbbsettings.filter0[2]    = aParSet->getUint16(prefix+setnr+"filter0_coeff2");
        tbbsettings.filter0[3]    = aParSet->getUint16(prefix+setnr+"filter0_coeff3");
        tbbsettings.filter1[0]    = aParSet->getUint16(prefix+setnr+"filter1_coeff0");
        tbbsettings.filter1[1]    = aParSet->getUint16(prefix+setnr+"filter1_coeff1");
        tbbsettings.filter1[2]    = aParSet->getUint16(prefix+setnr+"filter1_coeff2");
        tbbsettings.filter1[3]    = aParSet->getUint16(prefix+setnr+"filter1_coeff3");

        tbbsettings.operatingMode = aParSet->getUint16(prefix+setnr+"operatingMode");
        tbbsettings.startLevel    = aParSet->getUint16(prefix+setnr+"startlevel");
        tbbsettings.stopLevel     = aParSet->getUint16(prefix+setnr+"stoplevel");
        tbbsettings.triggerMode   = aParSet->getUint16(prefix+setnr+"triggerMode");
        tbbsettings.detectWindow  = _windowNr(aParSet->getString(prefix+setnr+"window"));

        tbbsettings.RCUset.reset();                            // clear RCUset by default.
        string    rcuString("x=" + expandArrayString(aParSet->getString(prefix+setnr+"RCUs")));
        ParameterSet    rcuParset;
        rcuParset.adoptBuffer(rcuString);
        vector<uint16> RCUnumbers(rcuParset.getUint16Vector("x"));
        if (RCUnumbers.empty()) {            // No receivers in the list?
            tbbsettings.RCUset.set();                        // assume all receivers
            allRCUset.set();
        } else {
            for (uint i = 0; i < RCUnumbers.size();i++) {
                tbbsettings.RCUset.set(RCUnumbers[i]);
                allRCUset.set(RCUnumbers[i]);    // set mentioned receivers in all set
            }
        }

        TbbSettings.push_back(tbbsettings);

        tbbsettings.SubbandList = aParSet->getInt32Vector(prefix+"subbandList", vector<int32>(), true);
        setNr++;
        setnr = formatString("[%d]", setNr);
    }
    if (setNr == 0) {
        LOG_DEBUG_STR("No TBB parameterSets defined");
    }
    
    // get used clock frequency
    prefix = aParSet->locateModule("Observation") + "Observation.";
    LOG_DEBUG_STR("'Observation' located at: " << prefix);
    clockFreq = aParSet->getInt32(prefix + "sampleClock");
    sampleTime = 1. / (clockFreq*1e6);
    NSEC2SAMPLE = 1. / (sampleTime*1e9);
}

//
// _windowNr
//
uint16 TBBObservation::_windowNr(const string&        wdwName)
{
    static    char* windowNames[] = {
        "4K", "16K", "64K", "256K", "1M", "4M", "16M", "64M", "256M", "" };

    uint32    idx = 0;
    while (windowNames[idx][0]) {
        if (!strcmp(wdwName.c_str() , windowNames[idx])) {
            return (idx);
        }
        idx++;
    }

    LOG_WARN_STR("detectWindow '" << wdwName << "' is unknown, assuming 1M");
    return (4);
}

//#
//# operator<<
//#
ostream& TBBObservation::print(ostream&    os) const
{
    int setNr = 0;
    vector<cSettings>::const_iterator it;
    for (it = TbbSettings.begin(); it != TbbSettings.end(); it++ ) {
        os << "Observation set[" << setNr << "]" << endl;
        os << "Filter-0 coeff-0 : " << (*it).filter0[0] << endl;
        os << "Filter-0 coeff-1 : " << (*it).filter0[1] << endl;
        os << "Filter-0 coeff-2 : " << (*it).filter0[2] << endl;
        os << "Filter-0 coeff-3 : " << (*it).filter0[3] << endl;
        os << "Filter-1 coeff-0 : " << (*it).filter1[0] << endl;
        os << "Filter-1 coeff-1 : " << (*it).filter1[1] << endl;
        os << "Filter-1 coeff-2 : " << (*it).filter1[2] << endl;
        os << "Filter-1 coeff-3 : " << (*it).filter1[3] << endl;
        os << "TriggerLevel     : " << (*it).triggerLevel << endl;
        os << "Filter           : " << (*it).filter << endl;
        os << "StartLevel       : " << (*it).startLevel << endl;
        os << "StopLevel        : " << (*it).stopLevel << endl;
        os << "detectWindow     : " << (*it).detectWindow << endl;
        os << "triggerMode      : " << (*it).triggerMode << endl;
        os << "operatingMode    : " << (*it).operatingMode << endl;
        os << "RCUset           : " << (*it).RCUset << endl << endl;
        os << "subbandList      : ";
        std::vector<int32>::const_iterator sb_it;
        for (sb_it = (*it).SubbandList.begin(); sb_it != (*it).SubbandList.end(); sb_it++) {
            os << (*sb_it) << " ";
        }
        os << endl;
        setNr++;
    }
    if (vhecrTaskEnabled) {
        os << "vhecrTaskEnabled : yes" << endl;
    }
    else {
        os << "vhecrTaskEnabled : no" << endl;
    }
    return (os);
}
