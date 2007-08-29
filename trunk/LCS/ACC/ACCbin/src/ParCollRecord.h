//#  ParCollRecord.h: structure how PC's are stored in the DB
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACCBIN_PARCOLLRECORD_H
#define LOFAR_ACCBIN_PARCOLLRECORD_H

// \file
// Structure how PC's are stored in the DB

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_string.h>
#include <PL/TPersistentObject.h>
#include <dtl/dtl_config.h>

using dtl::blob;

namespace LOFAR {
  namespace ACC {
// \addtogroup ACCbin
// @{

// Structure how PC's are stored in the DB
class ParCollRecord
{
public:
	ParCollRecord () {};
	ParCollRecord (const string&	aName,
				   const string&	aVersion,
				   const string&	Aqualification,
				   const string&	aContents);
	int32 getLine(string&		buffer, uint32*	bufLen);
	inline string	getCollection() { return (itsCollection); }
	inline string	getVersionNr()  { return (itsVersionNr); }

private:
	friend class LOFAR::PL::TPersistentObject<ParCollRecord>;

	string		itsName;
	string		itsVersionNr;
	string		itsQualification;
	string		itsCollection;
};

// @} addgroup
} // namespace ACC
} // namespace LOFAR

#endif
