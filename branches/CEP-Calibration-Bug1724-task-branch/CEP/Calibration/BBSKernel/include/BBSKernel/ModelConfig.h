//# ModelConfig.h: Aggregation of all the model configuration options.
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

#ifndef LOFAR_BBSKERNEL_MODELCONFIG_H
#define LOFAR_BBSKERNEL_MODELCONFIG_H

// \file
// Aggregation of all the model configuration options.

#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iosfwd.h>

#include <casa/OS/Path.h>
#include <casa/Utilities/Regex.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

// Configuration options specific to the direction independent gain model.
class GainConfig
{
public:
    explicit GainConfig(bool phasors = false);
    bool phasors() const;

private:
    bool    itsPhasors;
};

// Configuration options specific to direction dependent models.
class DDEConfig
{
public:
    DDEConfig();
    virtual ~DDEConfig();

    template <typename T>
    void setPatchFilter(T first, T last);

    bool enabled(const string &patch) const;

    friend ostream &operator<<(ostream &out, const DDEConfig &obj);

private:
    casa::Regex     itsRegEx;
};

// Configuration options specific to the direction dependent gain model.
class DirectionalGainConfig: public DDEConfig
{
public:
    explicit DirectionalGainConfig(bool phasors = false);
    bool phasors() const;

private:
    bool    itsPhasors;
};

// Configuration options specific to the beam model.
class BeamConfig: public DDEConfig
{
public:
    enum Mode
    {
        DEFAULT,
        ELEMENT,
        ARRAY_FACTOR,
        N_Mode
    };

    explicit BeamConfig(Mode mode = DEFAULT, bool conjugateAF = false,
        const casa::Path &elementPath = casa::Path());

    Mode mode() const;
    bool conjugateAF() const;
    const casa::Path &getElementPath() const;

    static bool isDefined(Mode in);
    static Mode asMode(const string &in);
    static const string &asString(Mode in);

private:
    Mode            itsMode;
    bool            itsConjugateAF;
    casa::Path      itsElementPath;
};

// Configuration options specific to the ionospheric model.
class IonosphereConfig: public DDEConfig
{
public:
    enum ModelType
    {
        MIM,
        EXPION,
        N_ModelType
    };

    explicit IonosphereConfig(ModelType type = N_ModelType,
        unsigned int degree = 0);
    ModelType getModelType() const;
    unsigned int degree() const;

    static bool isDefined(ModelType in);
    static ModelType asModelType(const string &in);
    static const string &asString(ModelType in);

private:
    ModelType       itsModelType;
    unsigned int    itsDegree;
};

// Configuration options specific to the condition number flagger.
class FlaggerConfig
{
public:
    explicit FlaggerConfig(double threshold = 1.0);
    double threshold() const;

private:
    double itsThreshold;
};

// Configuration of the LOFAR sky / environment / instrument model.
class ModelConfig
{
public:
    ModelConfig();

    bool useBandpass() const;
    void setBandpass(bool value = true);

    bool useClock() const;
    void setClock(bool value = true);

    bool useGain() const;
    void setGainConfig(const GainConfig &config);
    const GainConfig &getGainConfig() const;
    void clearGainConfig();

    bool useTEC() const;
    void setTEC(bool value = true);

    bool useDirectionalGain() const;
    void setDirectionalGainConfig(const DirectionalGainConfig &config);
    const DirectionalGainConfig &getDirectionalGainConfig() const;
    void clearDirectionalGainConfig();

    bool useBeam() const;
    void setBeamConfig(const BeamConfig &config);
    const BeamConfig &getBeamConfig() const;
    void clearBeamConfig();

    bool useDirectionalTEC() const;
    void setDirectionalTECConfig(const DDEConfig &config);
    const DDEConfig &getDirectionalTECConfig() const;
    void clearDirectionalTECConfig();

    bool useFaradayRotation() const;
    void setFaradayRotationConfig(const DDEConfig &config);
    const DDEConfig &getFaradayRotationConfig() const;
    void clearFaradayRotationConfig();

    bool useIonosphere() const;
    void setIonosphereConfig(const IonosphereConfig &config);
    const IonosphereConfig &getIonosphereConfig() const;
    void clearIonosphereConfig();

    bool useFlagger() const;
    void setFlaggerConfig(const FlaggerConfig &config);
    const FlaggerConfig &getFlaggerConfig() const;
    void clearFlaggerConfig();

    bool useCache() const;
    void setCache(bool value = true);

    void setSources(const vector<string> &sources);
    const vector<string> &getSources() const;

private:
    enum ModelOptions
    {
        BANDPASS,
        CLOCK,
        GAIN,
        TEC,
        DIRECTIONAL_GAIN,
        BEAM,
        DIRECTIONAL_TEC,
        FARADAY_ROTATION,
        IONOSPHERE,
        FLAGGER,
        CACHE,
        N_ModelOptions
    };

    bool                    itsModelOptions[N_ModelOptions];

    GainConfig              itsConfigGain;
    DirectionalGainConfig   itsConfigDirectionalGain;
    BeamConfig              itsConfigBeam;
    DDEConfig               itsConfigDirectionalTEC;
    DDEConfig               itsConfigFaradayRotation;
    IonosphereConfig        itsConfigIonosphere;
    FlaggerConfig           itsConfigFlagger;

    vector<string>          itsSources;
};

ostream &operator<<(ostream &out, const GainConfig &obj);
ostream &operator<<(ostream &out, const DDEConfig &obj);
ostream &operator<<(ostream &out, const DirectionalGainConfig &obj);
ostream &operator<<(ostream &out, const BeamConfig &obj);
ostream &operator<<(ostream &out, const IonosphereConfig &obj);
ostream &operator<<(ostream &out, const FlaggerConfig &obj);
ostream &operator<<(ostream &out, const ModelConfig &obj);

// @}

// -------------------------------------------------------------------------- //
// - DDEConfig implementation                                               - //
// -------------------------------------------------------------------------- //

template <typename T>
void DDEConfig::setPatchFilter(T first, T last)
{
    if(first == last)
    {
        itsRegEx = casa::Regex(".*");
        return;
    }

    casa::String regex("(");
    regex += casa::Regex::fromPattern(*first++);
    for(; first != last; ++first)
    {
        regex += ")|(";
        regex += casa::Regex::fromPattern(*first);
    }
    regex += ")";

    itsRegEx = casa::Regex(regex);
    ASSERT(itsRegEx.OK());
}

} //# namespace BBS
} //# namespace LOFAR

#endif
