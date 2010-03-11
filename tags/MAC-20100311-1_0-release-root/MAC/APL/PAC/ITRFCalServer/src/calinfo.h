//#  calinfo.h: one_line_description
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
//#  $Id: calinfo.h 13125 2009-04-19 12:32:55Z overeem $

#ifndef LOFAR_CAL_CALINFO_H
#define LOFAR_CAL_CALINFO_H

// \file calinfo.h
// one_line_description

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <GCF/TM/GCF_Control.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  using GCF::TM::GCFTask;
  using GCF::TM::GCFPortInterface;
  using GCF::TM::GCFTCPPort;
  namespace CAL {

// \addtogroup CAL
// @{

// class_description
// ...
class calinfo : public GCFTask
{
public:
	explicit calinfo(const string&  name);
	~calinfo();

	GCFEvent::TResult	initial		 (GCFEvent&	event, GCFPortInterface&	port);
	GCFEvent::TResult  	getInfo	 (GCFEvent&	event, GCFPortInterface&	port);
	GCFEvent::TResult  	finish		 (GCFEvent&	event, GCFPortInterface&	port);

private:
	// Copying is not allowed
	calinfo(const calinfo&	that);
	calinfo& operator=(const calinfo& that);

	//# --- Datamembers ---
	string			itsSAname;
	GCFTCPPort		itsCalPort;
};

// @}
  } // namespace CAL
} // namespace LOFAR

#endif
