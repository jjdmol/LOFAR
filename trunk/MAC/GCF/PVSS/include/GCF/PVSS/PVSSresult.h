//#  PVSSresult.h: preprocessor definitions of various constants
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

#ifndef PVSSRESULT_H
#define PVSSRESULT_H

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

enum PVSSresult 
{
  SA_NO_ERROR = 0, 				// 0
  SA_SCHEDULED,					// 1
  SA_PROPNAME_MISSING,			// 2
  SA_DPTYPE_UNKNOWN,			// 3
  SA_MACTYPE_UNKNOWN,			// 4
  SA_CREATEPROP_FAILED,			// 5
  SA_DELETEPROP_FAILED,			// 6
  SA_SUBSCRIBEPROP_FAILED,		// 7
  SA_UNSUBSCRIBEPROP_FAILED,	// 8
  SA_SETPROP_FAILED,			// 9
  SA_GETPROP_FAILED,			// 10
  SA_QUERY_SUBSC_FAILED,		// 11
  SA_QUERY_UNSUBSC_FAILED,		// 12
  SA_SCADA_NOT_AVAILABLE, 		// 13
  SA_PROP_DOES_NOT_EXIST,		// 14
  SA_PROP_ALREADY_EXIST,		// 15
  SA_MACTYPE_MISMATCH,			// 16
  SA_ELEMENTS_MISSING			// 17
};

  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR
#endif
