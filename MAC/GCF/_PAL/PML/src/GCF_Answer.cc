//#  GCF_Answer.cc: 
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

#include <lofar_config.h>

#include <GCF/PAL/GCF_Answer.h>

/**
 * F_PVSS_PROTOCOL signal names
 */
namespace LOFAR {
 namespace GCF {
  namespace PAL {

const char* F_PML_PROTOCOL_signalnames[] =
{
  "F_PML_PROTOCOL: invalid signal",
  "F_SUBCRIBED",
  "F_UNSUBCRIBED",
  "F_VCHANGEMSG",
  "F_VGETRESP",
  "F_VSETRESP",
  "F_EXTPS_LOADED",
  "F_EXTPS_UNLOADED",
  "F_PS_CONFIGURED",
  "F_MYPS_ENABLED",
  "F_MYPS_DISABLED",
  "F_SERVER_GONE",
};

const struct TM::protocolStrings F_PML_PROTOCOL_STRINGS = {
	12, 0, F_PML_PROTOCOL_signalnames, 0
};


  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
