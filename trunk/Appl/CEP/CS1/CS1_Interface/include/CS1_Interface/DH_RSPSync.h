//# DH_RSPSync.h: DataHolder used to synchronize incoming RSP data
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
//#  $Id$

#ifndef LOFAR_APPL_CEP_CS1_CS1_INTERFACE_DH_RSPSYNC_H
#define LOFAR_APPL_CEP_CS1_CS1_INTERFACE_DH_RSPSYNC_H


#include <lofar_config.h>

#include <Transport/DataHolder.h>
#include <CS1_Interface/RSPTimeStamp.h>

namespace LOFAR
{
  class DH_RSPSync: public DataHolder
{
public:

  explicit DH_RSPSync (const string& name);

  DH_RSPSync(const DH_RSPSync&);

  virtual ~DH_RSPSync();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  /// Set the sync stamp
  void setSyncStamp(const timestamp_t syncStamp);

  /// Get the sync stamp
  const timestamp_t getSyncStamp() const;
  void incrementStamp(const int value);

private:
  /// Forbid assignment.
  DH_RSPSync& operator= (const DH_RSPSync&);

  timestamp_t*  itsSyncStamp;

  void fillDataPointers();
};

inline void DH_RSPSync::setSyncStamp(const timestamp_t syncStamp)
  { *itsSyncStamp = syncStamp; }
 
inline const timestamp_t DH_RSPSync::getSyncStamp() const
  { return *itsSyncStamp;}

inline void DH_RSPSync::incrementStamp(const int value)
  { *itsSyncStamp += value;}
}
#endif 
