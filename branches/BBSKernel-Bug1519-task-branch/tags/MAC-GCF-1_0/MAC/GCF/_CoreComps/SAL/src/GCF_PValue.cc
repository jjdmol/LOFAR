//#  GCF_PValue.cc: 
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


#include "GCF_PValue.h"
#include "GCF_PVBool.h"
#include "GCF_PVDouble.h"
#include "GCF_PVChar.h"
#include "GCF_PVString.h"
#include "GCF_PVInteger.h"
#include "GCF_PVUnsigned.h"
#include "GCF_PVDynArr.h"
#include "GSA_Defines.h"

GCFPValue* GCFPValue::createMACTypeObject(TMACValueType type)
{
  GCFPValue* pResult(0);
  
  switch (type)
  {
    case LPT_BOOL:
      pResult = new GCFPVBool();
      break;
    case LPT_CHAR:
      pResult = new GCFPVChar();
      break;
    case LPT_UNSIGNED:
      pResult = new GCFPVUnsigned();
      break;
    case LPT_INTEGER:
      pResult = new GCFPVInteger();
      break;
    case LPT_DOUBLE:
      pResult = new GCFPVDouble();
      break;
    case LPT_STRING:
      pResult = new GCFPVString();
      break;
/*    case BIT32_VAL:
      pResult = new GCFPVBit32();
      break;
    case LPT_REF:
      pResult = new GCFPVRef();
      break;
    case LPT_BLOB:
      pResult = new GCFPVBlob();
      break;
    case LPT_DATETIME:
      pResult = new GCFPVDateTime();
      break;*/
    default:
      if (type > LPT_DYNARR &&
          type <= LPT_DYNSTRING)
      {
        pResult = new GCFPVDynArr(type);
      }
      else
      {
        LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
            "Type of MAC value is unknown or not supported yet: '%d'", 
            type));
      }
      break;
  }  
  
  return pResult;
}
