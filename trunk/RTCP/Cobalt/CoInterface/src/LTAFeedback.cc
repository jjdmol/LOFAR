//# LTAFeedback.cc
//# Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <CoInterface/LTAFeedback.h>
#include <CoInterface/TimeFuncs.h>

#include <string>
#include <boost/format.hpp>

using namespace std;
using boost::format;

namespace LOFAR
{
  namespace Cobalt
  {
    LTAFeedback::LTAFeedback(const ObservationSettings &settings):
      settings(settings)
    {
    }

    std::string LTAFeedback::correlatedPrefix(size_t fileno)
    {
      return str(format("Observation.DataProducts.Output_Correlated_[%u].") % fileno);
    }

    std::string LTAFeedback::beamFormedPrefix(size_t fileno)
    {
      return str(format("Observation.DataProducts.Output_Beamformed_[%u].") % fileno);
    }

    ParameterSet LTAFeedback::correlatedFeedback(size_t fileno) const
    {
      ParameterSet ps;

      const ObservationSettings::Correlator::File &f = settings.correlator.files[fileno];

      const string prefix = correlatedPrefix(fileno);

      ps.add(prefix + "fileFormat",           "AIPS++/CASA");
      ps.add(prefix + "filename",             f.location.filename);
      ps.add(prefix + "size",                 "0");
      ps.add(prefix + "location",             f.location.host + ":" + f.location.directory);

      ps.add(prefix + "percentageWritten",    "0");
      ps.add(prefix + "startTime",            TimeDouble::toString(settings.startTime, false));
      ps.add(prefix + "duration",             "0");
      ps.add(prefix + "integrationInterval",  str(format("%f") % settings.correlator.integrationTime()));
      ps.add(prefix + "centralFrequency",     str(format("%f") % settings.subbands[fileno].centralFrequency));
      ps.add(prefix + "channelWidth",         str(format("%f") % settings.correlator.channelWidth));
      ps.add(prefix + "channelsPerSubband",   str(format("%u") % settings.correlator.nrChannels));
      ps.add(prefix + "stationSubband",       str(format("%u") % settings.subbands[fileno].stationIdx));
      ps.add(prefix + "subband",              str(format("%u") % fileno));
      ps.add(prefix + "SAP",                  str(format("%u") % settings.subbands[fileno].SAP));

      return ps;
    }


