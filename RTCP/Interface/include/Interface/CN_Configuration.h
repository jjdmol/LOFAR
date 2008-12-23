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

#include <vector>
#include <string>

namespace LOFAR {
namespace RTCP {

class CN_Configuration
{
  public:
    unsigned		  &nrStations();
    unsigned		  &nrBitsPerSample();
    unsigned		  &nrChannelsPerSubband();
    unsigned		  &nrSamplesPerIntegration();
    unsigned		  &nrSamplesToCNProc();
    unsigned		  &nrUsedCoresPerPset();
    unsigned		  &nrSubbandsPerPset();
    bool		  &delayCompensation();
    bool		  &correctBandPass();
    double		  &sampleRate();
    std::vector<unsigned> &inputPsets(), &outputPsets(), &tabList();
    std::vector<double>	  &refFreqs();
    unsigned              &nrPencilRings();
    double                &pencilRingSize();
    std::vector<double>   &refPhaseCentre();
    Matrix<double>        &phaseCentres();
    std::string           &outputDataType();
    
    void		  read(Stream *);
    void		  write(Stream *);

    static const unsigned MAX_PSETS    = 64;
    static const unsigned MAX_SUBBANDS = 248;
    static const unsigned MAX_STATIONS = 100;
    static const unsigned MAX_OUTPUTDATATYPE_LENGTH = 128;

  private:
    std::vector<unsigned> itsInputPsets, itsOutputPsets, itsTabList;
    std::vector<double>	  itsRefFreqs;
    std::vector<double>	  itsRefPhaseCentre;
    Matrix<double>        itsPhaseCentres;
    std::string           itsOutputDataType;

    struct MarshalledData
    {
      unsigned		  itsNrStations;
      unsigned		  itsNrBitsPerSample;
      unsigned		  itsNrChannelsPerSubband;
      unsigned		  itsNrSamplesPerIntegration;
      unsigned		  itsNrSamplesToCNProc;
      unsigned		  itsNrUsedCoresPerPset;
      unsigned		  itsNrSubbandsPerPset;
      bool		  itsDelayCompensation;
      bool		  itsCorrectBandPass;
      double		  itsSampleRate;
      unsigned		  itsInputPsetsSize, itsOutputPsetsSize, itsTabListSize;
      unsigned		  itsRefFreqsSize;
      unsigned		  itsInputPsets[MAX_PSETS], itsOutputPsets[MAX_PSETS], itsTabList[MAX_PSETS];
      double		  itsRefFreqs[MAX_SUBBANDS];
      unsigned            itsNrPencilRings;
      double              itsPencilRingSize;
      double              itsRefPhaseCentre[3];
      double              itsPhaseCentres[MAX_STATIONS * 3];
      char                itsOutputDataType[MAX_OUTPUTDATATYPE_LENGTH];
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

inline unsigned &CN_Configuration::nrChannelsPerSubband()
{
  return itsMarshalledData.itsNrChannelsPerSubband;
}

inline unsigned &CN_Configuration::nrSamplesPerIntegration()
{
  return itsMarshalledData.itsNrSamplesPerIntegration;
}

inline unsigned &CN_Configuration::nrSamplesToCNProc()
{
  return itsMarshalledData.itsNrSamplesToCNProc;
}

inline unsigned &CN_Configuration::nrUsedCoresPerPset()
{
  return itsMarshalledData.itsNrUsedCoresPerPset;
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

inline std::vector<double> & CN_Configuration::refFreqs()
{
  return itsRefFreqs;
}

inline unsigned &CN_Configuration::nrPencilRings()
{
  return itsMarshalledData.itsNrPencilRings;
}

inline double &CN_Configuration::pencilRingSize()
{
  return itsMarshalledData.itsPencilRingSize;
}

inline std::vector<double> &CN_Configuration::refPhaseCentre()
{
  return itsRefPhaseCentre;
}

inline Matrix<double> &CN_Configuration::phaseCentres()
{
  return itsPhaseCentres;
}

inline std::string &CN_Configuration::outputDataType()
{
  return itsOutputDataType;
}

} // namespace RTCP
} // namespace LOFAR

#endif
