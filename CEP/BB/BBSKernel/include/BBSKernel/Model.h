//# Model.h:
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
//# $Id$


#ifndef LOFAR_BBS_BBSKERNEL_MODEL_H
#define LOFAR_BBS_BBSKERNEL_MODEL_H

#include <Common/lofar_smartptr.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>
#include <Common/lofar_set.h>

#include <BBSKernel/VisData.h>
#include <BBSKernel/MNS/MeqJonesResult.h>
#include <BBSKernel/MNS/MeqSourceList.h>

#include <ParmDB/ParmDB.h>


namespace LOFAR
{
namespace BBS
{
class Instrument;
class MeqParmGroup;
class MeqPhaseRef;
class MeqRequest;
class MeqStation;
class MeqStatUVW;
class MeqLMN;
class MeqJonesExpr;

class Model
{
public:
    typedef shared_ptr<Model> Pointer;

    enum ModelComponent
    {
        BANDPASS = 0,
        GAIN,
        DIRECTIONAL_GAIN,
        DIPOLE_BEAM,
        PHASORS,
        N_ModelComponent
    };

    enum EquationType
    {
        PREDICT = 0,
        CORRECT,
        N_EquationType
    };

    Model(const Instrument &instrument, MeqParmGroup &parmGroup,
        ParmDB::ParmDB *skyDBase, MeqPhaseRef *phaseRef);

    void setStationUVW(const Instrument &instrument, VisData::Pointer buffer);

    void makeEquations(EquationType type, const vector<string> &components,
        const set<baseline_t> &baselines, const vector<string> &sources,
        MeqParmGroup &parmGroup, ParmDB::ParmDB *instrumentDBase,
        MeqPhaseRef *phaseRef, VisData::Pointer buffer);

    void clearEquations();

    void precalculate(const MeqRequest& request);

    MeqJonesResult evaluate(baseline_t baseline, const MeqRequest& request);

private:
    void makeStationNodes(const Instrument &instrument,
        const MeqPhaseRef &phaseRef);

    void makeSourceNodes(const vector<string> &names, MeqPhaseRef *phaseRef);

    scoped_ptr<MeqSourceList>           itsSourceList;
    vector<shared_ptr<MeqStation> >     itsStationNodes;
    vector<shared_ptr<MeqStatUVW> >     itsUVWNodes;
    vector<MeqSource*>                  itsSourceNodes;
    vector<MeqLMN*>                     itsLMNNodes;
    map<baseline_t, MeqJonesExpr>       itsEquations;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
