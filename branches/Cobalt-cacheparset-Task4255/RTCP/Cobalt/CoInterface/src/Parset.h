//#  Parset.h: class/struct that holds the Parset information
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_INTERFACE_PARSET_H
#define LOFAR_INTERFACE_PARSET_H

// \file
// class/struct that holds the Parset information

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/ParameterSet.h>
#include <Common/LofarBitModeInfo.h>
#include <Common/StreamUtil.h>
#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <CoInterface/BeamCoordinates.h>
#include <CoInterface/Config.h>
#include <CoInterface/OutputTypes.h>
#include <CoInterface/SmartPtr.h>
#include <Stream/Stream.h>
#include <CoInterface/PrintVector.h>

#include <algorithm>
#include <numeric>
#include <sstream>
#include <vector>
#include <string>

namespace LOFAR
{
  namespace RTCP
  {

    class Transpose2;

    enum StokesType { STOKES_I = 0, STOKES_IQUV, STOKES_XXYY, INVALID_STOKES = -1 };


    // The Parset class is a public struct that can be used as base-class
    // for holding Parset related information.
    // It can be instantiated with a parset containing Parset information.
    class Parset : public ParameterSet
    {
    public:
      Parset();
      Parset(const std::string &name);
      Parset(Stream *);

      // Fill the cache based on the ParameterSet keys.
      // Call this if keys are added or changed.
      void updateCache();

      struct Cache {
        /*
         * Generic information
         */

        // The SAS/MAC observation number
        unsigned observationID;

        // Specified observation start time, in seconds since 1970.
        double startTime;

        // Specified observation stop time, in seconds since 1970.
        double stopTime;

        // The station clock, in MHz (200 or 160)
        unsigned clockMHz;

        // The number of bits in each input sample (16, 8, or 4)
        unsigned nrBitsPerSample;

        /*
         * Station information
         */

        struct Station {
          // The name of the station ("CS001LBA", etc)
          std::string name;
        };

        // All stations specified as input
        std::vector<struct Station> stations;

        /*
         * Spectral resolution information
         */

        struct Subband {
          // Index (f.e. 0..243)
          unsigned idx;

          // Index at station (f.e. 100..343)
          unsigned stationIdx;

          // SAP number
          unsigned SAP;

          // Central frequency (Hz)
          double centralFrequency;
        };

        // The list of subbands
        std::vector<struct Subband> subbands;

      } cache;

      std::string                 name() const;
      void                        check() const;

      void                        write(Stream *) const;

      unsigned                    observationID() const;
      double                      startTime() const;
      double                      stopTime() const;

      unsigned    nrCorrelatedBlocks() const;
      unsigned    nrBeamFormedBlocks() const;

      unsigned                    nrStations() const;
      unsigned                    nrTabStations() const;
      unsigned                    nrMergedStations() const;
      std::vector<std::string>    mergedStationNames() const;
      unsigned                    nrBaselines() const;
      unsigned                    nrCrossPolarisations() const;
      unsigned                    clockSpeed() const; // Hz
      double                      subbandBandwidth() const;
      double                      sampleDuration() const;
      unsigned                    nrBitsPerSample() const;
      size_t                      nrBytesPerComplexSample() const;
      std::vector<double>         positions() const;
      std::string                 positionType() const;
      std::vector<double>         getRefPhaseCentre() const;
      std::vector<double>         getPhaseCentreOf(const std::string &name) const;
      unsigned                    dedispersionFFTsize() const;
      unsigned                    CNintegrationSteps() const;
      unsigned                    IONintegrationSteps() const;
      unsigned                    integrationSteps() const;
      unsigned                    coherentStokesTimeIntegrationFactor() const;
      unsigned                    coherentStokesNrSubbandsPerFile() const;
      unsigned                    incoherentStokesTimeIntegrationFactor() const;
      unsigned                    coherentStokesChannelsPerSubband() const;
      unsigned                    incoherentStokesChannelsPerSubband() const;
      unsigned                    incoherentStokesNrSubbandsPerFile() const;
      double                      CNintegrationTime() const;
      double                      IONintegrationTime() const;
      unsigned                    nrSamplesPerChannel() const;
      unsigned                    nrSamplesPerSubband() const;
      unsigned                    nrHistorySamples() const;
      unsigned                    nrSamplesToCNProc() const;
      unsigned                    inputBufferSize() const; // in samples
      unsigned                    maxNetworkDelay() const;
      unsigned                    nrPPFTaps() const;
      unsigned                    nrChannelsPerSubband() const;
      double                      channelWidth() const;
      bool                        delayCompensation() const;
      unsigned                    nrCalcDelays() const;
      bool                        correctClocks() const;
      double                      clockCorrectionTime(const std::string &station) const;
      bool                        correctBandPass() const;
      bool                        hasStorage() const;
      std::string                 stationName(int index) const;
      int                         stationIndex(const std::string &name) const;
      std::vector<std::string>    allStationNames() const;
      unsigned                    getLofarStManVersion() const;
      std::vector<unsigned>       phaseOnePsets() const;
      std::vector<unsigned>       tabList() const;

      std::string                 getTransportType(const std::string &prefix) const;

      bool                        outputCorrelatedData() const;
      bool                        outputBeamFormedData() const;
      bool                        outputTrigger() const;
      bool outputThisType(OutputType) const;

      bool                        onlineFlagging() const;
      bool                        onlinePreCorrelationFlagging() const;
      bool                        onlinePreCorrelationNoChannelsFlagging() const;
      bool                        onlinePostCorrelationFlagging() const;
      bool                        onlinePostCorrelationFlaggingDetectBrokenStations() const;
      unsigned                    onlinePreCorrelationFlaggingIntegration() const;
      std::string                 onlinePreCorrelationFlaggingType(std::string defaultVal) const;
      std::string                 onlinePreCorrelationFlaggingStatisticsType(std::string defaultVal) const;
      std::string                 onlinePostCorrelationFlaggingType(std::string defaultVal) const;
      std::string                 onlinePostCorrelationFlaggingStatisticsType(std::string defaultVal) const;

