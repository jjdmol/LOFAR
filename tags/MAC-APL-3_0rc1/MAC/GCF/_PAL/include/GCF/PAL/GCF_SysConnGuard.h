//#  GCF_SysConnGuard.h: 
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

#ifndef GCF_SYSCONNGUARD_H
#define GCF_SYSCONNGUARD_H

#include <Common/lofar_string.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/TM/GCF_Fsm.h>
#include <set>

using std::set;
using std::string;

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM
  {
// forward declaration
class GCFTask;
class GCFEvent;
  }
  namespace PAL
  {


class GSASysConnGuard;

class GCFSysConnGuard
{
  private:
    explicit GCFSysConnGuard ();
  
  public:
    static GCFSysConnGuard* instance();
    virtual ~GCFSysConnGuard ();
  
  public:
    bool isRunning() const;
    bool isSysConnected(const string& sysName) const;
    bool registerTask(TM::GCFTask& task);
    bool unregisterTask(TM::GCFTask& task);
    
  private:
    // Don't allow copying this object.
    // <group>
    GCFSysConnGuard (const GCFSysConnGuard&);
    GCFSysConnGuard& operator= (const GCFSysConnGuard&);
    // </group>
    
  private: // helper methods
    friend class GSASysConnGuard;
    void serviceEvent(const string& sysName, bool gone);
            
  private: // data members
    static GCFSysConnGuard* _instance;
    
    GSASysConnGuard* _pSysConnGuardService;
    TM::GCFDummyPort     _dummyPort;
    
    typedef set<TM::GCFTask*> TRegisteredTasks;
    TRegisteredTasks _registeredTasks;
};

inline bool GCFSysConnGuard::isSysConnected(const string& sysName) const
{
  return (GCFPVSSInfo::getSysId(sysName) > 0); 
}

enum 
{
  F_SCF_PROTOCOL = TM::F_GCF_PROTOCOL + 4,
};


// F_PML_PROTOCOL signals
enum 
{
  F_SYSGONE_ID = 1,                                 
  F_SYSCONN_ID,      
};

#define F_SYSGONE   F_SIGNAL(F_SCF_PROTOCOL, F_SYSGONE_ID, F_IN)
#define F_SYSCONN   F_SIGNAL(F_SCF_PROTOCOL, F_SYSCONN_ID, F_IN)

// NOTE: These structs cannot be used to send messages by real port 
// implementations like TCP. 
struct GCFSysConnGuardEvent : public TM::GCFEvent
{
  // @param sig can only be F_SYSGONE, F_SYSCONN
  GCFSysConnGuardEvent(unsigned short sig) : TM::GCFEvent(sig)
  {
      length = sizeof(GCFSysConnGuardEvent);
  }
  // Pointer to the system name string
  const char* pSysName; 
};

// defined in GCF_SysConnGuard.cc
extern const char* F_SCG_PROTOCOL_signalnames[];

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
