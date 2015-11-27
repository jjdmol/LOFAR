//# ModelConfig.cc: Aggregation of all the model configuration options.
//#
//# Copyright (C) 2008
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

#include <lofar_config.h>
#include <BBSKernel/ModelConfig.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_algorithm.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::operator<<;

// -------------------------------------------------------------------------- //
// - ClockConfig implementation                                             - //
// -------------------------------------------------------------------------- //
ClockConfig::ClockConfig()
    : itsSplitClock(false)
{
}

ClockConfig::ClockConfig(bool splitClock)
    :   itsSplitClock(splitClock)
{
}

bool ClockConfig::splitClock() const
{
  return itsSplitClock;
}

// -------------------------------------------------------------------------- //
// - TECConfig implementation                                               - //
// -------------------------------------------------------------------------- //
TECConfig::TECConfig()
    : itsSplitTEC(false)
{
}

TECConfig::TECConfig(bool splitTEC)
    :   itsSplitTEC(splitTEC)
{
}

bool TECConfig::splitTEC() const
{
  return itsSplitTEC;
}

// -------------------------------------------------------------------------- //
// - GainConfig implementation                                               - //
// -------------------------------------------------------------------------- //
GainConfig::GainConfig(bool phasors)
    :   itsPhasors(phasors)
{
}

bool GainConfig::phasors() const
{
    return itsPhasors;
}

// -------------------------------------------------------------------------- //
// - DirectionalGainConfig implementation                                   - //
// -------------------------------------------------------------------------- //
DirectionalGainConfig::DirectionalGainConfig(bool phasors)
    :   itsPhasors(phasors)
{
}

bool DirectionalGainConfig::phasors() const
{
    return itsPhasors;
}

// -------------------------------------------------------------------------- //
// - ElevationCutConfig implementation                                      - //
// -------------------------------------------------------------------------- //
ElevationCutConfig::ElevationCutConfig()
    :   itsThreshold(0.0)
{
}

ElevationCutConfig::ElevationCutConfig(double threshold)
    :   itsThreshold(threshold)
{
}

double ElevationCutConfig::threshold() const
{
    return itsThreshold;
}

// -------------------------------------------------------------------------- //
// - BeamConfig implementation                                              - //
// -------------------------------------------------------------------------- //
bool BeamConfig::isDefined(Mode in)
{
    return in != N_Mode;
}

BeamConfig::Mode BeamConfig::asMode(const string &in)
{
    Mode out = N_Mode;
    for(unsigned int i = 0; i < N_Mode; ++i)
    {
        if(in == asString(static_cast<Mode>(i)))
        {
            out = static_cast<Mode>(i);
            break;
        }
    }

    return out;
}

const string &BeamConfig::asString(Mode in)
{
    //# Caution: Always keep this array of strings in sync with the enum
    //# Mode that is defined in the header.
    static const string name[N_Mode + 1] =
        {"DEFAULT",
        "ELEMENT",
        "ARRAY_FACTOR",
        //# "<UNDEFINED>" should always be last.
        "<UNDEFINED>"};

    return name[in];
}

BeamConfig::BeamConfig()
    :   notdef(0),
        itsMode(DEFAULT),
        itsUseChannelFreq(false)
{
}

BeamConfig::BeamConfig(Mode mode, bool useChannelFreq)
    :   notdef(0),
        itsMode(mode),
        itsUseChannelFreq(useChannelFreq)
{
}

BeamConfig::Mode BeamConfig::mode() const
{
    return itsMode;
}

bool BeamConfig::useChannelFreq() const
{
    return itsUseChannelFreq;
}

// -------------------------------------------------------------------------- //
// - IonosphereConfig implementation                                        - //
// -------------------------------------------------------------------------- //
bool IonosphereConfig::isDefined(ModelType in)
{
    return in != N_ModelType;
}

IonosphereConfig::ModelType IonosphereConfig::asModelType(const string &in)
{
    ModelType out = N_ModelType;
    for(unsigned int i = 0; i < N_ModelType; ++i)
    {
        if(in == asString(static_cast<ModelType>(i)))
        {
            out = static_cast<ModelType>(i);
            break;
        }
    }

    return out;
}

