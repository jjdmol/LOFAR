//#  GCF_Defines.h: preprocessor definitions of various constants
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

#ifndef GCF_DEFINES_H
#define GCF_DEFINES_H

#include <Common/LofarLogger.h>

#include <assert.h>

#define GCF_PROP_NAME_SEP     '.'

enum TGCFResult {
  GCF_NO_ERROR, 
  GCF_UNKNOWN_ERROR,
  GCF_PML_ERROR, 
  GCF_EXTPS_LOAD_ERROR,
  GCF_PS_CONFIGURE_ERROR,
  GCF_EXTPS_UNLOAD_ERROR,
  GCF_MYPS_ENABLE_ERROR,
  GCF_MYPS_DISABLE_ERROR,
  GCF_VALUESTRING_NOT_VALID,
  GCF_DIFFERENT_TYPES,
  GCF_BUSY,
  GCF_ALREADY_LOADED,
  GCF_NOT_LOADED,
  GCF_PROP_NOT_VALID,
  GCF_PROP_WRONG_TYPE,
  GCF_PROP_NOT_IN_SET,
  GCF_PROTECTED_STATE,
  GCF_NO_PROPER_DATA,
  GCF_SCOPE_ALREADY_REG,
  GCF_ALREADY_SUBSCRIBED,
  GCF_NOT_SUBSCRIBED,
  GCF_WRONG_STATE
};

typedef unsigned char TAccessMode;

#define GCF_READABLE_PROP 1
#define GCF_WRITABLE_PROP 2
typedef enum TMACValueType {NO_LPT, LPT_BOOL, LPT_CHAR, LPT_UNSIGNED, LPT_INTEGER, 
                    LPT_BIT32, LPT_BLOB, LPT_REF, LPT_DOUBLE, LPT_DATETIME,
                    LPT_STRING, LPT_DYNARR = 0x80,
                    LPT_DYNBOOL, LPT_DYNCHAR, LPT_DYNUNSIGNED, LPT_DYNINTEGER, 
                    LPT_DYNBIT32, LPT_DYNBLOB, LPT_DYNREF, LPT_DYNDOUBLE, LPT_DYNDATETIME,
                    LPT_DYNSTRING };
 
typedef struct
{
  char*         propName;
  TMACValueType  type;
  TAccessMode   accessMode;
  char*         defaultValue;
}
TProperty;

typedef struct
{
  char*             typeName;
  bool              isTemporary;
  unsigned int      nrOfProperties;
  const TProperty*  properties;  
}
TPropertySet;
#endif
