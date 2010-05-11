//#  PR_BGL.h: ProcesRule based for BG/L jobs
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACCBIN_PR_BGL_H
#define LOFAR_ACCBIN_PR_BGL_H

// \file
// ProcessRule based on shell scripts

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include "ProcRule.h"

namespace LOFAR {
  namespace ACC {
    // \addtogroup ACCbin
    // @{

    // The PR_BGL class contains all information to (over)rule a process.
    // Its known how to start and stop a process and knows its current state.
    class PR_BGL : public ProcRule {
    public:
      PR_BGL(const string& aJobName, 
	     const string& aPartition,
	     const string& aExecutable,
	     const string& aWorkingDir,
             const string& aObservationID,
	     const string& aParamfile,
	     const uint numberOfNodes);

      PR_BGL(const PR_BGL& that);

      ~PR_BGL() {};

      // Redefine the start and stop commands.
      virtual bool start();
      virtual bool stop();
      virtual string getType() const
	{ return ("PR_BGL"); }
      virtual PR_BGL* clone() const
	{ return (new PR_BGL(*this)); }

    private:
      // Default construction not allowed
      PR_BGL();
      
      //# --- Datamembers ---
    };

    // @} addgroup
  } // namespace ACC
} // namespace LOFAR

#endif
