//#  GSB_Defines.h: preprocessor definitions of various constants
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
//#  MERCHANTABILITY or FITNESS FOR A SBRTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef GSB_DEFINES_H
#define GSB_DEFINES_H

#define LOFARLOGGER_SUBPACKAGE "SB"

#include <GCF/GCF_Defines.h>

class GCFPValue;

enum TSBResult 
{
  SB_NO_ERROR, 
  SB_UNKNOWN_ERROR,
  SB_SERVICE_ALREADY_EXIST,
  SB_NO_FREE_PORTNR,
  SB_UNKNOWN_SERVICE
};

#define PARAM_SB_SERVER_PORT "mac.gcf.sb.port"
#define PARAM_SB_SERVER_HOST "mac.gcf.sb.host"
#endif
