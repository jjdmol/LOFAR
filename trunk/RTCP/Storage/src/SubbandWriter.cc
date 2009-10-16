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
#include <Interface/Exceptions.h>
#include <Interface/CorrelatedData.h>

#ifdef USE_MAC_PI
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#endif

#include <boost/lexical_cast.hpp>

#include <time.h>
#include <sys/stat.h>

namespace LOFAR {
namespace RTCP {

SubbandWriter::SubbandWriter(const Parset *ps, unsigned rank, unsigned size) 
:
  itsPS(ps),
  itsRank(rank),
  itsSize(size),
  itsObservationID(ps->observationID()),
  itsConfiguration(*ps),
  itsPlan(itsConfiguration,false,true),
  itsNrOutputs(itsPlan.nrOutputs())
#ifdef USE_MAC_PI
,itsPropertySet(0)
#endif
{
  itsPlan.removeNonOutputs();

#ifdef USE_MAC_PI
  itsWriteToMAC = itsPS.getBool("Storage.WriteToMAC");
#endif
  if (itsPS->nrTabStations() > 0) {
    itsNStations = itsPS->nrTabStations();
  } else {
    itsNStations = itsPS->nrStations();
  }
}


SubbandWriter::~SubbandWriter() 
{
#ifdef USE_MAC_PI
  delete itsPropertySet;

  GCF::Common::GCFPValueArray::iterator it;
  for (it = itsVArray.begin(); it != itsVArray.end(); it++){
    delete *it;
  }
  itsVArray.clear();
#endif
}


void SubbandWriter::preprocess() 
{
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

  double startTime = itsPS->startTime();
  LOG_TRACE_VAR_STR("startTime = " << startTime);
  
  vector<double> antPos = itsPS->positions();
  ASSERTSTR(antPos.size() == 3 * itsNStations,
	    antPos.size() << " == " << 3 * itsNStations);
  itsNrSubbands           = itsPS->nrSubbands();
  itsNrSubbandsPerPset	  = itsPS->nrSubbandsPerPset();
  if(itsNrSubbands % itsSize == 0) {
    itsNrSubbandsPerStorage = itsNrSubbands / itsSize;
  } else {
    itsNrSubbandsPerStorage = (itsNrSubbands / itsSize) + 1;
  }

  const double sampleFreq   = itsPS->sampleRate();
  const unsigned seconds    = static_cast<unsigned>(floor(startTime));
  const unsigned samples    = static_cast<unsigned>((startTime - floor(startTime)) * sampleFreq);

  itsStartStamp.setStationClockSpeed(static_cast<unsigned>(sampleFreq * 1024));
  itsStartStamp = TimeStamp(seconds, samples);

  LOG_TRACE_VAR_STR("SubbandsPerStorage = " << itsNrSubbandsPerStorage);

  itsMyNrSubbands = 0;
  for (unsigned i = 0; i < itsNrSubbandsPerStorage; i ++) {
    unsigned currentSubband = itsRank * itsNrSubbandsPerStorage + i;
    if(currentSubband < itsNrSubbands) {
      itsMyNrSubbands++;
    }
  }

  if( itsPS->outputCorrelatedData()) {
    MeasurementSetFormat myFormat(itsPS, 512);
    // create root directory of the observation tree
    if ( (mkdir(itsPS->getMSBaseDir().c_str(), 0770) != 0) && (errno != EEXIST) ) {
      LOG_FATAL_STR("failed to create directory " << itsPS->getMSBaseDir());
      perror("mkdir");
      exit(1);
    }
          
    for (unsigned sb = 0; sb < itsMyNrSubbands; sb++) {
      /// Make MeasurementSet filestructures and required tables
      myFormat.addSubband(itsRank * itsNrSubbandsPerStorage + sb);
    }

    // clean up the format object, so we don't carry this overhead into
    // the online part of the application.
    LOG_INFO_STR("MeasuremetSet created");
  }

  LOG_TRACE_VAR_STR("Subbands per storage = " << itsNrSubbandsPerStorage << ", I will store " 
		    << itsMyNrSubbands << " subbands, nrOutputs = " << itsNrOutputs);

#endif // defined HAVE_AIPSPP

  for (unsigned sb = 0; sb < itsMyNrSubbands; sb ++) {
    unsigned currentSubband = itsRank * itsNrSubbandsPerStorage + sb;

    InputThread *i = new InputThread(itsPS, currentSubband);
    OutputThread *o = new OutputThread(itsPS, currentSubband, i, itsNrOutputs, itsPlan);

    itsInputThreads.push_back(i);
    itsOutputThreads.push_back(o);
  }
}

void SubbandWriter::process() 
{
  // nothing to do.. postprocess() will wait for the threads to finish
}

void SubbandWriter::postprocess() 
{
  for (unsigned sb = 0; sb < itsMyNrSubbands; sb ++) {
    delete itsOutputThreads[sb];
    delete itsInputThreads[sb];
  }

  itsInputThreads.clear();
}

} // namespace RTCP
} // namespace LOFAR