const string &IonosphereConfig::asString(ModelType in)
{
    //# Caution: Always keep this array of strings in sync with the enum
    //# ModelType that is defined in the header.
    static const string name[N_ModelType + 1] =
        {"MIM",
        "EXPION",
        //# "<UNDEFINED>" should always be last.
        "<UNDEFINED>"};

    return name[in];
}

IonosphereConfig::IonosphereConfig()
    :   notdef(0),
        itsModelType(N_ModelType),
        itsDegree(0)
{
}

IonosphereConfig::IonosphereConfig(ModelType type, unsigned int degree = 0)
    :   notdef(0),
        itsModelType(type),
        itsDegree(degree)
{
}

IonosphereConfig::ModelType IonosphereConfig::getModelType() const
{
    return itsModelType;
}

unsigned int IonosphereConfig::degree() const
{
    return itsDegree;
}

// -------------------------------------------------------------------------- //
// - FlaggerConfig implementation                                           - //
// -------------------------------------------------------------------------- //
FlaggerConfig::FlaggerConfig()
    :   itsThreshold(1.0)
{
}

FlaggerConfig::FlaggerConfig(double threshold)
    :   itsThreshold(threshold)
{
}

double FlaggerConfig::threshold() const
{
    return itsThreshold;
}

// -------------------------------------------------------------------------- //
// - ModelConfig implementation                                             - //
// -------------------------------------------------------------------------- //
ModelConfig::ModelConfig()
{
    fill(itsModelOptions, itsModelOptions + N_ModelOptions, false);
}

bool ModelConfig::useBandpass() const
{
    return itsModelOptions[BANDPASS];
}

bool ModelConfig::useClock() const
{
    return itsModelOptions[CLOCK];
}

const ClockConfig &ModelConfig::getClockConfig() const
{
    return itsConfigClock;
}

bool ModelConfig::useGain() const
{
    return itsModelOptions[GAIN];
}

const GainConfig &ModelConfig::getGainConfig() const
{
    return itsConfigGain;
}

bool ModelConfig::useTEC() const
{
    return itsModelOptions[TEC];
}

const TECConfig &ModelConfig::getTECConfig() const
{
    return itsConfigTEC;
}

bool ModelConfig::useCommonRotation() const
{
    return itsModelOptions[COMMON_ROTATION];
}

bool ModelConfig::useCommonScalarPhase() const
{
    return itsModelOptions[COMMON_SCALAR_PHASE];
}

bool ModelConfig::useDirectionalGain() const
{
    return itsModelOptions[DIRECTIONAL_GAIN];
}

const DirectionalGainConfig &ModelConfig::getDirectionalGainConfig() const
{
    return itsConfigDirectionalGain;
}

bool ModelConfig::useElevationCut() const
{
    return itsModelOptions[ELEVATION_CUT];
}

const ElevationCutConfig &ModelConfig::getElevationCutConfig() const
{
    return itsConfigElevationCut;
}

bool ModelConfig::useBeam() const
{
    return itsModelOptions[BEAM];
}

const BeamConfig &ModelConfig::getBeamConfig() const
{
    return itsConfigBeam;
}

bool ModelConfig::useDirectionalTEC() const
{
    return itsModelOptions[DIRECTIONAL_TEC];
}

bool ModelConfig::useFaradayRotation() const
{
    return itsModelOptions[FARADAY_ROTATION];
}

bool ModelConfig::useRotation() const
{
    return itsModelOptions[ROTATION];
}

bool ModelConfig::useScalarPhase() const
{
    return itsModelOptions[SCALAR_PHASE];
}

bool ModelConfig::useIonosphere() const
{
    return itsModelOptions[IONOSPHERE];
}

const IonosphereConfig &ModelConfig::getIonosphereConfig() const
{
    return itsConfigIonosphere;
}

bool ModelConfig::useFlagger() const
{
    return itsModelOptions[FLAGGER];
}

const FlaggerConfig &ModelConfig::getFlaggerConfig() const
{
    return itsConfigFlagger;
}

bool ModelConfig::useCache() const
{
    return itsModelOptions[CACHE];
}

void ModelConfig::setBandpass(bool value)
{
    itsModelOptions[BANDPASS] = value;
}

void ModelConfig::setClockConfig(const ClockConfig &config)
{
    itsModelOptions[CLOCK] = true;
    itsConfigClock = config;
}

