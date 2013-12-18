//#  wSpaceSplit.h: Splits a string in substrings with whitespace as separator.
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

#ifndef LOFAR_OTDB_WSPACESPLIT_H
#define LOFAR_OTDB_WSPACESPLIT_H

// \file
// Splits a string in substrings with whitespace as seperator.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

// Splits the given string into a vector of substring using white space
// as the seperator between the elements.
// When the max number of substrings is limited the 'remainder' of the input-
// string is returned as the last element.
//
// Syntax: <element> <whitespace>  <element> <whitespace>  <element> ...
//
// Each element may be placed between single or double qoutes.
//
vector<string> wSpaceSplit(const string& targetStr, uint16	maxStrings = 16384);


// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
