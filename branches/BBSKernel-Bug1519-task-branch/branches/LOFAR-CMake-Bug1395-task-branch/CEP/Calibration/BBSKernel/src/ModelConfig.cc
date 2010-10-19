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
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
namespace BBS 
{
using LOFAR::operator<<;

IonoConfig::IonoConfig():rank(0)
{
}


IonoConfig::~IonoConfig()
{
}

void IonoConfig::print(ostream &out) const
{
    out << "Iono:" << endl;
    Indent id;
    out << indent << "Rank: " << rank ;
}


BeamConfig::BeamConfig()
{
}


BeamConfig::~BeamConfig()
{
}

HamakerDipoleConfig::HamakerDipoleConfig()
    : BeamConfig()
{
}


HamakerDipoleConfig::~HamakerDipoleConfig()
{
}


void HamakerDipoleConfig::print(ostream &out) const
{
    out << "Beam:" << endl;
    Indent id;
    out << indent << "Type: " << type() << endl
        << indent << "Coefficient file: " << coeffFile;
}


const string &HamakerDipoleConfig::type() const
{
    static string typeName("HamakerDipole");
    return typeName;
}


YatawattaDipoleConfig::YatawattaDipoleConfig()
{
}


YatawattaDipoleConfig::~YatawattaDipoleConfig()
{
}


void YatawattaDipoleConfig::print(ostream &out) const
{
    out << "Beam:" << endl;
    Indent id;
    out << indent << "Type: " << type() << endl
        << indent << "Module theta: " << moduleTheta << endl
        << indent << "Module phi: " << modulePhi;
}


const string &YatawattaDipoleConfig::type() const
{
    static string typeName("YatawattaDipole");
    return typeName;
}


ModelConfig::ModelConfig()
    : usePhasors(false)
{
}


ModelConfig::~ModelConfig()
{
}


ostream& operator<<(ostream &out, const ModelConfig &obj)
{
    out << "Model configuration:" << endl;
    Indent id;
    out << indent << "Use phasors: "
        << boolalpha
        << obj.usePhasors
        << noboolalpha << endl
        << indent << "Sources: " << obj.sources << endl
        << indent << "Components: " << obj.components;

    if(obj.beamConfig)
    {
        out << endl << indent;
        obj.beamConfig->print(out);
    }
    if(obj.ionoConfig)
    {
        out << endl << indent;
        obj.ionoConfig->print(out);
    }

    return out;
}    


} //# namespace BBS
} //# namespace LOFAR
