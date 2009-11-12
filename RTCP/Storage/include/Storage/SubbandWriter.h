//#  SubbandWriter.h: Write subband(s) in an AIPS++ Measurement Set
//#
//#  Copyright (C) 2002-2005
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

#ifndef LOFAR_STORAGE_SUBBANDWRITER_H
#define LOFAR_STORAGE_SUBBANDWRITER_H

// \file
// Write subband(s) in an AIPS++ Measurement Set

#include <Blob/KeyValueMap.h>
#include <Interface/Config.h>
#ifdef USE_MAC_PI
# include <GCF/PALlight/CEPPropertySet.h>
# include <GCF/GCF_PVDynArr.h>
#endif
#include <Common/Timer.h>
#include <Common/lofar_vector.h>
#include <Interface/Parset.h>
#include <Interface/CN_Configuration.h>
#include <Interface/CN_ProcessingPlan.h>
#include <Interface/StreamableData.h>
#include <Interface/Queue.h>
#include <Storage/InputThread.h>
#include <Storage/OutputThread.h>
#include <Storage/MSWriter.h>
#include <Stream/Stream.h>


namespace LOFAR {
namespace RTCP {

class SubbandWriter
{
  public:
			    SubbandWriter(const Parset *, unsigned rank, unsigned size);
			    ~SubbandWriter();

    void		    preprocess();
    void		    process();
    void		    postprocess();

  private:
    const Parset	    *itsPS;
    unsigned		    itsRank;
    unsigned		    itsSize;
    unsigned		    itsObservationID;
    CN_Configuration        itsConfiguration;
    CN_ProcessingPlan<>     itsPlan;
    unsigned                itsNrOutputs;

    std::vector<InputThread *> itsInputThreads;
    std::vector<OutputThread *> itsOutputThreads;

#ifdef USE_MAC_PI
    bool itsWriteToMAC;
    GCF::CEPPMLlight::CEPPropertySet* itsPropertySet;
    GCF::Common::GCFPValueArray itsVArray; 
#endif
};

} // namespace RTCP
} // namespace LOFAR

#endif
