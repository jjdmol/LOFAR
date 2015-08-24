//#  GCF_PVTypes.cc: 
//#
//#  Copyright (C) 2014
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
//#  $Id: GCF_PVString.cc 22948 2012-11-23 08:54:47Z loose $


#include <lofar_config.h>

#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/GCF_Defines.h>

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

#define CREATE_AND_INIT_PVALUE(type, className, value) \
{ \
	GCFPValue* p = GCFPValue::createMACTypeObject(type); \
	ASSERTSTR(p, "Creation of object of type " << type << " failed"); \
	((className*)p)->setValue(value); \
	return (p); \
}
		
GCFPValue*	createPValue(const bool		someVal) CREATE_AND_INIT_PVALUE(LPT_BOOL, 	  GCFPVBool,	someVal)
GCFPValue*	createPValue(const char		someVal) CREATE_AND_INIT_PVALUE(LPT_CHAR, 	  GCFPVChar,	someVal)
GCFPValue*	createPValue(const uint		someVal) CREATE_AND_INIT_PVALUE(LPT_UNSIGNED, GCFPVUnsigned,someVal)
GCFPValue*	createPValue(const int		someVal) CREATE_AND_INIT_PVALUE(LPT_INTEGER,  GCFPVInteger,	someVal)
GCFPValue*	createPValue(const float	someVal) CREATE_AND_INIT_PVALUE(LPT_DOUBLE,	  GCFPVDouble,	someVal)
GCFPValue*	createPValue(const double	someVal) CREATE_AND_INIT_PVALUE(LPT_DOUBLE,	  GCFPVDouble,	someVal)
GCFPValue*	createPValue(const char*	someVal) CREATE_AND_INIT_PVALUE(LPT_STRING,	  GCFPVString,	someVal)
GCFPValue*	createPValue(const string&	someVal) CREATE_AND_INIT_PVALUE(LPT_STRING,	  GCFPVString,	someVal)
GCFPValue*	createPValue(const time_t	someVal) CREATE_AND_INIT_PVALUE(LPT_DATETIME, GCFPVDateTime,someVal)

  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
