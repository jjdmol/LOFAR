//# BaseSim.h:
//#
//# Copyright (C) 2000, 2001
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


//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#ifndef LOFAR_TRANSPORT_BASESIM_H
#define LOFAR_TRANSPORT_BASESIM_H

#define MPI_OPT

/// Node of the MPI controller.
const int CONTROLLER_NODE = 0;

// Define correct transporter mechanism.
#ifdef HAVE_MPI
# define TRANSPORTER TH_MPI    // to MPI or  not to MPI ..
# define TRANSPORTERINCLUDE <Transport/TH_MPI.h>
#else // don't use MPI
# define TRANSPORTER TH_Mem    // to MPI or  not to MPI ..
# define TRANSPORTERINCLUDE <Transport/TH_Mem.h>
#endif // MPI


#endif
