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
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

//# GCF Includes

//# local includes

// forward declaration

namespace LOFAR
{
  
namespace APLCommon
{

class APLUtilities
{
  public:

    APLUtilities(); 
    virtual ~APLUtilities();

    static void decodeCommand(const string& commandString, string& command, vector<string>& parameters);
    static void string2Vector(const string& parametersString, vector<string>& parameters); 
    static time_t getUTCtime();
    static time_t decodeTimeString(const string& timeStr);
    static int remoteCopy(const string& localFile, const string& remoteHost, const string& remoteFile);
    static string getTempFileName();
    
  protected:
    // protected copy constructor
    APLUtilities(const APLUtilities&);
    // protected assignment operator
    APLUtilities& operator=(const APLUtilities&);

  private:

    ALLOC_TRACER_CONTEXT  
};

};//APL
};//LOFAR
#endif