    ParameterSet LTAFeedback::beamFormedFeedback(size_t fileno) const
    {
      ParameterSet ps;

      // construct feedback for LTA -- Implements Output_Beamformed_.comp
      const struct ObservationSettings::BeamFormer::File &f = settings.beamFormer.files.at(fileno);

      const string prefix = beamFormedPrefix(fileno);

      const struct ObservationSettings::BeamFormer::StokesSettings &stokesSet =
        f.coherent ? settings.beamFormer.coherentSettings
                   : settings.beamFormer.incoherentSettings;

      const struct ObservationSettings::SAP &sap =
        settings.SAPs.at(f.sapNr);
      const struct ObservationSettings::BeamFormer::TAB &tab =
        settings.beamFormer.SAPs.at(f.sapNr).TABs.at(f.tabNr);

      vector<string> stokesVars;

      switch (stokesSet.type) {
      case STOKES_I:
        stokesVars.push_back("I");
        break;

      case STOKES_IQUV:
        stokesVars.push_back("I");
        stokesVars.push_back("Q");
        stokesVars.push_back("U");
        stokesVars.push_back("V");
        break;

      case STOKES_XXYY:
        stokesVars.push_back("Xre");
        stokesVars.push_back("Xim");
        stokesVars.push_back("Yre");
        stokesVars.push_back("Yim");
        break;

      case INVALID_STOKES:
        THROW(CoInterfaceException, "MSWriterDAL asked to write INVALID_STOKES");
      }

      ps.add(prefix + "fileFormat",                "HDF5");
      ps.add(prefix + "filename",                  f.location.filename);
      ps.add(prefix + "size",                      "0");
      ps.add(prefix + "location",                  f.location.host + ":" + f.location.directory);
      ps.add(prefix + "percentageWritten",         "0");
      ps.add(prefix + "beamTypes",                 "[]");

      const string type = 
        settings.beamFormer.doFlysEye ? "FlysEyeBeam" : 
        (f.coherent ? "CoherentStokesBeam" : "IncoherentStokesBeam");

      ps.add(prefix + "nrOfCoherentStokesBeams",   "0");
      ps.add(prefix + "nrOfIncoherentStokesBeams", "0");
      ps.add(prefix + "nrOfFlysEyeBeams",          "0");
      ps.replace(prefix + str(format("nrOf%ss") % type), "1");

      const string beamPrefix = str(format("%s%s[0].") % prefix % type);

      ps.add(beamPrefix + "SAP",               str(format("%u") % f.sapNr));
      ps.add(beamPrefix + "TAB",               str(format("%u") % f.tabNr));
      ps.add(beamPrefix + "samplingTime",      str(format("%f") % (settings.sampleDuration() * stokesSet.nrChannels * stokesSet.timeIntegrationFactor)));
      ps.add(beamPrefix + "dispersionMeasure", str(format("%f") % tab.dispersionMeasure));
      ps.add(beamPrefix + "nrSubbands",        str(format("%u") % (f.lastSubbandIdx - f.firstSubbandIdx)));

      ostringstream centralFreqsStr;
      centralFreqsStr << "[";
      for (size_t i = f.firstSubbandIdx; i < f.lastSubbandIdx; ++i) {
        if (i > f.firstSubbandIdx)
          centralFreqsStr << ", ";
        centralFreqsStr << str(format("%.4lf") % settings.subbands[i].centralFrequency);
      }
      centralFreqsStr << "]";

      ps.add(beamPrefix + "centralFrequencies", centralFreqsStr.str());

      ostringstream stationSubbandsStr;
      stationSubbandsStr << "[";
      for (size_t i = f.firstSubbandIdx; i < f.lastSubbandIdx; ++i) {
        if (i > f.firstSubbandIdx)
          stationSubbandsStr << ", ";
        stationSubbandsStr << str(format("%u") % settings.subbands[i].stationIdx);
      }
      stationSubbandsStr << "]";

      ps.add(beamPrefix + "stationSubbands",  stationSubbandsStr.str());

      ps.add(beamPrefix + "channelWidth",      str(format("%f") % (settings.subbandWidth() / stokesSet.nrChannels)));
      ps.add(beamPrefix + "channelsPerSubband",str(format("%u") % stokesSet.nrChannels));
      ps.add(beamPrefix + "stokes",            str(format("[%s]") % stokesVars[f.stokesNr]));

      if (type == "CoherentStokesBeam") {
        ps.add(beamPrefix + "Pointing.equinox",   "J2000");
        ps.add(beamPrefix + "Pointing.coordType", "RA-DEC");
        ps.add(beamPrefix + "Pointing.angle1",    str(format("%f") % tab.direction.angle1));
        ps.add(beamPrefix + "Pointing.angle2",    str(format("%f") % tab.direction.angle2));

        ps.add(beamPrefix + "Offset.equinox",     "J2000");
        ps.add(beamPrefix + "Offset.coordType",   "RA-DEC");
        ps.add(beamPrefix + "Offset.angle1",      str(format("%f") % (tab.direction.angle1 - sap.direction.angle1)));
        ps.add(beamPrefix + "Offset.angle2",      str(format("%f") % (tab.direction.angle2 - sap.direction.angle2)));
      } else if (type == "FlysEyeBeam") {
        string fullName = settings.antennaFields.at(f.tabNr).name;
        string stationName = fullName.substr(0,5);
        string antennaFieldName = fullName.substr(5);
        ps.add(beamPrefix + "stationName", stationName);
        ps.add(beamPrefix + "antennaFieldName", antennaFieldName);
      }

      return ps;
    }


