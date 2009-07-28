//# ModelConfig.h: Aggregation of all the model configuration options.
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

// \ingroup BBSKernel
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

    bool usePhasors() const
    {
        return itsModelOptions[PHASORS];
    }

    bool useBandpass() const
    {
        return itsModelOptions[BANDPASS];
    }

    bool useIsotropicGain() const
    {
        return itsModelOptions[ISOTROPIC_GAIN];
    }

    bool useAnisotropicGain() const
    {
        return itsModelOptions[ANISOTROPIC_GAIN];
    }

    bool useBeam() const
    {
        return itsModelOptions[BEAM];
    }

    BeamType getBeamType() const
    {
        return itsBeamType;
    }

    void getBeamConfig(HamakerDipoleConfig &config) const
    {
        config = itsConfigBeamHamakerDipole;
    }

    void getBeamConfig(YatawattaDipoleConfig &config) const
    {
        config = itsConfigBeamYatawattaDipole;
    }

    bool useIonosphere() const
    {
        return itsModelOptions[IONOSPHERE];
    }

    void getIonosphereConfig(IonosphereConfig &config) const
    {
        config = itsConfigIonosphere;
    }

    bool useFlagger() const
    {
        return itsModelOptions[FLAGGER];
    }

    void getFlaggerConfig(FlaggerConfig &config) const
    {
        config = itsConfigFlagger;
    }

    void setPhasors(bool value = true)
    {
        itsModelOptions[PHASORS] = value;
    }

    void setBandpass(bool value = true)
    {
        itsModelOptions[BANDPASS] = value;
    }

    void setIsotropicGain(bool value = true)
    {
        itsModelOptions[ISOTROPIC_GAIN] = value;
    }

    void setAnisotropicGain(bool value = true)
    {
        itsModelOptions[ANISOTROPIC_GAIN] = value;
    }

    void setBeamConfig(const HamakerDipoleConfig &config)
    {
        itsModelOptions[BEAM] = true;
        itsBeamType = HAMAKER_DIPOLE;
        itsConfigBeamHamakerDipole = config;
    }

    void setBeamConfig(const YatawattaDipoleConfig &config)
    {
        itsModelOptions[BEAM] = true;
        itsBeamType = YATAWATTA_DIPOLE;
        itsConfigBeamYatawattaDipole = config;
    }

    void clearBeamConfig()
    {
        itsConfigBeamHamakerDipole = HamakerDipoleConfig();
        itsConfigBeamYatawattaDipole = YatawattaDipoleConfig();
        itsBeamType = UNKNOWN_BEAM_TYPE;
        itsModelOptions[BEAM] = false;
    }

    void setIonosphereConfig(const IonosphereConfig &config)
    {
        itsModelOptions[IONOSPHERE] = true;
        itsConfigIonosphere = config;
    }

    void clearIonosphereConfig()
    {
        itsConfigIonosphere = IonosphereConfig();
        itsModelOptions[IONOSPHERE] = false;
    }

    void setFlaggerConfig(const FlaggerConfig &config)
    {
        itsModelOptions[FLAGGER] = true;
        itsConfigFlagger = config;
    }

    void clearFlaggerConfig()
    {
        itsConfigFlagger = FlaggerConfig();
        itsModelOptions[FLAGGER] = false;
    }

    void setSources(const vector<string> &sources)
    {
        itsSources = sources;
    }

    const vector<string> &getSources() const
    {
        return itsSources;
    }

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