      unsigned nrStreams(OutputType, bool force = false) const;
      static std::string keyPrefix(OutputType);
      std::string getHostName(OutputType, unsigned streamNr) const;
      std::string getFileName(OutputType, unsigned streamNr) const;
      std::string getDirectoryName(OutputType, unsigned streamNr) const;

      bool                        fakeInputData() const;
      bool                        checkFakeInputData() const;

      std::string                 coherentStokes() const;
      std::string                 incoherentStokes() const;
      std::string                 bandFilter() const;
      std::string                 antennaSet() const;

      size_t          nrCoherentStokes() const
      {
        return coherentStokes().size();
      }
      size_t          nrIncoherentStokes() const
      {
        return incoherentStokes().size();
      }

      unsigned                    nrBeams() const;
      std::string                 beamTarget(unsigned beam) const;
      double                      beamDuration(unsigned beam) const;

      unsigned                    nrTABs(unsigned beam) const;
      std::vector<unsigned>       nrTABs() const;
      unsigned                    totalNrTABs() const;
      unsigned                    maxNrTABs() const;
      bool                        isCoherent(unsigned beam, unsigned pencil) const;
      BeamCoordinates             TABs(unsigned beam) const;
      double                      dispersionMeasure(unsigned beam = 0,unsigned pencil = 0) const;
      std::vector<std::string>    TABStationList(unsigned beam = 0,unsigned pencil = 0, bool raw = false) const;

      std::vector<unsigned>       subbandList() const;
      unsigned                    nrSubbands() const;
      unsigned                    nrSubbandsPerSAP(unsigned sap) const;
      unsigned                    nyquistZone() const;

      std::vector<unsigned>       subbandToSAPmapping() const;
      std::vector<double>         subbandToFrequencyMapping() const;
      std::vector<unsigned>       subbandToRSPboardMapping(const std::string &stationName) const;
      std::vector<unsigned>       subbandToRSPslotMapping(const std::string &stationName) const;

      double channel0Frequency( size_t subband ) const;

      unsigned                    nrSlotsInFrame() const;
      std::string                 partitionName() const;
      bool                        realTime() const;

      std::vector<double>         getBeamDirection(unsigned beam) const;
      std::string                 getBeamDirectionType(unsigned beam) const;

      bool                        haveAnaBeam() const;
      std::vector<double>         getAnaBeamDirection() const;
      std::string                 getAnaBeamDirectionType() const;

      struct StationRSPpair {
        std::string station;
        unsigned rsp;
      };

      std::vector<StationRSPpair> getStationNamesAndRSPboardNumbers(unsigned psetNumber) const;

      std::string                 getInputStreamName(const string &stationName, unsigned rspBoardNumber) const;

      std::vector<double>         itsStPositions;

      std::string                 PVSS_TempObsName() const;

      std::string                 AntennaSetsConf() const;
      std::string                 AntennaFieldsDir() const;
      std::string                 HBADeltasDir() const;

      const Transpose2            &transposeLogic() const;

    private:
      const std::string itsName;

      mutable std::string itsWriteCache;

      mutable SmartPtr<const Transpose2>     itsTransposeLogic;

      void                        checkVectorLength(const std::string &key, unsigned expectedSize) const;
      void                        checkInputConsistency() const;

      std::vector<double>         getTAB(unsigned beam, unsigned pencil) const;

      void                        addPosition(string stName);
      double                      getTime(const std::string &name, const std::string &defaultValue) const;
      static int                  findIndex(unsigned pset, const vector<unsigned> &psets);

      std::vector<double>         centroidPos(const string &stations) const;
    };

    //
    // All of the logic for the second transpose.
    //

    struct StreamInfo {
      unsigned stream;

      unsigned sap;
      unsigned beam;

      bool coherent;
      unsigned nrChannels; // channels per subband
      unsigned timeIntFactor; // time integration factor
      unsigned nrStokes;   // total # stokes for this beam
      StokesType stokesType;
      unsigned nrSamples;  // # samples/channel, after temporal integration

      unsigned stokes;
      unsigned part;

      std::vector<unsigned> subbands;

      void log() const;
    };

    class Transpose2
    {
    public:
      Transpose2( const Parset &parset );

      unsigned nrStreams() const;

      // compose and decompose a stream number
      unsigned stream( unsigned sap, unsigned beam, unsigned stokes, unsigned part, unsigned startAt = 0) const;
      void decompose( unsigned stream, unsigned &sap, unsigned &beam, unsigned &stokes, unsigned &part ) const;

      std::vector<unsigned> subbands( unsigned stream ) const;
      unsigned nrSubbands( unsigned stream ) const;
      unsigned maxNrSubbands() const;
      unsigned maxNrChannels() const;
      unsigned maxNrSamples() const;

      size_t subbandSize( unsigned stream ) const;

      const unsigned nrChannels;
      const unsigned nrCoherentChannels;
      const unsigned nrIncoherentChannels;
      const unsigned nrSamples;
      const unsigned coherentTimeIntFactor;
      const unsigned incoherentTimeIntFactor;

      const unsigned nrBeams;
      const unsigned coherentNrSubbandsPerFile;
      const unsigned incoherentNrSubbandsPerFile;

      const std::vector<struct StreamInfo> streamInfo;

    private:
      std::vector<struct StreamInfo> generateStreamInfo( const Parset &parset ) const;
    };

  } // namespace RTCP
} // namespace LOFAR

#endif
