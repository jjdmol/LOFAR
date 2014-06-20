//#  GCF_DynTypes.h: preprocessor definitions of various constants
//#
//#  Copyright (C) 2011
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
//#  $Id: GCF_Defines.h 15657 2010-05-11 10:19:59Z loose $

#ifndef GCF_DYNTYPES_H
#define GCF_DYNTYPES_H

#include <GCF/PVSS/GCF_Defines.h>

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

 
#define	LPT_DYNBOOL			(TMACValueType)(LPT_DYNARR | LPT_BOOL)
#define	LPT_DYNCHAR			(TMACValueType)(LPT_DYNARR | LPT_CHAR)
#define	LPT_DYNUNSIGNED		(TMACValueType)(LPT_DYNARR | LPT_UNSIGNED)
#define	LPT_DYNINTEGER		(TMACValueType)(LPT_DYNARR | LPT_INTEGER)
#define	LPT_DYNBIT32		(TMACValueType)(LPT_DYNARR | LPT_BIT32)
#define	LPT_DYNBLOB			(TMACValueType)(LPT_DYNARR | LPT_BLOB)
#define	LPT_DYNREF			(TMACValueType)(LPT_DYNARR | LPT_REF)
#define	LPT_DYNDOUBLE		(TMACValueType)(LPT_DYNARR | LPT_DOUBLE)
#define	LPT_DYNDATETIME		(TMACValueType)(LPT_DYNARR | LPT_DATETIME)
#define	LPT_DYNSTRING		(TMACValueType)(LPT_DYNARR | LPT_STRING)
#define	LPT_DYNARRAY		(TMACValueType)(LPT_DYNARR | LPT_ARRAY)


  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR
#endif
