//#  GSA_SysConnGuard.h: manages the properties with its use count
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

#ifndef GSA_SYSCONNGUARD_H
#define GSA_SYSCONNGUARD_H

#include <GSA_Service.h>
#include <GCF/GCF_PVString.h>

/**
 * 
*/
namespace LOFAR {
 namespace GCF {
  namespace PAL {

class GCFSysConnGuard;

class GSASysConnGuard : public GSAService
{
public:
  	GSASysConnGuard(GCFSysConnGuard& sysConnGuard) :
    _sysConnGuard(sysConnGuard),
    _isSubscribed(false)
    {};
  	virtual ~GSASysConnGuard();

    void start ();
    void stop ();
    bool isSubscribed() const;
        
protected:
    void dpCreated(const string& /*propName*/) {};
    void dpDeleted(const string& /*propName*/) {};
    void dpeValueGet(const string& /*propName*/, const Common::GCFPValue& /*value*/) {}; 
    void dpeValueChanged(const string& propName, const Common::GCFPValue& value);
    void dpeValueSet(const string& /*propName*/) {};
    void dpeSubscribed(const string& propName);
    void dpeSubscriptionLost (const string& propName);
    void dpeUnsubscribed(const string& /*propName*/) {};

private: 
	// data members
    GCFSysConnGuard& _sysConnGuard;
    
	// admin. data members
    bool _isSubscribed;
};

inline bool GSASysConnGuard::isSubscribed() const
{
  return _isSubscribed;
}

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
