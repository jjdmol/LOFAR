//#
//#  ARAConstants.h: constants related to the BeamServer
//#
//#  Copyright (C) 2002-2004
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

#ifndef ARACONSTANTS_H_
#define ARACONSTANTS_H_

namespace ARA
{
  
  const char PARAM_N_RACKS[]                     = "mac.apl.ara.N_RACKS";
  const char PARAM_N_SUBRACKS_PER_RACK[]         = "mac.apl.ara.N_SUBRACKS_PER_RACK";
  const char PARAM_N_BOARDS_PER_SUBRACK[]        = "mac.apl.ara.N_BOARDS_PER_SUBRACK";
  const char PARAM_N_APS_PER_BOARD[]             = "mac.apl.ara.N_APS_PER_BOARD";
  const char PARAM_N_RCUS_PER_AP[]               = "mac.apl.ara.N_RCUS_PER_AP";
  const char PARAM_STATUS_UPDATE_INTERVAL[]      = "mac.apl.ara.STATUS_UPDATE_INTERVAL";
  const char PARAM_STATISTICS_UPDATE_INTERVAL[]  = "mac.apl.ara.STATISTICS_UPDATE_INTERVAL";
  
};
     
#endif /* ARACONSTANTS_H_ */