void ModelConfig::clearClockConfig()
{
    itsConfigClock = ClockConfig();
    itsModelOptions[CLOCK] = false;
}

void ModelConfig::setTECConfig(const TECConfig &config)
{
    itsModelOptions[TEC] = true;
    itsConfigTEC= config;
}

void ModelConfig::clearTECConfig()
{
    itsConfigTEC= TECConfig();
    itsModelOptions[TEC] = false;
}

void ModelConfig::setGainConfig(const GainConfig &config)
{
    itsModelOptions[GAIN] = true;
    itsConfigGain = config;
}

void ModelConfig::clearGainConfig()
{
    itsModelOptions[GAIN] = false;
    itsConfigGain = GainConfig();
}

void ModelConfig::setCommonRotation(bool value)
{
    itsModelOptions[COMMON_ROTATION] = value;
}

void ModelConfig::setCommonScalarPhase(bool value)
{
    itsModelOptions[COMMON_SCALAR_PHASE] = value;
}

void ModelConfig::setDirectionalGainConfig(const DirectionalGainConfig &config)
{
    itsModelOptions[DIRECTIONAL_GAIN] = true;
    itsConfigDirectionalGain = config;
}

void ModelConfig::clearDirectionalGainConfig()
{
    itsModelOptions[DIRECTIONAL_GAIN] = false;
    itsConfigDirectionalGain = DirectionalGainConfig();
}

void ModelConfig::setElevationCutConfig(const ElevationCutConfig &config)
{
    itsModelOptions[ELEVATION_CUT] = true;
    itsConfigElevationCut = config;
}

void ModelConfig::clearElevationCutConfig()
{
    itsConfigElevationCut = ElevationCutConfig();
    itsModelOptions[ELEVATION_CUT] = false;
}

void ModelConfig::setBeamConfig(const BeamConfig &config)
{
    itsModelOptions[BEAM] = true;
    itsConfigBeam = config;
}

void ModelConfig::clearBeamConfig()
{
    itsConfigBeam = BeamConfig();
    itsModelOptions[BEAM] = false;
}

void ModelConfig::setDirectionalTEC(bool value)
{
    itsModelOptions[DIRECTIONAL_TEC] = value;
}

void ModelConfig::setFaradayRotation(bool value)
{
    itsModelOptions[FARADAY_ROTATION] = value;
}

void ModelConfig::setRotation(bool value)
{
    itsModelOptions[ROTATION] = value;
}

void ModelConfig::setScalarPhase(bool value)
{
    itsModelOptions[SCALAR_PHASE] = value;
}

void ModelConfig::setIonosphereConfig(const IonosphereConfig &config)
{
    itsModelOptions[IONOSPHERE] = true;
    itsConfigIonosphere = config;
}

void ModelConfig::clearIonosphereConfig()
{
    itsConfigIonosphere = IonosphereConfig();
    itsModelOptions[IONOSPHERE] = false;
}

void ModelConfig::setFlaggerConfig(const FlaggerConfig &config)
{
    itsModelOptions[FLAGGER] = true;
    itsConfigFlagger = config;
}

void ModelConfig::clearFlaggerConfig()
{
    itsConfigFlagger = FlaggerConfig();
    itsModelOptions[FLAGGER] = false;
}

void ModelConfig::setCache(bool value)
{
    itsModelOptions[CACHE] = value;
}

void ModelConfig::setSources(const vector<string> &sources)
{
    itsSources = sources;
}

const vector<string> &ModelConfig::sources() const
{
    return itsSources;
}

// -------------------------------------------------------------------------- //
// - Non-member functions                                                   - //
// -------------------------------------------------------------------------- //
ostream &operator<<(ostream &out, const ClockConfig &obj)
{
    out << indent << "Split clock: " << boolalpha << obj.splitClock()
        << noboolalpha;
    return out;
}

ostream &operator<<(ostream &out, const TECConfig &obj)
{
    out << indent << "Split TEC: " << boolalpha << obj.splitTEC()
        << noboolalpha;
    return out;
}

ostream &operator<<(ostream &out, const GainConfig &obj)
{
    out << indent << "Phasors: " << boolalpha << obj.phasors() << noboolalpha;
    return out;
}

