//# CN_Configuration.h:
//#
//#  Copyright (C) 2007
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

#ifndef LOFAR_INTERFACE_CN_CONFIGURATION_H
#define LOFAR_INTERFACE_CN_CONFIGURATION_H

#include <Stream/Stream.h>
#include <Interface/MultiDimArray.h>
#include <Interface/Parset.h>

#include <vector>

namespace LOFAR {
namespace RTCP {

class CN_Configuration
{
  public:
    CN_Configuration();

#if !defined HAVE_BGP_CN
    CN_Configuration(const Parset &parset);
#endif

    double                &startTime();
    double                &stopTime();
    double                &integrationTime();
    double		  &dispersionMeasure();
    unsigned		  &dedispersionFFTsize();

    unsigned		  &nrStations();
    unsigned		  nrMergedStations();

    unsigned		  &nrBitsPerSample();
    unsigned		  &nrSubbands();
    unsigned		  &nrChannelsPerSubband();
    unsigned		  &nrSamplesPerIntegration();
    unsigned		  &nrSamplesPerStokesIntegration();
    unsigned              &stokesNrChannelsPerSubband();
    unsigned		  &nrSamplesToCNProc();
    unsigned		  &nrSubbandsPerPset();
    unsigned		  &nrSubbandsPerPart();
    unsigned		  &nrPartsPerStokes();
    unsigned		  &nrBeamsPerPset();
    bool		  &delayCompensation();
    bool		  &correctBandPass();
    double		  &sampleRate();
    std::vector<unsigned> &phaseOnePsets(), &phaseTwoPsets(), &phaseThreePsets(), &tabList();
    bool                  &phaseThreeDisjunct();
    std::vector<unsigned> &usedCoresInPset();
    std::vector<unsigned> &phaseOneTwoCores();
    std::vector<unsigned> &phaseThreeCores();
    std::vector<double>	  &refFreqs();
    std::vector<unsigned> &subbandList();
    std::vector<double>   &refPhaseCentre();
    Matrix<double>        &phaseCentres();

    bool                  &outputFilteredData();
    bool                  &outputCorrelatedData();
    bool                  &outputBeamFormedData();
    bool                  &outputCoherentStokes();
    bool                  &outputIncoherentStokes();
    bool                  &outputTrigger();
    unsigned              &nrStokes();
    bool                  &flysEye();

    bool                  &fakeInputData();

    unsigned              &nrPencilBeams();

    unsigned              &observationID();
    
    void		  read(Stream *);
    void		  write(Stream *);

    static const unsigned MAX_PSETS	     = 64;
    static const unsigned MAX_SUBBANDS	     = 1024;
    static const unsigned MAX_STATIONS	     = 100;
    static const unsigned MAX_CORES_PER_PSET = 64;

  private:
    std::vector<unsigned> itsPhaseOnePsets, itsPhaseTwoPsets, itsPhaseThreePsets, itsTabList;
    std::vector<unsigned> itsUsedCoresInPset;
    std::vector<unsigned> itsPhaseOneTwoCores;
    std::vector<unsigned> itsPhaseThreeCores;
    std::vector<double>	  itsRefFreqs;
    std::vector<unsigned> itsSubbandList;
    std::vector<double>	  itsRefPhaseCentre;
    Matrix<double>        itsPhaseCentres;

