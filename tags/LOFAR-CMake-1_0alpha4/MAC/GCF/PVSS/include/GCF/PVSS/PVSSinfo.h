//#  PVSSinfo.h: PVSS connection to a remote process
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

#ifndef PVSSINFO_H
#define PVSSINFO_H

#include <GCF/PVSS/GCF_Defines.h>
#include <Common/LofarTypes.h>

// forward declaration

namespace LOFAR {
 namespace GCF {  
  namespace PVSS {

class PVSSservice;

// By means of this class information from the PVSS system can be retreived
class PVSSinfo
{
public:
    static bool propExists (const string& dpeName);
    static bool typeExists (const string& dpTypeName);
    static TMACValueType getMACTypeId (const string& dpeName);
//	static bool EventPort::_askBrokerThePortNumber();
	static bool isValidPropName(const char* propName);
	static bool isValidScope   (const char* propName);
	static const string&  getDPbasename(const string& name);	// strip off systemname

    static const string getSystemName(uint sysnr);
    static const string& getLocalSystemName();
    static uint getLocalSystemId();
    static uint getSysId(const string& name);
    static uint getLastEventSysId();

    static const string& getProjectName();

    static timeval getLastEventTimestamp();

    static uint getLastEventManNum();
    static uint getOwnManNum();
    static uint getLastEventManType();

    static TGCFResult getTypeStruct(const string& 			typeName, 
									list<TPropertyInfo>& 	propInfo, 
									uint 					sysNr = getLocalSystemId());

    static void getAllProperties(const string& typeFilter, const string& dpFitler, 
                                 vector<string>& foundProperties);
    
    static void getAllTypes(const string& typeFilter, 
                            vector<string>& foundTypes);
private:
    friend class PVSSservice;
    static string	_sysName;
    static string	_projName;
    static uint	_lastSysNr;
    static timeval	_lastTimestamp;
    static uint	_lastManNum;
    static uint	_lastManType;
  
    // Construction methods
    // Don't allow to (con/de)struct an instance of this class
    PVSSinfo ();
  
    virtual ~PVSSinfo ();

    // Don't allow copying this object.
    // <group>
    PVSSinfo (const PVSSinfo&);
    PVSSinfo& operator= (const PVSSinfo&);
    // </group>
};

inline uint PVSSinfo::getLastEventSysId()
{
  return _lastSysNr;
}

inline timeval PVSSinfo::getLastEventTimestamp()
{
  return _lastTimestamp;
}

inline uint PVSSinfo::getLastEventManNum()
{
  return _lastManNum;
}

inline uint PVSSinfo::getLastEventManType()
{
  return _lastManType;
}

  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR
#endif
