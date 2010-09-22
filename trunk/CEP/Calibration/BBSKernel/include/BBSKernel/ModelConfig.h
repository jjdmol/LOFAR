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

#include <casa/OS/Path.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

// Configuration options specific to the beam model.
class BeamConfig
{
public:
    enum ElementType
    {
        HAMAKER_LBA,
        HAMAKER_HBA,
        YATAWATTA_LBA,
        YATAWATTA_HBA,
        N_ElementType
    };

    BeamConfig();
    BeamConfig(const string &configName, const casa::Path &configPath,
        ElementType elementType, const casa::Path &elementPath);

    const string &getConfigName() const;
    const casa::Path &getConfigPath() const;
    ElementType getElementType() const;
    const casa::Path &getElementPath() const;

    static bool isDefined(ElementType in);
    static ElementType asElementType(const string &in);
    static const string &asString(ElementType in);

private:
    string          itsConfigName;
    casa::Path      itsConfigPath;
    ElementType     itsElementType;
    casa::Path      itsElementPath;
};

// Configuration options specific to the ionospheric model.
class IonosphereConfig
{
public:
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

    double getThreshold() const;

private:
    double itsThreshold;
};

// Configuration of the LOFAR sky / environment / instrument model.
class ModelConfig
{
public:
    ModelConfig();

    bool usePhasors() const;
    void setPhasors(bool value = true);

    bool useBandpass() const;
    void setBandpass(bool value = true);

    bool useClock() const;
    void setClock(bool value = true);

    bool useGain() const;
    void setGain(bool value = true);

    bool useDirectionalGain() const;
    void setDirectionalGain(bool value = true);

    bool useFaradayRotation() const;
    void setFaradayRotation(bool value = true);

    bool useBeam() const;
    void setBeamConfig(const BeamConfig &config);
    const BeamConfig &getBeamConfig() const;
    void clearBeamConfig();

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
        PHASORS,
        BANDPASS,
        CLOCK,
        GAIN,
        DIRECTIONAL_GAIN,
        FARADAY_ROTATION,
        BEAM,
        IONOSPHERE,
        FLAGGER,
        CACHE,
        N_ModelOptions
    };

    bool                itsModelOptions[N_ModelOptions];

    BeamConfig          itsConfigBeam;
    IonosphereConfig    itsConfigIonosphere;
    FlaggerConfig       itsConfigFlagger;

    vector<string>      itsSources;
};

ostream &operator<<(ostream &out, const FlaggerConfig &obj);
ostream &operator<<(ostream &out, const IonosphereConfig &obj);
ostream &operator<<(ostream &out, const BeamConfig &obj);
ostream &operator<<(ostream &out, const ModelConfig &obj);

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
