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
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::operator<<;

CondNumFlagConfig::CondNumFlagConfig()
    :   threshold(1.0)
{
}

void CondNumFlagConfig::print(ostream &out) const
{
    out << "Condition number flagging:" << endl;
    Indent id;
    out << indent << "Threshold: " << threshold;
}

IonoConfig::IonoConfig()
    :   rank(0)
{
}

void IonoConfig::print(ostream &out) const
{
    out << "Ionosphere:" << endl;
    Indent id;
    out << indent << "Rank: " << rank;
}

BeamConfig::~BeamConfig()
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
    :   usePhasors(false)
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

    if(obj.ionoConfig)
    {
        out << endl << indent;
        obj.ionoConfig->print(out);
    }

    if(obj.beamConfig)
    {
        out << endl << indent;
        obj.beamConfig->print(out);
    }

    if(obj.condNumFlagConfig)
    {
        out << endl << indent;
        obj.condNumFlagConfig->print(out);
    }

    return out;
}

} //# namespace BBS
} //# namespace LOFAR
