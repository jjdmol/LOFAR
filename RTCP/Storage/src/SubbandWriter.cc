//#  SubbandWriter.cc: Writes visibilities in an AIPS++ measurement set
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

#include <lofar_config.h>

#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>
#include <Storage/SubbandWriter.h>
#include <Storage/MeasurementSetFormat.h>
#include <Stream/SystemCallException.h>
#include <Interface/Exceptions.h>
#include <Interface/CN_Configuration.h>
#include <Interface/CN_ProcessingPlan.h>

#ifdef USE_MAC_PI
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#endif

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

using boost::format;

#include <time.h>
#include <sys/stat.h>

namespace LOFAR {
namespace RTCP {

SubbandWriter::SubbandWriter(const Parset *ps, unsigned rank, unsigned size) 
:
  itsPS(ps),
  itsRank(rank),
  itsSize(size),
  itsObservationID(ps->observationID())
#ifdef USE_MAC_PI
,itsPropertySet(0)
#endif
{
#ifdef USE_MAC_PI
  itsWriteToMAC = itsPS.getBool("Storage.WriteToMAC");
#endif

  CN_Configuration configuration(*itsPS);
  CN_ProcessingPlan<> plan(configuration);
  plan.removeNonOutputs();

  // determine the indices for both phase 2 and 3.
  std::vector<unsigned> phaseTwoSubbands;
  std::vector<unsigned> subbandStorageList = itsPS->subbandStorageList();
  for (unsigned sb = 0; sb < subbandStorageList.size(); sb++ ) {
    if (subbandStorageList[sb] == itsRank ) {
      phaseTwoSubbands.push_back( sb );
    }
  }

  std::vector<unsigned> phaseThreeBeams;
  std::vector<unsigned> beamStorageList = itsPS->beamStorageList();
  for (unsigned sb = 0; sb < beamStorageList.size(); sb++ ) {
    if (beamStorageList[sb] == itsRank ) {
      phaseThreeBeams.push_back( sb );
    }
  }


#if defined HAVE_AIPSPP
  LOG_TRACE_FLOW("SubbandWriter enabling PropertySet");
#ifdef USE_MAC_PI
  if (itsWriteToMAC) {
    itsPropertySet = new GCF::CEPPMLlight::CEPPropertySet("CEP_TFCD", "TTeraFlopCorrelator", GCF::Common::PS_CAT_PERMANENT);
    itsPropertySet->enable();
    LOG_TRACE_FLOW("SubbandWriter PropertySet enabled");
  } else {
    LOG_TRACE_FLOW("SubbandWriter PropertySet not enabled");
  }
#endif

  if (!phaseTwoSubbands.empty() && itsPS->outputCorrelatedData()) {
    MeasurementSetFormat myFormat(itsPS, 512);

    // create root directory of the observation tree
    if ( (mkdir(itsPS->getMSBaseDir().c_str(), 0770) != 0) && (errno != EEXIST) ) {
      throw SystemCallException(("mkdir " + itsPS->getMSBaseDir()).c_str(), errno, THROW_ARGS);
    }
          
    for (unsigned i = 0; i < phaseTwoSubbands.size(); i++) {
      /// Make MeasurementSet filestructures and required tables
      myFormat.addSubband(phaseTwoSubbands[i]);
    }

    LOG_INFO_STR("MeasurementSet created");
  }

#endif // defined HAVE_AIPSPP

  for (unsigned outputNr = 0; outputNr < plan.nrOutputTypes(); outputNr ++) {
    ProcessingPlan::planlet &outputConfig = plan.plan[outputNr];
    StreamableData *dataTemplate = outputConfig.source;
    std::vector<unsigned> list;

    switch( outputConfig.distribution ) {
      case ProcessingPlan::DIST_SUBBAND:
        list = phaseTwoSubbands;
        break;

      case ProcessingPlan::DIST_BEAM:
        list = phaseThreeBeams;
        break;

      default:  
        continue;
    }

    for (unsigned i = 0; i < list.size(); i++ ) {
      InputThread *in = new InputThread(itsPS, list[i], outputNr, dataTemplate);
      OutputThread *out = new OutputThread(itsPS, list[i], outputNr, in, outputConfig);

      itsInputThreads.push_back(in);
      itsOutputThreads.push_back(out);
    }
  }
}


SubbandWriter::~SubbandWriter() 
{
  // wait for all threads to finish

  for (unsigned i = 0; i < itsInputThreads.size(); i++ )
    delete itsInputThreads[i];

  itsInputThreads.clear();

  for (unsigned i = 0; i < itsOutputThreads.size(); i++ )
    delete itsOutputThreads[i];

  itsOutputThreads.clear();


#ifdef USE_MAC_PI
  delete itsPropertySet;

  GCF::Common::GCFPValueArray::iterator it;
  for (it = itsVArray.begin(); it != itsVArray.end(); it++){
    delete *it;
  }
  itsVArray.clear();
#endif
}

} // namespace RTCP
} // namespace LOFAR
