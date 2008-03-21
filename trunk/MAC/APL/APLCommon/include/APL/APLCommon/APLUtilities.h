//#  APLUtilities.h: Utility functions
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

#ifndef APLUtilities_H
#define APLUtilities_H

//# Includes
#include <time.h>
//# Common Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

//# GCF Includes

//# local includes
#include "APL/APLCommon/APL_Defines.h"

// forward declaration

namespace LOFAR { 
  namespace APLCommon {

class APLUtilities
{
public:

    APLUtilities(); 
    virtual ~APLUtilities();

	// Break a PVSS commandstring into a command with args
    static void		decodeCommand (const string& commandString, 
								   string& command, 
								   vector<string>& parameters,
								   const char delimiter=',');

	// Break a ParameterSet array into a vector of values.
    static void		string2Vector (const string& parametersString, 
								   vector<string>& parameters, 
								   const char delimiter=','); 
    static void		string2Vector (const string& parametersString, 
								   vector<int>& parameters, 
								   const char delimiter=','); 
    static void		string2Vector (const string& parametersString, 
								   vector<int16>& parameters, 
								   const char delimiter=','); 
	
	// Construct a ParameterSet array from a vector of strings
    static void		vector2String (const vector<int16>& parameters, 
								   string& parametersString, 
								   const char delimiter=','); 

	// time utilities
    static time_t	getUTCtime();
    static time_t	decodeTimeString(const string& timeStr);

	// Copy file to remote machine
    static int		remoteCopy	  (const string& localFile, 
								   const string& remoteHost, 
								   const string& remoteFile);
	static int      copyFromRemote(const string& remoteHost, 
								   const string& remoteFile,
								   const string& localFile);
    static string	getTempFileName(const string&	format="");

protected:
    // protected copy constructor
    APLUtilities(const APLUtilities&);
    // protected assignment operator
    APLUtilities& operator=(const APLUtilities&);

private:
};

string	byteSize(double		nrBytes);

};//APL
};//LOFAR
#endif
