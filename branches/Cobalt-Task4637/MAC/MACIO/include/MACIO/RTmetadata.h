//#  RTmetadata.h: LCS-Common-Socket based impl to store metadata in PVSS.
//#
//#  Copyright (C) 2013
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
//#  $Id: RTmetadata.h 11220 2008-05-14 10:04:13Z overeem $

#ifndef LOFAR_MACIO_RTMETADATA_H
#define LOFAR_MACIO_RTMETADATA_H

// \file RTmetadata.h
// LCS-Common-Socket based impl to store metadata in PVSS.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <MACIO/GCF_Event.h>
#include <MACIO/EventPort.h>
#include <Common/KVpair.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace MACIO {

// \addtogroup MACIO
// @{


// The RTmetadata class is a LCS/Common Socket based TCP port to make it
// possible for CEP applications to store Key-Value_Timestamp values in PVSS
class RTmetadata
{
public:
	// RTmetadata (obsID, uniqID_name_in_SAS_tree)
	RTmetadata(uint32			observationID,
			   const string&	registerName, 
			   const string&	hostname = "");

	// ~RTmetadata
	~RTmetadata();

	// log()
	// KVpair based functions.
	bool log(const KVpair& aPair);
	bool log(const vector<KVpair> pairs);

	// string based functions
	template <typename T> 
		inline bool log(const string& key, const T& value, double secsEpoch1970=0.0)
		{ return (log(KVpair(key, value, secsEpoch1970))); }
	bool log(const vector<string> key, const vector<string> value, const vector<double> times);

private:
	RTmetadata();
	// Copying is not allowed
	RTmetadata(const RTmetadata&	that);
	RTmetadata& operator=(const RTmetadata& that);

	bool _setupConnection();

	//# --- Datamembers ---
	uint32			itsObsID;
	string			itsRegisterName;
	bool			itsLoggingEnabled;
	int32			itsSeqnr;
	EventPort*		itsKVTport;
};


// @}
  } // namespace MACIO
} // namespace LOFAR

#endif
