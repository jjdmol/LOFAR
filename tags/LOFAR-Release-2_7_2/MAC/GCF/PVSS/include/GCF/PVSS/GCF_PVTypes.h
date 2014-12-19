//#  GCF_PVTypes.h: Define all PVSS types at once
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

#ifndef GCF_PVTYPES_H
#define GCF_PVTYPES_H

#include <GCF/PVSS/GCF_PValue.h>
#include <GCF/PVSS/GCF_PVBlob.h>
#include <GCF/PVSS/GCF_PVBool.h>
#include <GCF/PVSS/GCF_PVChar.h>
#include <GCF/PVSS/GCF_PVDateTime.h>
#include <GCF/PVSS/GCF_PVDouble.h>
#include <GCF/PVSS/GCF_PVDynArr.h>
#include <GCF/PVSS/GCF_PVInteger.h>
#include <GCF/PVSS/GCF_PVString.h>
#include <GCF/PVSS/GCF_PVUnsigned.h>

namespace LOFAR {
  namespace GCF {
	namespace PVSS {

GCFPValue*	createPValue(const bool		someVal);
GCFPValue*	createPValue(const char		someVal);
GCFPValue*	createPValue(const uint		someVal);
GCFPValue*	createPValue(const int		someVal);
GCFPValue*	createPValue(const float	someVal);
GCFPValue*	createPValue(const double	someVal);
GCFPValue*	createPValue(const char*	someVal);
GCFPValue*	createPValue(const string&	someVal);
GCFPValue*	createPValue(const time_t	someVal);

  } // namespace PVS
 } // namespace GCF
} // namespace LOFAR


#endif
