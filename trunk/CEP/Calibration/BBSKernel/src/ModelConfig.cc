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
// - BeamConfig implementation                                              - //
// -------------------------------------------------------------------------- //

bool BeamConfig::isDefined(ElementType in)
{
    return in != N_ElementType;
}

BeamConfig::ElementType BeamConfig::asElementType(const string &in)
{
    ElementType out = N_ElementType;
    for(unsigned int i = 0; i < N_ElementType; ++i)
    {
        if(in == asString(static_cast<ElementType>(i)))
        {
            out = static_cast<ElementType>(i);
            break;
        }
    }

    return out;
}

const string &BeamConfig::asString(ElementType in)
{
    //# Caution: Always keep this array of strings in sync with the enum
    //# ElementType that is defined in the header.
    static const string name[N_ElementType + 1] =
        {"HAMAKER_LBA",
        "HAMAKER_HBA",
        "YATAWATTA_LBA",
        "YATAWATTA_HBA",
        //# "<UNDEFINED>" should always be last.
        "<UNDEFINED>"};

    return name[in];
}

BeamConfig::BeamConfig()
    :   itsElementType(N_ElementType)
{
}

BeamConfig::BeamConfig(const string &configName, const casa::Path &configPath,
    ElementType elementType, const casa::Path &elementPath)
    :   itsConfigName(configName),
        itsConfigPath(configPath),
        itsElementType(elementType),
        itsElementPath(elementPath)
{
}

const string &BeamConfig::getConfigName() const
{
    return itsConfigName;
}

const casa::Path &BeamConfig::getConfigPath() const
{
    return itsConfigPath;
}

BeamConfig::ElementType BeamConfig::getElementType() const
{
    return itsElementType;
}

const casa::Path &BeamConfig::getElementPath() const
{
    return itsElementPath;
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

double FlaggerConfig::getThreshold() const
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

bool ModelConfig::useDirectionalGain() const
{
    return itsModelOptions[DIRECTIONAL_GAIN];
}

bool ModelConfig::useFaradayRotation() const
{
    return itsModelOptions[FARADAY_ROTATION];
}

bool ModelConfig::useBeam() const
{
    return itsModelOptions[BEAM];
}

const BeamConfig &ModelConfig::getBeamConfig() const
{
    return itsConfigBeam;
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

void ModelConfig::setDirectionalGain(bool value)
{
    itsModelOptions[DIRECTIONAL_GAIN] = value;
}

void ModelConfig::setFaradayRotation(bool value)
{
    itsModelOptions[FARADAY_ROTATION] = value;
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

const vector<string> &ModelConfig::getSources() const
{
    return itsSources;
}

// -------------------------------------------------------------------------- //
// - Non-member functions                                                   - //
// -------------------------------------------------------------------------- //

ostream &operator<<(ostream &out, const FlaggerConfig &obj)
{
    out << indent << "Threshold: " << obj.getThreshold();
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
    out << indent << "Antenna configuration name: " << obj.getConfigName()
        << endl << indent << "Antenna configuration path: "
        << obj.getConfigPath().originalName()
        << endl << indent << "Element model type: "
        << BeamConfig::asString(obj.getElementType())
        << endl << indent << "Element model path: "
        << obj.getElementPath().originalName();
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
    out << endl << indent << "Direction dependent gain enabled: " << boolalpha
        << obj.useDirectionalGain() << noboolalpha;
    out << endl << indent << "Faraday rotation enabled: " << boolalpha
        << obj.useFaradayRotation() << noboolalpha;

    out << endl << indent << "Beam enabled: " << boolalpha << obj.useBeam()
        << noboolalpha;
    if(obj.useBeam())
    {
        Indent id;
        out << endl << obj.getBeamConfig();
    }

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

    out << endl << indent << "Sources: " << obj.getSources();
    return out;
}

} //# namespace BBS
} //# namespace LOFAR
