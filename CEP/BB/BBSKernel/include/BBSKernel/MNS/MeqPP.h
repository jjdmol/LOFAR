//# MeqPP.h: PiercePoints for a direction (ra,dec) on the sky.
//#
//# Copyright (C) 2007
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
//# $Id: MeqPP.h 10458 2007-08-22 09:45:30Z zwieten $

#ifndef MNS_MEQPP_H
#define MNS_MEQPP_H

// piercepoints through the ionosphere

#include <BBSKernel/MNS/MeqExpr.h>
#include <BBSKernel/MNS/MeqResultVec.h>

#ifdef EXPR_GRAPH
#include <Common/lofar_string.h>
#endif

namespace LOFAR
{
namespace BBS
{
class MeqSource;
class MeqStation;
class MeqRequest;
class MeqMatrix;
  
const double iono_height=400000.; //height (m) 

// \ingroup BBSKernel
// \addtogroup MNS
// @{
class MeqPP: public MeqExprRep
{
public:
    enum
    {
        IN_AZEL,
        IN_X,
        IN_Y,
        IN_Z,
        N_InputPort
    } InputPort;
    
  MeqPP(MeqExpr &direction, MeqStation &station);
  MeqResultVec getResultVec(const MeqRequest &request);
    
private:
    void evaluate(const MeqRequest& request, const MeqMatrix &in_az,
        const MeqMatrix &in_el, const MeqMatrix &in_x, const MeqMatrix &in_y,
	const MeqMatrix &in_z, MeqMatrix &out_x, MeqMatrix &out_y, MeqMatrix &out_z, MeqMatrix &out_alpha);

#ifdef EXPR_GRAPH
    virtual std::string getLabel();
#endif
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
