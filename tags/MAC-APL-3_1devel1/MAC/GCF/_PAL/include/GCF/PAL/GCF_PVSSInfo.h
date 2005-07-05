//#  GCF_PVSSInfo.h: PVSS connection to a remote process
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

#ifndef GCF_PVSSINFO_H
#define GCF_PVSSINFO_H

#include <GCF/GCF_Defines.h>

// forward declaration

namespace LOFAR 
{
 namespace GCF 
 {
  namespace PAL
  {

class GSAService;

// By means of this class information from the PVSS system can be retreived
class GCFPVSSInfo
{
  public:
    static bool propExists (const string& dpeName);
    static bool typeExists (const string& dpTypeName);
    static const string& getLocalSystemName();
    static unsigned int getLocalSystemId();
    static const string& getProjectName();
    static const string getSystemName(unsigned int sysnr);
    static unsigned int getLastEventSysId();
    static unsigned int getSysId(const string& name);
    static unsigned int getManNum();
    static Common::TGCFResult getTypeStruct(const string& typeName, 
                                            list<Common::TPropertyInfo>& propInfo, 
                                            unsigned int sysNr);    
    
  private:
    friend class GSAService;
    static string _sysName;
    static string _projName;
    static unsigned int _lastSysNr;
  
    // Construction methods
    // Don't allow to (con/de)struct an instance of this class
    GCFPVSSInfo ();
  
    virtual ~GCFPVSSInfo ();

    // Don't allow copying this object.
    // <group>
    GCFPVSSInfo (const GCFPVSSInfo&);
    GCFPVSSInfo& operator= (const GCFPVSSInfo&);
    // </group>
};
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
