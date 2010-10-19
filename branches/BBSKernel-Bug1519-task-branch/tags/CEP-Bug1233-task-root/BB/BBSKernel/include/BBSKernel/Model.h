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
class ModelConfig;
class Instrument;
class MeqParmGroup;
class MeqPhaseRef;
class MeqRequest;
class MeqStation;
class MeqStatUVW;
class MeqLMN;
class MeqJonesExpr;
class BeamCoeff;

class Model
{
public:
    typedef shared_ptr<Model> Pointer;

    enum ModelComponent
    {
        BANDPASS = 0,
        GAIN,
        DIRECTIONAL_GAIN,
        BEAM,
        N_ModelComponent
    };

    enum EquationType
    {
        UNSET = 0,
        SIMULATE,
        CORRECT,
        N_EquationType
    };

    Model(const Instrument &instrument, MeqParmGroup &parmGroup,
        ParmDB::ParmDB *skyDBase, MeqPhaseRef *phaseRef);

    void setStationUVW(const Instrument &instrument, VisData::Pointer buffer);

    void makeEquations(EquationType type, const ModelConfig &config,
        const vector<baseline_t> &baselines, MeqParmGroup &parmGroup,
        ParmDB::ParmDB *instrumentDBase, MeqPhaseRef *phaseRef,
        VisData::Pointer buffer);

    void clearEquations();
    
    EquationType getEquationType()
    { return itsEquationType; }

    void precalculate(const MeqRequest& request);

    MeqJonesResult evaluate(baseline_t baseline, const MeqRequest& request);

private:
    void makeStationNodes(const Instrument &instrument,
        const MeqPhaseRef &phaseRef);

    void makeSourceNodes(const vector<string> &names, MeqPhaseRef *phaseRef);

    void makeBeamNodes(const ModelConfig &config,
        LOFAR::ParmDB::ParmDB *db, MeqParmGroup &group,
        vector<vector<MeqJonesExpr> > &result) const;

    BeamCoeff readBeamCoeffFile(const string &filename) const;

    scoped_ptr<MeqSourceList>           itsSourceList;
    vector<shared_ptr<MeqStation> >     itsStationNodes;
    vector<shared_ptr<MeqStatUVW> >     itsUVWNodes;
    vector<MeqSource*>                  itsSourceNodes;
    vector<MeqLMN*>                     itsLMNNodes;
    map<baseline_t, MeqJonesExpr>       itsEquations;
    EquationType                        itsEquationType;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
