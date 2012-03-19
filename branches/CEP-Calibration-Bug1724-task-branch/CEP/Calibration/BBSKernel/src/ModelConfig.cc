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
#include <Common/StringUtil.h>
#include <Common/lofar_algorithm.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::operator<<;

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
// - CasaStringFilter implementation                                        - //
// -------------------------------------------------------------------------- //

CasaStringFilter::CasaStringFilter(const string &filter)
{
//    LOG_DEBUG_STR("Create filter: " << filter);
    size_t last = 0;
    while(last < filter.size())
    {
//        LOG_DEBUG_STR("last: " << last << " find: " << filter.find(';', last));
        size_t pos = filter.find(';', last);
        if(pos == string::npos)
        {
            pos = filter.size();
        }

        ASSERTSTR(pos >= last, "pos: " << pos << " last: " << last);
        string term = filter.substr(last, pos - last);
        ltrim(term);
        rtrim(term);

        if(!term.empty())
        {
            if(term[0] != '!')
            {
                itsRegEx.push_back(casa::Regex(casa::Regex::fromPattern(term)));
                itsInverted.push_back(false);
            }
            else if(term.size() > 1)
            {
                term = term.substr(1);
                ltrim(term);
                rtrim(term);

                if(!term.empty())
                {
                    itsRegEx.push_back(casa::Regex(casa::Regex::fromPattern(term)));
                    itsInverted.push_back(true);
                }
            }
        }

        last = pos + 1;
    }
}

bool CasaStringFilter::matches(const string &in) const
{
    casa::String tmp(in);

    size_t i = 0, end = itsInverted.size();
    while(i < end)
    {
        if(itsInverted[i])
        {
            ++i;
            continue;
        }

        if(!tmp.matches(itsRegEx[i]))
        {
            ++i;
            continue;
        }

        ++i;
        bool accept = true;
        while(i < end && itsInverted[i])
        {
            if(tmp.matches(itsRegEx[i]))
            {
                accept = false;
                break;
            }

            ++i;
        }

        if(accept)
        {
            return true;
        }
    }

    return false;
}

bool CasaStringFilter::operator()(const string &in) const
{
    return matches(in);
}

// -------------------------------------------------------------------------- //
// - DDEPartition implementation                                            - //
// -------------------------------------------------------------------------- //

unsigned int DDEPartition::size() const
{
    return itsRegEx.size();
}

bool DDEPartition::matches(unsigned int i, const string &name) const
{
//    casa::String tmp(name);
//    return tmp.matches(itsRegEx[i]);
    return itsRegEx[i].matches(name);
}

bool DDEPartition::group(unsigned int i) const
{
//    return itsGroupFlag[i];
    return !itsGroupName[i].empty();
}

string DDEPartition::name(unsigned int i) const
{
    return itsGroupName[i];
}

void DDEPartition::append(const string &pattern)
{
//    itsRegEx.push_back(casa::Regex(casa::Regex::fromPattern(pattern)));
//    itsGroupFlag.push_back(group);
    itsGroupName.push_back(string());
    itsRegEx.push_back(CasaStringFilter(pattern));
}

void DDEPartition::append(const string &name, const string &pattern)
{
//    itsRegEx.push_back(casa::Regex(casa::Regex::fromPattern(pattern)));
//    itsGroupFlag.push_back(group);
    itsGroupName.push_back(name);
    itsRegEx.push_back(CasaStringFilter(pattern));
}

// -------------------------------------------------------------------------- //
// - DDEConfig implementation                                               - //
// -------------------------------------------------------------------------- //

DDEConfig::~DDEConfig()
{
}

const DDEPartition &DDEConfig::partition() const
{
    return itsPartition;
}

