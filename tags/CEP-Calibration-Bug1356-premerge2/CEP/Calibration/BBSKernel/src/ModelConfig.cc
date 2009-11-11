//# ModelConfig.cc: Aggregation of all the model configuration options.
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
// - HamakerDipoleConfig implementation                                     - //
// -------------------------------------------------------------------------- //

HamakerDipoleConfig::HamakerDipoleConfig()
{
}

HamakerDipoleConfig::HamakerDipoleConfig(const string &file)
    :   itsCoeffFile(file)
{
}

const string &HamakerDipoleConfig::getCoeffFile() const
{
    return itsCoeffFile;
}

// -------------------------------------------------------------------------- //
// - YatawattaDipoleConfig implementation                                   - //
// -------------------------------------------------------------------------- //

YatawattaDipoleConfig::YatawattaDipoleConfig()
{
}

YatawattaDipoleConfig::YatawattaDipoleConfig(const string &theta,
    const string &phi)
    :   itsModuleTheta(theta),
        itsModulePhi(phi)
{
}

const string &YatawattaDipoleConfig::getModuleTheta() const
{
    return itsModuleTheta;
}

const string &YatawattaDipoleConfig::getModulePhi() const
{
    return itsModulePhi;
}

// -------------------------------------------------------------------------- //
// - IonosphereConfig implementation                                        - //
// -------------------------------------------------------------------------- //

IonosphereConfig::IonosphereConfig()
    :   itsDegree(0)
{
}

IonosphereConfig::IonosphereConfig(unsigned int degree)
    :   itsDegree(degree)
{
}

unsigned int IonosphereConfig::getDegree() const
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
    :   itsBeamType(UNKNOWN_BEAM_TYPE)
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

bool ModelConfig::useIsotropicGain() const
{
    return itsModelOptions[ISOTROPIC_GAIN];
}

bool ModelConfig::useAnisotropicGain() const
{
    return itsModelOptions[ANISOTROPIC_GAIN];
}

bool ModelConfig::useBeam() const
{
    return itsModelOptions[BEAM];
}

ModelConfig::BeamType ModelConfig::getBeamType() const
{
    return itsBeamType;
}

void ModelConfig::getBeamConfig(HamakerDipoleConfig &config) const
{
    config = itsConfigBeamHamakerDipole;
}

void ModelConfig::getBeamConfig(YatawattaDipoleConfig &config) const
{
    config = itsConfigBeamYatawattaDipole;
}

bool ModelConfig::useIonosphere() const
{
    return itsModelOptions[IONOSPHERE];
}

void ModelConfig::getIonosphereConfig(IonosphereConfig &config) const
{
    config = itsConfigIonosphere;
}

bool ModelConfig::useFlagger() const
{
    return itsModelOptions[FLAGGER];
}

void ModelConfig::getFlaggerConfig(FlaggerConfig &config) const
{
    config = itsConfigFlagger;
}

void ModelConfig::setPhasors(bool value)
{
    itsModelOptions[PHASORS] = value;
}

void ModelConfig::setBandpass(bool value)
{
    itsModelOptions[BANDPASS] = value;
}

void ModelConfig::setIsotropicGain(bool value)
{
    itsModelOptions[ISOTROPIC_GAIN] = value;
}

void ModelConfig::setAnisotropicGain(bool value)
{
    itsModelOptions[ANISOTROPIC_GAIN] = value;
}

void ModelConfig::setBeamConfig(const HamakerDipoleConfig &config)
{
    itsModelOptions[BEAM] = true;
    itsBeamType = HAMAKER_DIPOLE;
    itsConfigBeamHamakerDipole = config;
}

void ModelConfig::setBeamConfig(const YatawattaDipoleConfig &config)
{
    itsModelOptions[BEAM] = true;
    itsBeamType = YATAWATTA_DIPOLE;
    itsConfigBeamYatawattaDipole = config;
}

void ModelConfig::clearBeamConfig()
{
    itsConfigBeamHamakerDipole = HamakerDipoleConfig();
    itsConfigBeamYatawattaDipole = YatawattaDipoleConfig();
    itsBeamType = UNKNOWN_BEAM_TYPE;
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

ostream &operator<<(ostream &out, const HamakerDipoleConfig &obj)
{
    out << indent << "Type: HamakerDipole" << endl
        << indent << "Coefficient file: " << obj.getCoeffFile();
    return out;
}

ostream &operator<<(ostream &out, const YatawattaDipoleConfig &obj)
{
    out << indent << "Type: YatawattaDipole" << endl
        << indent << "Module theta: " << obj.getModuleTheta() << endl
        << indent << "Module phi: " << obj.getModulePhi();
    return out;
}

ostream &operator<<(ostream &out, const FlaggerConfig &obj)
{
    out << indent << "Threshold: " << obj.getThreshold();
    return out;
}

ostream &operator<<(ostream &out, const IonosphereConfig &obj)
{
    out << indent << "Degree: " << obj.getDegree();
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
    out << endl << indent << "Isotropic gain enabled: " << boolalpha
        << obj.useIsotropicGain() << noboolalpha;
    out << endl << indent << "Anisotropic gain enabled: " << boolalpha
        << obj.useAnisotropicGain() << noboolalpha;

    out << endl << indent << "Beam enabled: " << boolalpha << obj.useBeam()
        << noboolalpha;
    if(obj.useBeam())
    {
        Indent id;

        switch(obj.getBeamType())
        {
        case ModelConfig::HAMAKER_DIPOLE:
            {
                HamakerDipoleConfig config;
                obj.getBeamConfig(config);
                out << endl << config;
                break;
            }
        case ModelConfig::YATAWATTA_DIPOLE:
            {
                YatawattaDipoleConfig config;
                obj.getBeamConfig(config);
                out << endl << config;
                break;
            }
        default:
            out << endl << indent << "Type: <unknown>";
        }
    }

    out << endl << indent << "Ionosphere enabled: " << boolalpha
        << obj.useIonosphere() << noboolalpha;
    if(obj.useIonosphere())
    {
        IonosphereConfig config;
        obj.getIonosphereConfig(config);

        Indent id;
        out << endl << config;
    }

    out << endl << indent << "Flagger enabled: " << boolalpha
        << obj.useFlagger() << noboolalpha;
    if(obj.useFlagger())
    {
        FlaggerConfig config;
        obj.getFlaggerConfig(config);

        Indent id;
        out << endl << config;
    }

    out << endl << indent << "Sources: " << obj.getSources();
    return out;
}

} //# namespace BBS
} //# namespace LOFAR
