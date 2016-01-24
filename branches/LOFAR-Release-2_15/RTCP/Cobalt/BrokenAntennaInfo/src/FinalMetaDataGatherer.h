//# FinalMetaDataGatherer.h:
//# Copyright (C) 2012-2014
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

#ifndef LOFAR_BROKENANTENNNAINFO_FINALMETADATAGATHERER_H
#define LOFAR_BROKENANTENNNAINFO_FINALMETADATAGATHERER_H

#include <CoInterface/FinalMetaData.h>
#include <CoInterface/Parset.h>

namespace LOFAR {
  namespace Cobalt {
    /* Obtain and return FinalMetaData, as obtained through OTDB.
       
       The parset provides the start- and stop-time for which broken
       antenna info is obtained.
     */
    FinalMetaData getFinalMetaData( const Parset &parset );
  }
}

#endif
