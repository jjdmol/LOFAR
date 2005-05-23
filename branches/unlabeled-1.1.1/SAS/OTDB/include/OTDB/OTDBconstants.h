//#  OTDBconstants.h: Collection of constants also available in the DB.
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

#ifndef LOFAR_OTDB_OTDBCONSTANTS_H
#define LOFAR_OTDB_OTDBCONSTANTS_H

// \file OTDBconstants.h
// Collection of constants also available in the DB.

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

// Enumeration of tree classifications. Used in VIC and PIC trees.
const int16	TC_all				= 0;
const int16	TC_experimental		= 1;
const int16	TC_operational		= 2;
const int16	TC_obsolete			= 3;


// Enumeration of parameter types. Used in VIC and PIC trees.
const int16 PT_node				= 0;		// not a parameter
const int16	PT_bool				= 101;
const int16	PT_int				= 102;
const int16	PT_long				= 103;
const int16	PT_float			= 104;
const int16	PT_double			= 105;
const int16	PT_icomplex			= 106;
const int16	PT_lcomplex			= 107;
const int16	PT_fcomplex			= 108;
const int16	PT_dcomplex			= 109;
const int16	PT_text				= 110;
const int16	PT_bin				= 111;


// Enumeration of tree types.
const int16	TT_hardware			= 10;
const int16	TT_template			= 20;
const int16	TT_configure		= 30;
const int16	TT_schedule			= 40;
const int16	TT_queued			= 50;
const int16	TT_active			= 60;
const int16	TT_finished			= 70;
const int16	TT_obsolete			= 80;


// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
