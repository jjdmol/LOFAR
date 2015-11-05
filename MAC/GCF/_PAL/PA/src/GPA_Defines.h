//#  GPA_Defines.h: preprocessor definitions of various constants
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

#ifndef GPA_DEFINES_H
#define GPA_DEFINES_H

#include <GCF/GCF_Defines.h>
#include <GCF/Protocols/PA_Protocol.ph>

namespace LOFAR {
 namespace GCF {
  namespace PAL {

const string PA_TASK_NAME("GCF-PA");
const string PA_PS_SESSION_TASK_NAME_EXT("_session");
const string PS_ENABLED_EXT = "__enabled";

#define PS_IS_AUTOLOAD(cat) (cat > PS_CAT_PERMANENT)
#define PS_IS_TEMPORARY(cat) (cat == PS_CAT_TEMPORARY || cat == PS_CAT_TEMP_AUTOLOAD)

// internal extension of F_PA_PROTOCOL signals
enum 
{
  PA_PROP_SET_DP_CREATED_ID = PA_PROP_SET_GONE_ID + 1,                                 
  PA_PROP_SET_DP_DELETED_ID
};

#define PA_PROP_SET_DP_CREATED F_SIGNAL(PA_PROTOCOL, PA_PROP_SET_DP_CREATED_ID, F_IN) 
#define PA_PROP_SET_DP_DELETED F_SIGNAL(PA_PROTOCOL, PA_PROP_SET_DP_DELETED_ID, F_IN) 

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

#endif
