//  BaseSim.h:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//  $Log$
//  Revision 1.16  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.15  2002/03/01 08:27:56  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.14  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.13  2001/09/24 14:04:08  gvd
//  Added preprocess and postprocess functions
//
//  Revision 1.12  2001/09/18 12:07:28  gvd
//  Changed to resolve Step and Simul memory leaks
//  Introduced ref.counted StepRep and SimulRep classes for that purposes
//  Changed several functions to pass by reference instead of pass by pointer
//
//  Revision 1.11  2001/09/05 15:01:23  wierenga
//  Use MPI_ instead of NOMPI_
//
//  Revision 1.10  2001/08/16 14:33:07  gvd
//  Determine TransportHolder at runtime in the connect
//
//  Revision 1.9  2001/08/09 15:48:47  wierenga
//  Implemented first version of TH_Corba and test program
//
//  Revision 1.8  2001/06/22 09:09:30  schaaf
//  Use TRANSPORTERINCLUDE to select the TH_XXX.h include files
//
//  Revision 1.7  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.6  2001/02/05 14:53:05  loose
//  Added GPL headers
//

#ifndef BASESIM_H
#define BASESIM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define MPI_OPT

/// Node of the MPI controller.
const int CONTROLLER_NODE = 0;

// Define correct transporter mechanism.
#ifdef HAVE_MPI
# define TRANSPORTER TH_MPI    // to MPI or  not to MPI ..
# define TRANSPORTERINCLUDE "BaseSim/TH_MPI.h"
#else // use MPI
# define TRANSPORTER TH_Mem    // to MPI or  not to MPI ..
# define TRANSPORTERINCLUDE "BaseSim/TH_Mem.h"
#endif // MPI


#endif
