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

ModelConfig::ModelConfig()
    :   itsBeamType(UNKNOWN_BEAM_TYPE)
{
    fill(itsModelOptions, itsModelOptions + N_ModelOptions, false);
}

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
//        << indent << "Components: " << obj.components;
    return out;
}

} //# namespace BBS
} //# namespace LOFAR