void DDEConfig::setPartition(const DDEPartition &partition)
{
    itsPartition = partition;
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

BeamConfig::BeamConfig(Mode mode, bool conjugateAF,
    const casa::Path &elementPath)
    :   itsMode(mode),
        itsConjugateAF(conjugateAF),
        itsElementPath(elementPath)
{
}

BeamConfig::Mode BeamConfig::mode() const
{
    return itsMode;
}

bool BeamConfig::conjugateAF() const
{
    return itsConjugateAF;
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

IonosphereConfig::IonosphereConfig(ModelType type, unsigned int degree)
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

bool ModelConfig::useDirectionalGain() const
{
    return itsModelOptions[DIRECTIONAL_GAIN];
}

const DirectionalGainConfig &ModelConfig::getDirectionalGainConfig() const
{
    return itsConfigDirectionalGain;
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

const DDEConfig &ModelConfig::getDirectionalTECConfig() const
{
    return itsConfigDirectionalTEC;
}

bool ModelConfig::useFaradayRotation() const
{
    return itsModelOptions[FARADAY_ROTATION];
}

const DDEConfig &ModelConfig::getFaradayRotationConfig() const
{
    return itsConfigFaradayRotation;
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

void ModelConfig::setClock(bool value)
{
    itsModelOptions[CLOCK] = value;
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

void ModelConfig::setTEC(bool value)
{
    itsModelOptions[TEC] = value;
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

void ModelConfig::setDirectionalTECConfig(const DDEConfig &config)
{
    itsModelOptions[DIRECTIONAL_TEC] = true;
    itsConfigDirectionalTEC = config;
}

void ModelConfig::clearDirectionalTECConfig()
{
    itsConfigDirectionalTEC = DDEConfig();
    itsModelOptions[DIRECTIONAL_TEC] = false;
}

void ModelConfig::setFaradayRotationConfig(const DDEConfig &config)
{
    itsModelOptions[FARADAY_ROTATION] = true;
    itsConfigFaradayRotation = config;
}

void ModelConfig::clearFaradayRotationConfig()
{
    itsConfigFaradayRotation = DDEConfig();
    itsModelOptions[FARADAY_ROTATION] = false;
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

ostream &operator<<(ostream &out, const GainConfig &obj)
{
    out << indent << "Phasors: " << boolalpha << obj.phasors() << noboolalpha;
    return out;
}

ostream &operator<<(ostream &out, const CasaStringFilter &obj)
{
    for(unsigned int i = 0; i < obj.itsInverted.size(); ++i)
    {
        if(obj.itsInverted[i])
        {
            out << "!";
        }

        out << obj.itsRegEx[i].regexp();

        if(obj.itsInverted.size() > i + 1)
        {
            out << ";";
        }
    }

    return out;
}

ostream &operator<<(ostream &out, const DDEPartition &obj)
{
    out << "[";
    for(unsigned int i = 0; i < obj.size(); ++i)
    {
        if(obj.group(i))
        {
            out << obj.name(i) << ":" << obj.itsRegEx[i];
        }
        else
        {
            out << obj.itsRegEx[i];
        }

        if(obj.size() > i + 1)
        {
            out << ", ";
        }
    }

    if(obj.matchesRemainder())
    {
        if(obj.size() > 0)
        {
            out << ", ";
        }

        if(obj.groupRemainder())
        {
            out << obj.remainderGroupName() << ":*";
        }
        else
        {
            out << "*";
        }
    }
    out << "]";

    return out;
}

ostream &operator<<(ostream &out, const DDEConfig &obj)
{
    out << indent << "Partition: " << obj.partition();
    return out;
}

ostream &operator<<(ostream &out, const DirectionalGainConfig &obj)
{
    out << static_cast<const DDEConfig&>(obj);
    out << endl << indent << "Phasors: " << boolalpha << obj.phasors()
        << noboolalpha;
    return out;
}

ostream &operator<<(ostream &out, const BeamConfig &obj)
{
    out << static_cast<const DDEConfig&>(obj);
    out << endl << indent << "Mode: " << BeamConfig::asString(obj.mode())
        << endl << indent << "Conjugate array factor: " << boolalpha
        << obj.conjugateAF() << noboolalpha
        << endl << indent << "Element model path: "
        << obj.getElementPath().originalName();
    return out;
}

ostream &operator<<(ostream &out, const IonosphereConfig &obj)
{
    out << static_cast<const DDEConfig&>(obj);
    out << endl << indent << "Ionosphere model type: "
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

    out << endl << indent << "Gain enabled: " << boolalpha
        << obj.useGain() << noboolalpha;
    if(obj.useGain())
    {
        Indent id;
        out << endl << obj.getGainConfig();
    }

    out << endl << indent << "TEC enabled: " << boolalpha
        << obj.useTEC() << noboolalpha;

    out << endl << indent << "Direction dependent gain enabled: " << boolalpha
        << obj.useDirectionalGain() << noboolalpha;
    if(obj.useDirectionalGain())
    {
        Indent id;
        out << endl << obj.getDirectionalGainConfig();
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
    if(obj.useDirectionalTEC())
    {
        Indent id;
        out << endl << obj.getDirectionalTECConfig();
    }

    out << endl << indent << "Faraday rotation enabled: " << boolalpha
        << obj.useFaradayRotation() << noboolalpha;
    if(obj.useFaradayRotation())
    {
        Indent id;
        out << endl << obj.getFaradayRotationConfig();
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
