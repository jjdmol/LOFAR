//# ACSyncClient.cc: Implements the synchroon I/F of the ApplController.
//#
//# Copyright (C) 2004
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

#include <lofar_config.h>

//# Includes
#include <ALC/ACSyncClient.h>

namespace LOFAR {
  namespace ACC {
    namespace ALC {

ACSyncClient::ACSyncClient(const string&	aUniqUserName,
				  		   uint16			aNrProcs,
				  		   uint32			aExpectedLifeTime,
				  		   uint16			anActivityLevel,
				  		   uint16			anArchitecture,
						   const string&	hostname) :
 	ApplControlClient(aUniqUserName, 
					  aNrProcs, 
					  aExpectedLifeTime,
					  anActivityLevel,
					  anArchitecture, 
					  true,
					  hostname)
{
}

// Destructor
ACSyncClient::~ACSyncClient() 
{
}

//# ---------- private ----------
// NOT default constructable;
ACSyncClient::ACSyncClient() { }


    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

