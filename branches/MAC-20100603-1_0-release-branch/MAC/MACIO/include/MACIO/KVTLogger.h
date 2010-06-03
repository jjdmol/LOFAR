//#  KVTLogger.h: LCS-Common-Socket based impl to store KVT triples in SAS
//#
//#  Copyright (C) 2010
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
//#  $Id: KVTLogger.h 11220 2008-05-14 10:04:13Z overeem $

#ifndef LOFAR_MACIO_KVTLOGGER_H
#define LOFAR_MACIO_KVTLOGGER_H

// \file KVTLogger.h
// LCS-Common-Socket based impl to store KVT triples in SAS.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <MACIO/GCF_Event.h>
#include <MACIO/EventPort.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace MACIO {

// \addtogroup MACIO
// @{


// The KVTLogger class is a LCS/Common Socket based TCP port to make it
// possible for CEP applications to store Key-Value_Timestamp values in SAS
class KVTLogger
{
public:
	// KVTLogger (obsID, uniqID_name_in_SAS_tree)
	KVTLogger(uint32		observationID,
			  const string&	registerName, 
			  const string&	hostname = "", 
			  bool			syncCommunication = false);

	// ~KVTLogger
	~KVTLogger();

	// log()
	void log(const string& key, const string& value, double secsEpoch1970);

private:
	KVTLogger();
	// Copying is not allowed
	KVTLogger(const KVTLogger&	that);
	KVTLogger& operator=(const KVTLogger& that);

	bool _setupConnection();

	//# --- Datamembers ---
	uint32			itsObsID;
	string			itsRegisterName;
	bool			itsLoggingEnabled;
	uint32			itsSeqnr;
	EventPort*		itsKVTport;
};


// @}
  } // namespace MACIO
} // namespace LOFAR

#endif
