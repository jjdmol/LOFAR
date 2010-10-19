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
#include <Interface/CN_Mode.h>
#include <Interface/PencilCoordinates.h>
#include <Interface/Parset.h>

#include <cstring>
#include <vector>
#include <string>

namespace LOFAR {
namespace RTCP {

class CN_Configuration
{
  public:
    CN_Configuration();

#if !defined HAVE_BGP_CN
    CN_Configuration(const Parset &parset);
#endif

    unsigned		  &nrStations();
    unsigned		  &nrBitsPerSample();
    unsigned		  &nrSubbands();
    unsigned		  &nrChannelsPerSubband();
    unsigned		  &nrSamplesPerIntegration();
    unsigned		  &nrSamplesPerStokesIntegration();
    unsigned		  &nrSamplesToCNProc();
    unsigned		  &nrSubbandsPerPset();
    bool		  &delayCompensation();
    bool		  &correctBandPass();
    double		  &sampleRate();
    std::vector<unsigned> &inputPsets(), &outputPsets(), &tabList();
    std::vector<unsigned> &usedCoresInPset();
    std::vector<double>	  &refFreqs();
    PencilCoordinates     &pencilBeams();
    std::vector<double>   &refPhaseCentre();
    Matrix<double>        &phaseCentres();
    CN_Mode               &mode();
    bool                  &outputIncoherentStokesI();
    bool                  &stokesIntegrateChannels();
    unsigned              nrPencilBeams() const;
    
    void		  read(Stream *);
    void		  write(Stream *);

    static const unsigned MAX_PSETS	     = 64;
    static const unsigned MAX_SUBBANDS	     = 1024;
    static const unsigned MAX_STATIONS	     = 100;
    static const unsigned MAX_PENCILBEAMS    = 256;
    static const unsigned MAX_CORES_PER_PSET = 64;

  private:
    std::vector<unsigned> itsInputPsets, itsOutputPsets, itsTabList;
    std::vector<unsigned> itsUsedCoresInPset;
    std::vector<double>	  itsRefFreqs;
    std::vector<double>	  itsRefPhaseCentre;
    PencilCoordinates     itsPencilBeams;
    Matrix<double>        itsPhaseCentres;
    CN_Mode               itsMode;

    struct MarshalledData
    {
      unsigned		  itsNrStations;
      unsigned		  itsNrBitsPerSample;
      unsigned            itsNrSubbands;
      unsigned		  itsNrChannelsPerSubband;
      unsigned		  itsNrSamplesPerIntegration;
      unsigned		  itsNrSamplesPerStokesIntegration;
      unsigned		  itsNrSamplesToCNProc;
      unsigned		  itsNrUsedCoresPerPset;
      unsigned		  itsNrSubbandsPerPset;
      bool		  itsDelayCompensation;
      bool		  itsCorrectBandPass;
      double		  itsSampleRate;
      unsigned		  itsInputPsetsSize, itsOutputPsetsSize, itsTabListSize;
      unsigned		  itsRefFreqsSize;
      unsigned		  itsInputPsets[MAX_PSETS], itsOutputPsets[MAX_PSETS], itsTabList[MAX_PSETS];
      unsigned		  itsUsedCoresInPset[MAX_CORES_PER_PSET];
      double		  itsRefFreqs[MAX_SUBBANDS];
      double              itsRefPhaseCentre[3];
      double              itsPhaseCentres[MAX_STATIONS * 3];
      unsigned            itsNrPencilBeams;
      double              itsPencilBeams[MAX_PENCILBEAMS * 2];
      bool                itsOutputIncoherentStokesI;
      bool                itsStokesIntegrateChannels;
    } itsMarshalledData;
};


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

inline unsigned &CN_Configuration::nrSamplesToCNProc()
{
  return itsMarshalledData.itsNrSamplesToCNProc;
}

inline unsigned &CN_Configuration::nrSubbandsPerPset()
{
  return itsMarshalledData.itsNrSubbandsPerPset;
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

inline std::vector<unsigned> &CN_Configuration::inputPsets()
{
  return itsInputPsets;
}

inline std::vector<unsigned> &CN_Configuration::outputPsets()
{
  return itsOutputPsets;
}

inline std::vector<unsigned> &CN_Configuration::tabList()
{
  return itsTabList;
}

inline std::vector<unsigned> &CN_Configuration::usedCoresInPset()
{
  return itsUsedCoresInPset;
}

inline std::vector<double> & CN_Configuration::refFreqs()
{
  return itsRefFreqs;
}

inline PencilCoordinates &CN_Configuration::pencilBeams()
{
  return itsPencilBeams;
}

inline unsigned CN_Configuration::nrPencilBeams() const
{
  return itsPencilBeams.size();
}

inline std::vector<double> &CN_Configuration::refPhaseCentre()
{
  return itsRefPhaseCentre;
}

inline Matrix<double> &CN_Configuration::phaseCentres()
{
  return itsPhaseCentres;
}

inline CN_Mode &CN_Configuration::mode()
{
  return itsMode;
}

inline bool &CN_Configuration::outputIncoherentStokesI()
{
  return itsMarshalledData.itsOutputIncoherentStokesI;
}

inline bool &CN_Configuration::stokesIntegrateChannels()
{
  return itsMarshalledData.itsStokesIntegrateChannels;
}


} // namespace RTCP
} // namespace LOFAR

#endif