    struct MarshalledData
    {
      double              itsStartTime;
      double              itsStopTime;
      double              itsIntegrationTime;
      double		  itsDispersionMeasure;
      unsigned		  itsDedispersionFFTsize;
      unsigned		  itsNrStations;
      unsigned		  itsNrBitsPerSample;
      unsigned            itsNrSubbands;
      unsigned		  itsNrChannelsPerSubband;
      unsigned		  itsNrSamplesPerIntegration;
      unsigned		  itsNrSamplesPerStokesIntegration;
      unsigned            itsStokesNrChannelsPerSubband;
      unsigned		  itsNrSamplesToCNProc;
      unsigned		  itsNrUsedCoresPerPset;
      unsigned		  itsNrPhaseOneTwoCores;
      unsigned		  itsNrPhaseThreeCores;
      unsigned		  itsNrSubbandsPerPset;
      unsigned		  itsNrSubbandsPerPart;
      unsigned		  itsNrPartsPerStokes;
      unsigned		  itsNrBeamsPerPset;
      bool		  itsDelayCompensation;
      bool		  itsCorrectBandPass;
      double		  itsSampleRate;
      unsigned		  itsPhaseOnePsetsSize, itsPhaseTwoPsetsSize, itsPhaseThreePsetsSize, itsTabListSize;
      bool                itsPhaseThreeDisjunct;
      unsigned		  itsRefFreqsSize;
      unsigned		  itsSubbandListSize;
      unsigned		  itsPhaseOnePsets[MAX_PSETS], itsPhaseTwoPsets[MAX_PSETS], itsPhaseThreePsets[MAX_PSETS], itsTabList[MAX_PSETS];
      unsigned		  itsUsedCoresInPset[MAX_CORES_PER_PSET];
      unsigned		  itsPhaseOneTwoCores[MAX_CORES_PER_PSET];
      unsigned		  itsPhaseThreeCores[MAX_CORES_PER_PSET];
      double		  itsRefFreqs[MAX_SUBBANDS];
      unsigned		  itsSubbandList[MAX_SUBBANDS];
      double              itsRefPhaseCentre[3];
      double              itsPhaseCentres[MAX_STATIONS * 3];
      unsigned            itsNrPencilBeams;
      bool                itsOutputFilteredData;
      bool                itsOutputCorrelatedData;
      bool                itsOutputBeamFormedData;
      bool                itsOutputCoherentStokes;
      bool                itsOutputIncoherentStokes;
      bool                itsOutputTrigger;
      unsigned            itsNrStokes;
      bool                itsFlysEye;
      unsigned            itsObservationID;
      bool                itsFakeInputData;
    } itsMarshalledData;
};


inline double &CN_Configuration::startTime()
{
  return itsMarshalledData.itsStartTime;
}


inline double &CN_Configuration::stopTime()
{
  return itsMarshalledData.itsStopTime;
}


inline double &CN_Configuration::integrationTime()
{
  return itsMarshalledData.itsIntegrationTime;
}


inline double &CN_Configuration::dispersionMeasure()
{
  return itsMarshalledData.itsDispersionMeasure;
}


inline unsigned &CN_Configuration::dedispersionFFTsize()
{
  return itsMarshalledData.itsDedispersionFFTsize;
}


inline unsigned &CN_Configuration::nrStations()
{
  return itsMarshalledData.itsNrStations;
}

inline unsigned &CN_Configuration::nrBitsPerSample()
{
  return itsMarshalledData.itsNrBitsPerSample;
}

inline unsigned &CN_Configuration::nrSubbands()
{
  return itsMarshalledData.itsNrSubbands;
}

inline unsigned &CN_Configuration::nrChannelsPerSubband()
{
  return itsMarshalledData.itsNrChannelsPerSubband;
}

inline unsigned &CN_Configuration::nrSamplesPerIntegration()
{
  return itsMarshalledData.itsNrSamplesPerIntegration;
}

inline unsigned &CN_Configuration::nrSamplesPerStokesIntegration()
{
  return itsMarshalledData.itsNrSamplesPerStokesIntegration;
}

inline unsigned &CN_Configuration::stokesNrChannelsPerSubband()
{
  return itsMarshalledData.itsStokesNrChannelsPerSubband;
}

inline unsigned &CN_Configuration::nrSamplesToCNProc()
{
  return itsMarshalledData.itsNrSamplesToCNProc;
}

inline unsigned &CN_Configuration::nrSubbandsPerPset()
{
  return itsMarshalledData.itsNrSubbandsPerPset;
}

inline unsigned &CN_Configuration::nrSubbandsPerPart()
{
  return itsMarshalledData.itsNrSubbandsPerPart;
}

inline unsigned &CN_Configuration::nrPartsPerStokes()
{
  return itsMarshalledData.itsNrPartsPerStokes;
}

inline unsigned &CN_Configuration::nrBeamsPerPset()
{
  return itsMarshalledData.itsNrBeamsPerPset;
}

inline bool &CN_Configuration::delayCompensation()
{
  return itsMarshalledData.itsDelayCompensation;
}

inline bool &CN_Configuration::correctBandPass()
{
  return itsMarshalledData.itsCorrectBandPass;
}

inline double &CN_Configuration::sampleRate()
{
  return itsMarshalledData.itsSampleRate;
}

inline std::vector<unsigned> &CN_Configuration::phaseOnePsets()
{
  return itsPhaseOnePsets;
}

inline std::vector<unsigned> &CN_Configuration::phaseTwoPsets()
{
  return itsPhaseTwoPsets;
}

inline std::vector<unsigned> &CN_Configuration::phaseThreePsets()
{
  return itsPhaseThreePsets;
}

inline bool &CN_Configuration::phaseThreeDisjunct()
{
  return itsMarshalledData.itsPhaseThreeDisjunct;
}

inline std::vector<unsigned> &CN_Configuration::tabList()
{
  return itsTabList;
}

inline std::vector<unsigned> &CN_Configuration::usedCoresInPset()
{
  return itsUsedCoresInPset;
}

inline std::vector<unsigned> &CN_Configuration::phaseOneTwoCores()
{
  return itsPhaseOneTwoCores;
}

inline std::vector<unsigned> &CN_Configuration::phaseThreeCores()
{
  return itsPhaseThreeCores;
}

inline std::vector<double> & CN_Configuration::refFreqs()
{
  return itsRefFreqs;
}

inline std::vector<unsigned> & CN_Configuration::subbandList()
{
  return itsSubbandList;
}

inline unsigned &CN_Configuration::nrPencilBeams()
{
  return itsMarshalledData.itsNrPencilBeams;
}

inline std::vector<double> &CN_Configuration::refPhaseCentre()
{
  return itsRefPhaseCentre;
}

inline Matrix<double> &CN_Configuration::phaseCentres()
{
  return itsPhaseCentres;
}

inline bool &CN_Configuration::outputFilteredData()
{
  return itsMarshalledData.itsOutputFilteredData;
}

inline bool &CN_Configuration::outputCorrelatedData()
{
  return itsMarshalledData.itsOutputCorrelatedData;
}

inline bool &CN_Configuration::outputBeamFormedData()
{
  return itsMarshalledData.itsOutputBeamFormedData;
}

inline bool &CN_Configuration::outputCoherentStokes()
{
  return itsMarshalledData.itsOutputCoherentStokes;
}

inline bool &CN_Configuration::outputIncoherentStokes()
{
  return itsMarshalledData.itsOutputIncoherentStokes;
}

inline bool &CN_Configuration::outputTrigger()
{
  return itsMarshalledData.itsOutputTrigger;
}

inline unsigned &CN_Configuration::nrStokes()
{
  return itsMarshalledData.itsNrStokes;
}

inline bool &CN_Configuration::flysEye()
{
  return itsMarshalledData.itsFlysEye;
}

inline unsigned &CN_Configuration::observationID()
{
  return itsMarshalledData.itsObservationID;
}

inline bool &CN_Configuration::fakeInputData()
{
  return itsMarshalledData.itsFakeInputData;
}


} // namespace RTCP
} // namespace LOFAR

#endif