    ParameterSet LTAFeedback::allFeedback() const
    {
      ParameterSet ps;

      // for MoM, to discriminate between Cobalt and BG/P observations
      ps.add("_isCobalt", "T");

      ps.add("Observation.DataProducts.nrOfOutput_Correlated_", 
             str(format("%u") % (settings.correlator.enabled ? settings.correlator.files.size() : 0)));

      if (settings.correlator.enabled) {
        ps.add("Observation.Correlator.integrationInterval",
               str(format("%.16g") % settings.correlator.integrationTime()));

        // add the feedback for the individual files
        for (size_t i = 0; i < settings.correlator.files.size(); ++i) {
          ps.adoptCollection(correlatedFeedback(i));
        }
      }

      ps.add("Observation.DataProducts.nrOfOutput_Beamformed_", 
             str(format("%u") % (settings.beamFormer.enabled ? settings.beamFormer.files.size() : 0)));

      if (settings.beamFormer.enabled) {
        const ObservationSettings::BeamFormer::StokesSettings&
          coherentStokes = settings.beamFormer.coherentSettings;
        const ObservationSettings::BeamFormer::StokesSettings&
          incoherentStokes = settings.beamFormer.incoherentSettings;

        // The 'rawSamplingTime' is the duration of a sample right after the PPF
        ps.add("Observation.CoherentStokes.rawSamplingTime",
               str(format("%.16g") % 
                   (settings.sampleDuration() * coherentStokes.nrChannels)));
        ps.add("Observation.IncoherentStokes.rawSamplingTime",
               str(format("%.16g") % 
                   (settings.sampleDuration() * incoherentStokes.nrChannels)));

        // The 'samplingTime' is the duration of a sample in the output
        ps.add("Observation.CoherentStokes.samplingTime",
               str(format("%.16g") % 
                   (settings.sampleDuration() * coherentStokes.nrChannels * coherentStokes.timeIntegrationFactor)));
        ps.add("Observation.IncoherentStokes.samplingTime",
               str(format("%.16g") % 
                   (settings.sampleDuration() * incoherentStokes.nrChannels * incoherentStokes.timeIntegrationFactor)));

        ps.add("Observation.CoherentStokes.timeDownsamplingFactor",
               str(format("%.16g") % coherentStokes.timeIntegrationFactor));
        ps.add("Observation.IncoherentStokes.timeDownsamplingFactor",
               str(format("%.16g") % incoherentStokes.timeIntegrationFactor));

        // The BG/P could 'collapse channels'. Cobalt does not need that, so we
        // put fixed/trivial values here.
        ps.add("Observation.CoherentStokes.nrOfCollapsedChannels",
               str(format("%u") % coherentStokes.nrChannels));
        ps.add("Observation.IncoherentStokes.nrOfCollapsedChannels",
               str(format("%u") % incoherentStokes.nrChannels));
        ps.add("Observation.CoherentStokes.frequencyDownsamplingFactor", "1");
        ps.add("Observation.IncoherentStokes.frequencyDownsamplingFactor", "1");

        ps.add("Observation.CoherentStokes.stokes",
               stokesType(coherentStokes.type));
        ps.add("Observation.IncoherentStokes.stokes",
               stokesType(incoherentStokes.type));
        ps.add("Observation.CoherentStokes.antennaSet",
               settings.antennaSet);
        ps.add("Observation.IncoherentStokes.antennaSet",
               settings.antennaSet);
        ps.add("Observation.CoherentStokes.stationList",
               settings.rawStationList);
        ps.add("Observation.IncoherentStokes.stationList",
               settings.rawStationList);

        // add the feedback for the individual files
        for (size_t i = 0; i < settings.beamFormer.files.size(); ++i) {
          ps.adoptCollection(beamFormedFeedback(i));
        }
      }
      return ps;
    }
  } // namespace Cobalt
} // namespace LOFAR