ostream &operator<<(ostream &out, const DirectionalGainConfig &obj)
{
    out << indent << "Phasors: " << boolalpha << obj.phasors() << noboolalpha;
    return out;
}

ostream &operator<<(ostream &out, const ElevationCutConfig &obj)
{
    out << indent << "Threshold: " << obj.threshold() << " (deg)";
    return out;
}

ostream &operator<<(ostream &out, const BeamConfig &obj)
{
    out << indent << "Mode: " << BeamConfig::asString(obj.mode())
        << endl << indent << "Use channel frequency: " << boolalpha
        << obj.useChannelFreq() << noboolalpha;
    return out;
}

ostream &operator<<(ostream &out, const IonosphereConfig &obj)
{
    out << indent << "Ionosphere model type: "
        << IonosphereConfig::asString(obj.getModelType());

    if(obj.getModelType() == IonosphereConfig::MIM)
    {
        out << endl << indent << "Degree: " << obj.degree();
    }

    return out;
}

ostream &operator<<(ostream &out, const FlaggerConfig &obj)
{
    out << indent << "Threshold: " << obj.threshold();
    return out;
}

ostream& operator<<(ostream &out, const ModelConfig &obj)
{
    out << "Model configuration:";

    Indent id;
    out << endl << indent << "Bandpass enabled: " << boolalpha
        << obj.useBandpass() << noboolalpha;
    out << endl << indent << "Clock enabled: " << boolalpha
        << obj.useClock() << noboolalpha;
    if (obj.useClock()) {
      Indent id;
      out << endl << obj.getClockConfig();
    }

    out << endl << indent << "Gain enabled: " << boolalpha
        << obj.useGain() << noboolalpha;
    if(obj.useGain())
    {
        Indent id;
        out << endl << obj.getGainConfig();
    }

    out << endl << indent << "TEC enabled: " << boolalpha
        << obj.useTEC() << noboolalpha;
    if (obj.useTEC()) {
      Indent id;
      out << endl << obj.getTECConfig();
    }
    out << endl << indent << "Common rotation enabled: " << boolalpha
        << obj.useCommonRotation() << noboolalpha;
    out << endl << indent << "Common scalar phase enabled: " << boolalpha
        << obj.useCommonScalarPhase() << noboolalpha;
    out << endl << indent << "Direction dependent gain enabled: " << boolalpha
        << obj.useDirectionalGain() << noboolalpha;
    if(obj.useDirectionalGain())
    {
        Indent id;
        out << endl << obj.getDirectionalGainConfig();
    }

    out << endl << indent << "Elevation cut enabled: " << boolalpha
        << obj.useElevationCut() << noboolalpha;
    if(obj.useElevationCut())
    {
        Indent id;
        out << endl << obj.getElevationCutConfig();
    }

    out << endl << indent << "Beam enabled: " << boolalpha << obj.useBeam()
        << noboolalpha;
    if(obj.useBeam())
    {
        Indent id;
        out << endl << obj.getBeamConfig();
    }

    out << endl << indent << "Direction dependent TEC enabled: " << boolalpha
        << obj.useDirectionalTEC() << noboolalpha;
    out << endl << indent << "Faraday rotation enabled: " << boolalpha
        << obj.useFaradayRotation() << noboolalpha;
    out << endl << indent << "Polarization rotation enabled: " << boolalpha
        << obj.useRotation() << noboolalpha;
    out << endl << indent << "Scalar phase enabled: " << boolalpha
        << obj.useScalarPhase() << noboolalpha;

    out << endl << indent << "Ionosphere enabled: " << boolalpha
        << obj.useIonosphere() << noboolalpha;
    if(obj.useIonosphere())
    {
        Indent id;
        out << endl << obj.getIonosphereConfig();
    }

    out << endl << indent << "Flagger enabled: " << boolalpha
        << obj.useFlagger() << noboolalpha;
    if(obj.useFlagger())
    {
        Indent id;
        out << endl << obj.getFlaggerConfig();
    }

    out << endl << indent << "Cache enabled: " << boolalpha << obj.useCache()
        << noboolalpha;

    out << endl << indent << "Sources: " << obj.sources();
    return out;
}

} //# namespace BBS
} //# namespace LOFAR
