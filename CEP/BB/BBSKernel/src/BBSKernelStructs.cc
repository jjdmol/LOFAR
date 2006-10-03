//# BBSKernelStructs.h: some global structs used in the kernel.
//#
//# Copyright (C) 2006
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

#include <BBSKernel/BBSKernelStructs.h>
#include <BBSKernel/MNS/MeqDomain.h>
#include <Common/StreamUtil.h>

#include <iostream>

using namespace std;

namespace LOFAR
{
namespace BBS
{
    using LOFAR::operator<<;
    
    ostream& operator<<(ostream& os, const Correlation& obj)
    {
        os << "Correlation:" << endl;
        os << ". " << "Selection: ";
        switch(obj.selection)
        {
            case Correlation::AUTO:
                os << ". " << "AUTO";
                break;
    
            case Correlation::CROSS:
                os << ". " << "CROSS";
                break;
    
            case Correlation::ALL:
                os << ". " << "ALL";
                break;
            
            default:
                os << ". " << "*****";
                break;
        }
        os << endl;
        os << ". " << "Type:" << obj.type << endl;
        return os;
    }

    ostream& operator<<(ostream& os, const Baselines& obj)
    {
        os << "Baselines:" << endl;
        os << ". " << "Station1: " << obj.station1 << endl;
        os << ". " << "Station2: " << obj.station2 << endl;
        return os;
    }

} // namespace BBS
} // namespace LOFAR
