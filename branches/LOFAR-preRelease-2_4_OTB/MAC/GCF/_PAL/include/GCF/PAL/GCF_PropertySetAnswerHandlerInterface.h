//#  GCF_PropertySetAnswerHandlerInterface.h: I/F for the propertyset answer handler.
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

#ifndef GCF_PROPERTYSETANSWERHANDLERINTERFACE_H
#define GCF_PROPERTYSETANSWERHANDLERINTERFACE_H

//# GCF Includes
#include <GCF/TM/GCF_Event.h>

namespace LOFAR {
  namespace GCF {
    namespace PAL {

class GCFPropertySetAnswerHandlerInterface
{
public:
	GCFPropertySetAnswerHandlerInterface() {}; 
	virtual ~GCFPropertySetAnswerHandlerInterface() {};

	// PropertySet answer handling is implemented in the derived classes. 
	virtual void handlePropertySetAnswer(GCF::TM::GCFEvent& answer)=0;

protected:
	// protected copy constructor
	GCFPropertySetAnswerHandlerInterface(const GCFPropertySetAnswerHandlerInterface&);
	// protected assignment operator
	GCFPropertySetAnswerHandlerInterface& operator=(const GCFPropertySetAnswerHandlerInterface&);

};

    }; // PAL
  }; // GCF
}; // LOFAR
#endif
