//#  AH_Generator.h: Application that generates data (and emulates an RSP board)
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_GENERATOR_AH_GENERATOR_H
#define LOFAR_GENERATOR_AH_GENERATOR_H

// \file
// Data generator

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <CEPFrame/ApplicationHolder.h>
#include <Transport/TransportHolder.h>

namespace LOFAR 
{
  namespace Generator 
  {

    // \addtogroup Generator
    // @{

    class AH_Generator: public LOFAR::ApplicationHolder
      {
      public:
	AH_Generator();
	virtual ~AH_Generator();
	virtual void undefine();
	virtual void define  (const LOFAR::KeyValueMap&);
	virtual void run     (int nsteps);
      private:
	vector<TransportHolder*> itsTHs;
      };

    // @}

  } // namespace Generator
} // namespace LOFAR

#endif
