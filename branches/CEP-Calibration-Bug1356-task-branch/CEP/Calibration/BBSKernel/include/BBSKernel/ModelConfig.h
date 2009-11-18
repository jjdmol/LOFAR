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

// Configuration options specific to Hamaker's dipole beam model.
class HamakerDipoleConfig
{
public:
    HamakerDipoleConfig();
    HamakerDipoleConfig(const string &file);

    const string &getCoeffFile() const;

private:
    string  itsCoeffFile;
};

// Configuration options specific to Yatawatta's dipole beam model.
class YatawattaDipoleConfig
{
public:
    YatawattaDipoleConfig();
    YatawattaDipoleConfig(const string &theta, const string &phi);

    const string &getModuleTheta() const;
    const string &getModulePhi() const;

private:
    string  itsModuleTheta;
    string  itsModulePhi;
};

// Configuration options specific to Mevius' minimal ionospheric model.
class IonosphereConfig
{
public:
    IonosphereConfig();
    IonosphereConfig(unsigned int degree);

    unsigned int getDegree() const;

private:
    unsigned int itsDegree;
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
    enum BeamType
    {
        UNKNOWN_BEAM_TYPE,
        HAMAKER_DIPOLE,
        YATAWATTA_DIPOLE
    };

    ModelConfig();

    bool usePhasors() const;
    void setPhasors(bool value = true);

    bool useBandpass() const;
    void setBandpass(bool value = true);

    void setIsotropicGain(bool value = true);
    bool useIsotropicGain() const;

    void setAnisotropicGain(bool value = true);
    bool useAnisotropicGain() const;

    bool useBeam() const;
    BeamType getBeamType() const;
    void setBeamConfig(const HamakerDipoleConfig &config);
    void setBeamConfig(const YatawattaDipoleConfig &config);
    void getBeamConfig(HamakerDipoleConfig &config) const;
    void getBeamConfig(YatawattaDipoleConfig &config) const;
    void clearBeamConfig();

    bool useIonosphere() const;
    void setIonosphereConfig(const IonosphereConfig &config);
    void getIonosphereConfig(IonosphereConfig &config) const;
    void clearIonosphereConfig();

    bool useFlagger() const;
    void setFlaggerConfig(const FlaggerConfig &config);
    void getFlaggerConfig(FlaggerConfig &config) const;
    void clearFlaggerConfig();

    void setSources(const vector<string> &sources);
    const vector<string> &getSources() const;

private:
    enum ModelOptions
    {
        PHASORS,
        BANDPASS,
        ISOTROPIC_GAIN,
        ANISOTROPIC_GAIN,
        BEAM,
        IONOSPHERE,
        FLAGGER,
        N_ModelOptions
    };

    bool                    itsModelOptions[N_ModelOptions];
    vector<string>          itsSources;

    BeamType                itsBeamType;
    HamakerDipoleConfig     itsConfigBeamHamakerDipole;
    YatawattaDipoleConfig   itsConfigBeamYatawattaDipole;

    IonosphereConfig        itsConfigIonosphere;
    FlaggerConfig           itsConfigFlagger;
};

ostream &operator<<(ostream &out, const HamakerDipoleConfig &obj);
ostream &operator<<(ostream &out, const YatawattaDipoleConfig &obj);
ostream &operator<<(ostream &out, const FlaggerConfig &obj);
ostream &operator<<(ostream &out, const IonosphereConfig &obj);
ostream &operator<<(ostream &out, const ModelConfig &obj);

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
