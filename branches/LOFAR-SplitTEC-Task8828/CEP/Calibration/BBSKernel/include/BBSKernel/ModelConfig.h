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

#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

// Configuration options specific to the clock.
class ClockConfig
{
public:
    ClockConfig();
    ClockConfig(bool splitClock);

    bool splitClock() const;

private:
    bool itsSplitClock;
};

// Configuration options specific to TEC.
class TECConfig
{
public:
    TECConfig();
    TECConfig(bool splitTEC);

    bool splitTEC() const;

private:
    bool itsSplitTEC;
};

// Configuration options specific to the direction independent gain model.
class GainConfig
{
public:
    explicit GainConfig(bool phasors = false);
    bool phasors() const;

private:
    bool    itsPhasors;
};

// Configuration options specific to the direction dependent gain model.
class DirectionalGainConfig
{
public:
    explicit DirectionalGainConfig(bool phasors = false);
    bool phasors() const;

private:
    bool    itsPhasors;
};

// Configuration options specific to the elevation cut-off.
class ElevationCutConfig
{
public:
    ElevationCutConfig();
    ElevationCutConfig(double threshold);

    double threshold() const;

private:
    double itsThreshold;
};

// Configuration options specific to the beam model.
class BeamConfig
{
public:
    int notdef;
    enum Mode
    {
        DEFAULT,
        ELEMENT,
        ARRAY_FACTOR,
        N_Mode
    };

    BeamConfig();
    BeamConfig(Mode mode, bool useChannelFreq);

    Mode mode() const;
    bool useChannelFreq() const;

    static bool isDefined(Mode in);
    static Mode asMode(const string &in);
    static const string &asString(Mode in);

private:
    Mode            itsMode;
    bool            itsUseChannelFreq;
};

// Configuration options specific to the ionospheric model.
class IonosphereConfig
{
public:
    int notdef;
    enum ModelType
    {
        MIM,
        EXPION,
        N_ModelType
    };

    IonosphereConfig();
    IonosphereConfig(ModelType type, unsigned int degree);

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
    FlaggerConfig();
    FlaggerConfig(double threshold);

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
    void setClockConfig(const ClockConfig &config);
    const ClockConfig &getClockConfig() const;
    void clearClockConfig();

    bool useGain() const;
    void setGainConfig(const GainConfig &config);
    const GainConfig &getGainConfig() const;
    void clearGainConfig();

    bool useTEC() const;
    void setTECConfig(const TECConfig &config);
    const TECConfig &getTECConfig() const;
    void clearTECConfig();

    bool useCommonRotation() const;
    void setCommonRotation(bool value = true);

    bool useCommonScalarPhase() const;
    void setCommonScalarPhase(bool value = true);

    bool useDirectionalGain() const;
    void setDirectionalGainConfig(const DirectionalGainConfig &config);
    const DirectionalGainConfig &getDirectionalGainConfig() const;
    void clearDirectionalGainConfig();

    bool useElevationCut() const;
    void setElevationCutConfig(const ElevationCutConfig &config);
    const ElevationCutConfig &getElevationCutConfig() const;
    void clearElevationCutConfig();

    bool useBeam() const;
    void setBeamConfig(const BeamConfig &config);
    const BeamConfig &getBeamConfig() const;
    void clearBeamConfig();

    bool useDirectionalTEC() const;
    void setDirectionalTEC(bool value = true);

    bool useFaradayRotation() const;
    void setFaradayRotation(bool value = true);

    bool useRotation() const;
    void setRotation(bool value = true);

    bool useScalarPhase() const;
    void setScalarPhase(bool value = true);

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
    const vector<string> &sources() const;

private:
    enum ModelOptions
    {
        BANDPASS,
        CLOCK,
        GAIN,
        TEC,
        COMMON_ROTATION,
        COMMON_SCALAR_PHASE,
        DIRECTIONAL_GAIN,
        ELEVATION_CUT,
        BEAM,
        DIRECTIONAL_TEC,
        FARADAY_ROTATION,
        ROTATION,
        SCALAR_PHASE,
        IONOSPHERE,
        FLAGGER,
        CACHE,
        N_ModelOptions
    };

    bool                    itsModelOptions[N_ModelOptions];

    ClockConfig             itsConfigClock;
    TECConfig               itsConfigTEC;
    GainConfig              itsConfigGain;
    DirectionalGainConfig   itsConfigDirectionalGain;
    ElevationCutConfig      itsConfigElevationCut;
    BeamConfig              itsConfigBeam;
    IonosphereConfig        itsConfigIonosphere;
    FlaggerConfig           itsConfigFlagger;

    vector<string>          itsSources;
};

ostream &operator<<(ostream &out, const ClockConfig &obj);
ostream &operator<<(ostream &out, const GainConfig &obj);
ostream &operator<<(ostream &out, const DirectionalGainConfig &obj);
ostream &operator<<(ostream &out, const ElevationCutConfig &obj);
ostream &operator<<(ostream &out, const BeamConfig &obj);
ostream &operator<<(ostream &out, const IonosphereConfig &obj);
ostream &operator<<(ostream &out, const FlaggerConfig &obj);
ostream &operator<<(ostream &out, const ModelConfig &obj);

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
