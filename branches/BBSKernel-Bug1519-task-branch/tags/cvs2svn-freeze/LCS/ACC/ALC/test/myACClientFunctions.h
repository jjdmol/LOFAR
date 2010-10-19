//#  myACClientFunctions.h: Implements the async functions of the AC client
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

#ifndef ACC_MYACCLIENTFUNCTIONS_H
#define ACC_MYACCLIENTFUNCTIONS_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <ALC/ACAsyncClient.h>

namespace LOFAR {
  namespace ACC {
    namespace ALC {

// Description of class.
class myACClientFunctions : public ACClientFunctions
{
	string	supplyInfoFunc(const string&	keyList)
		{ return ("myACClientFunctions::supplyInfo from ACClient was called"); }

	void	handleAnswerMsg(const string&	answer)
		{ cout << "myACClientFunctions::handleAnswerMessage from ACClient was called"; 
		  cout << "Answer=" << answer << endl;
	    }

	void	handleAckMsg(ACCmd	cmd, uint16 result, const string& info)
		{ cout << "myACClientFunctions::handleAckMessage was called" << endl; 
		  cout << "command = " << cmd << ", result = " << result
				<< ", info = " << info << endl;
		}

};

    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

#endif
