//# KVpair.h: Implements a KV pair as a pair<string, string>.
//#
//# Copyright (C) 2002-2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_COMMON_KVPAIR_H
#define LOFAR_COMMON_KVPAIR_H

// \file
// Implements a KV pair as a pair<string, string>.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>


namespace LOFAR {
// \addtogroup Common
// @{

// Implements a KV pair as a pair<string, string>.
class KVpair : public pair<string, string>
{
public:
	KVpair(const string& aKey, const string& aValue, bool genTimestamp = false);
	KVpair(const string& aKey, bool			 aValue, bool genTimestamp = false);
	KVpair(const string& aKey, int			 aValue, bool genTimestamp = false);
	KVpair(const string& aKey, double		 aValue, bool genTimestamp = false);
	KVpair(const string& aKey, float		 aValue, bool genTimestamp = false);
	KVpair(const string& aKey, time_t		 aValue, bool genTimestamp = false);
	KVpair(const string& aKey, const vector<int>&    aValue, bool genTimestamp = false); 

	~KVpair();

	// Copying is allowed
	KVpair(const KVpair&	that);
	KVpair& operator=(const KVpair& that);
};

// @} addgroup
} // namespace LOFAR

#endif
