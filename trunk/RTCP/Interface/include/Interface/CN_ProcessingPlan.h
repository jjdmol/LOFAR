//#  Copyright (C) 2009
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
//#  $Id: Mutex.h 13919 2009-09-02 21:28:43Z romein $

#ifndef LOFAR_INTERFACE_CN_PROCESSINGPLAN_H
#define LOFAR_INTERFACE_CN_PROCESSINGPLAN_H

#include <vector>
#include <cassert>
#include <Interface/ProcessingPlan.h>
#include <Interface/CN_Configuration.h>

#include <Interface/InputData.h>
#include <Interface/TransposedData.h>
#include <Interface/FilteredData.h>
#include <Interface/PencilBeamData.h>
#include <Interface/CorrelatedData.h>
#include <Interface/StokesData.h>

namespace LOFAR {

namespace RTCP {

/*
 * CN_ProcessingPlan: contains the layout of the processing pipeline, as specified by a CN_Configuration.
 *
 * All data set holders are created and managed by this class, but allocating the data sets is left
 * to the caller. A CN_ProcessingPlan will in its destructor destroy the data sets and holders.
 */

// SAMPLE_TYPE matters only for handling input, not output
template <typename SAMPLE_TYPE = i8complex> class CN_ProcessingPlan: public ProcessingPlan
{
  public:
    CN_ProcessingPlan( CN_Configuration &configuration, bool hasPhaseOne = false, bool hasPhaseTwo = true, bool hasPhaseThree = true );
    virtual ~CN_ProcessingPlan();

    InputData<SAMPLE_TYPE>	 *itsInputData;
    SubbandMetaData              *itsInputSubbandMetaData;
    SubbandMetaData              *itsSubbandMetaData;
    TransposedData<SAMPLE_TYPE>	 *itsTransposedData;
    FilteredData		 *itsFilteredData;
    CorrelatedData		 *itsCorrelatedData;
    BeamFormedData               *itsBeamFormedData;
    StokesData                   *itsCoherentStokesData;
    StokesData                   *itsIncoherentStokesData;
    StokesDataIntegratedChannels *itsCoherentStokesDataIntegratedChannels;
    StokesDataIntegratedChannels *itsIncoherentStokesDataIntegratedChannels;
};

}

}

#endif
