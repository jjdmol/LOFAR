//# BaseSim.h:
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef CEPFRAME_BASESIM_H
#define CEPFRAME_BASESIM_H

#include <lofar_config.h>

#define MPI_OPT

/// Node of the MPI controller.
const int CONTROLLER_NODE = 0;

// Define correct transporter mechanism.
#ifdef HAVE_MPI
# define TRANSPORTER TH_MPI    // to MPI or  not to MPI ..
# define TRANSPORTERINCLUDE "CEPFrame/TH_MPI.h"
#else // use MPI
# define TRANSPORTER TH_Mem    // to MPI or  not to MPI ..
# define TRANSPORTERINCLUDE "CEPFrame/TH_Mem.h"
#endif // MPI


#endif
