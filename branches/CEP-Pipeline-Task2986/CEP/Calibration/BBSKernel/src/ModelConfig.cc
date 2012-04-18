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
    :   itsMode(DEFAULT),
        itsUseChannelFreq(false),
        itsConjugateAF(false)
{
}

BeamConfig::BeamConfig(Mode mode, bool useChannelFreq, bool conjugateAF)
    :   itsMode(mode),
        itsUseChannelFreq(useChannelFreq),
        itsConjugateAF(conjugateAF)
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

bool BeamConfig::conjugateAF() const
{
    return itsConjugateAF;
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
    :   itsModelType(N_ModelType),
        itsDegree(0)
{
}

IonosphereConfig::IonosphereConfig(ModelType type, unsigned int degree = 0)
    :   itsModelType(type),
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

bool ModelConfig::usePhasors() const
{
    return itsModelOptions[PHASORS];
}

bool ModelConfig::useBandpass() const
{
    return itsModelOptions[BANDPASS];
}

bool ModelConfig::useClock() const
{
    return itsModelOptions[CLOCK];
}

bool ModelConfig::useGain() const
{
    return itsModelOptions[GAIN];
}

bool ModelConfig::useTEC() const
{
    return itsModelOptions[TEC];
}

bool ModelConfig::useDirectionalGain() const
{
    return itsModelOptions[DIRECTIONAL_GAIN];
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

void ModelConfig::setPhasors(bool value)
{
    itsModelOptions[PHASORS] = value;
}

void ModelConfig::setBandpass(bool value)
{
    itsModelOptions[BANDPASS] = value;
}

void ModelConfig::setClock(bool value)
{
    itsModelOptions[CLOCK] = value;
}

void ModelConfig::setGain(bool value)
{
    itsModelOptions[GAIN] = value;
}

void ModelConfig::setTEC(bool value)
{
    itsModelOptions[TEC] = value;
}

void ModelConfig::setDirectionalGain(bool value)
{
    itsModelOptions[DIRECTIONAL_GAIN] = value;
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

ostream &operator<<(ostream &out, const FlaggerConfig &obj)
{
    out << indent << "Threshold: " << obj.threshold();
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

ostream &operator<<(ostream &out, const BeamConfig &obj)
{
    out << indent << "Mode: " << BeamConfig::asString(obj.mode())
        << endl << indent << "Use channel frequency: " << boolalpha
        << obj.useChannelFreq() << noboolalpha
        << endl << indent << "Conjugate array factor: " << boolalpha
        << obj.conjugateAF() << noboolalpha;
    return out;
}

ostream &operator<<(ostream &out, const ElevationCutConfig &obj)
{
    out << indent << "Threshold: " << obj.threshold() << " (deg)";
    return out;
}

ostream& operator<<(ostream &out, const ModelConfig &obj)
{
    out << "Model configuration:";

    Indent id;
    out << endl << indent << "Phasors enabled: " << boolalpha
        << obj.usePhasors() << noboolalpha;
    out << endl << indent << "Bandpass enabled: " << boolalpha
        << obj.useBandpass() << noboolalpha;
    out << endl << indent << "Clock enabled: " << boolalpha
        << obj.useClock() << noboolalpha;
    out << endl << indent << "Gain enabled: " << boolalpha
        << obj.useGain() << noboolalpha;
    out << endl << indent << "TEC enabled: " << boolalpha
        << obj.useTEC() << noboolalpha;
    out << endl << indent << "Direction dependent gain enabled: " << boolalpha
        << obj.useDirectionalGain() << noboolalpha;

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
