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

#include <Common/lofar_list.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>

using std::string;
using std::list;
using std::map;
using std::vector;

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

// This header file will be included by each GCF API class header and provides
// therefore a number of defines, structs or macro's. They can/must be used when
// calling a GCF API class memberfunction.

#define GCF_SYS_NAME_SEP     ':' // separates the databasename from the ppropertyname
#define GCF_PROP_NAME_SEP    '.' // separates the levels in property sets
#define GCF_SCOPE_NAME_SEP   '_' // separates the levels in structure of property sets

// possible results of GCF api class member calls.
enum TGCFResult 
{
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
  GCF_NO_PROPER_DATA,
  GCF_SCOPE_ALREADY_REG,
  GCF_ALREADY_SUBSCRIBED,
  GCF_NOT_SUBSCRIBED,
  GCF_WRONG_STATE,
  GCF_PVSS_ERROR,
};

typedef unsigned char TAccessMode;

#define GCF_READABLE_PROP 1 // means monitorable
#define GCF_WRITABLE_PROP 2 // means controllable
#define GCF_READWRITE_PROP (GCF_READABLE_PROP | GCF_WRITABLE_PROP)

// struct for initalize properties with accessMode and/or defaultValue
// if defaultValue == 0 no default value will be set
struct TPropertyConfig
{
  char*         propName;
  TAccessMode   accessMode;
  char*         defaultValue;
};

// start macro of a list of configurations of properties belonging to the same
// property set
#define PROPERTYCONFIGLIST_BEGIN(_name_) \
const TPropertyConfig _name_[] = \
{

// config item of the list of configurations of properties belonging to the same
// property set
#define PROPERTYCONFIGLIST_ITEM(_propname_,_flags_,_default_) \
{_propname_,_flags_,_default_},

// end macro of a list of configurations of properties belonging to the same
// property set
#define PROPERTYCONFIGLIST_END \
{0,0,0} \
}; 

/**
 * The enumeration of possible MAC property types
 * In case a dynamic array will be used the type ID enumeration starts on 
 * 0x80.
 * END_* are only delimeters
 */
typedef enum TMACValueType 
{
  NO_LPT, LPT_BOOL, LPT_CHAR, LPT_UNSIGNED, LPT_INTEGER, 
  LPT_BIT32, LPT_BLOB, LPT_REF, LPT_DOUBLE, LPT_DATETIME,
  LPT_STRING, END_LPT, 
  LPT_DYNARR = 0x80, LPT_DYNBOOL, LPT_DYNCHAR, LPT_DYNUNSIGNED, LPT_DYNINTEGER, 
  LPT_DYNBIT32, LPT_DYNBLOB, LPT_DYNREF, LPT_DYNDOUBLE, LPT_DYNDATETIME,
  LPT_DYNSTRING, END_DYNLPT
};

// struct which holds the information about a property (name, type)
// the information is retrieved from PVSS and converted to this struct or set by 
// the user
struct TPropertyInfo
{
  string         propName;
  TMACValueType  type;
  TPropertyInfo() : propName(), type(NO_LPT) {};
  TPropertyInfo(const char* pPropName, TMACValueType atype) : propName(pPropName), type(atype) {};
  TPropertyInfo(const TPropertyInfo& other)
  {
    if (this != &other)
    {
      type = other.type;
      propName.replace(0, string::npos, other.propName);
    }
  }; 
};

// PropertySet category
typedef enum TPSCategory
{
  // corresponding DP does not exists
  // will be created on the first load request
  PS_CAT_TEMPORARY, 
  // corresponding DP must exists
  // only the usecount will be increased on load 
  PS_CAT_PERMANENT, 
  // corresponding DP must exists 
  // will be loaded automatically right after handled enable request in the PA
  PS_CAT_PERM_AUTOLOAD,  
  // corresponding DP does not exists
  // will be loaded automatically right after handled enable request in the PA
  PS_CAT_TEMP_AUTOLOAD,  
};

enum TKVLOrigin 
{
  KVL_NO_ORIGIN, 
  KVL_ORIGIN_MAC,
  KVL_ORIGIN_SHM,
  KVL_ORIGIN_OPERATOR
};

  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
#endif
