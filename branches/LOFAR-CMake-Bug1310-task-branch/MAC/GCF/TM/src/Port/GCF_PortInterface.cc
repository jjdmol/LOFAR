//#  GCF_PortInterface.cc: container class for all port implementations
//#
//#  Copyright (C) 2002-2003
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <sys/types.h>
#include <sys/uio.h>
#include <string.h>

#include <Common/StringUtil.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_PortInterface.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {

//
// ~GCFPortInterface()
//
GCFPortInterface::~GCFPortInterface () 
{ }

//
// GCFPortInterface(task, name, type, protocol ,raw)
//
GCFPortInterface::GCFPortInterface (GCFTask*		pTask, 
								    const string&	name, 
								    TPortType		type, 
								    int				protocol, 
								    bool			transportRawData) :
	_pTask(pTask), 
	_name(name), 
	_deviceNameMask(),
	_state(S_DISCONNECTED), 
	_type(type), 
	_protocol(protocol),
	_transportRawData(transportRawData),
	_instanceNr(0)
{
	if (_name.find(":",0) != string::npos) {	// colon in _name?
		_deviceNameMask = _name;				// name is also the mask
	}
}

//
// init(task, name, type, protocol ,raw)
//
void GCFPortInterface::init(GCFTask& 		task, 
						    const string&	name, 
						    TPortType	 	type,  
						    int 			protocol, 
						    bool 			transportRawData)
{
	if (_state == S_DISCONNECTED) {
		_pTask 				= &task;
		_name 				= name;  
		_type 				= type;
		_protocol 			= protocol;
		_transportRawData 	= transportRawData;
		_instanceNr 		= 0;
		if (_name.find(":",0) != string::npos) {	// colon in _name?
			_deviceNameMask = _name;				// name is also the mask
		}
		else {
			_deviceNameMask	= "";					// clear mask
		}
	}
}

//
// makeServiceName()
//
// Construct the ServiceName from the taskname and the _name OR
// from the instanceNr and the _name
//
string GCFPortInterface::makeServiceName() const
{
	if (!_deviceNameMask.empty()) {
		// use instanceNr and _name to construct the servicename
		string	instanceNrStr;
		if (_instanceNr) {
			instanceNrStr = toString(_instanceNr);
		}
		return(formatString(_deviceNameMask.c_str(), instanceNrStr.c_str()));
	}

	// no colon in _name, use old-style taskname and _name construction
	return(formatString("%s:%s", _pTask->getName().c_str(), _name.c_str()));
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
