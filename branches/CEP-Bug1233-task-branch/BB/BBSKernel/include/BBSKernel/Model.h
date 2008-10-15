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
#include <iterator>

#include <BBSKernel/ParmManager.h>
#include <BBSKernel/Instrument.h>
//#include <BBSKernel/SourceList.h>
#include <BBSKernel/VisData.h>
#include <BBSKernel/MNS/ExprParm.h>
#include <BBSKernel/MNS/MeqPhaseRef.h>
#include <BBSKernel/MNS/MeqJonesResult.h>
#include <BBSKernel/MNS/MeqJonesExpr.h>
#include <BBSKernel/MNS/MeqSource.h>
#include <BBSKernel/MNS/MeqStatUVW.h>

#include <ParmDB/ParmDB.h>

#include <boost/multi_array.hpp>


namespace LOFAR
{
namespace BBS
{
class ModelConfig;
class Instrument;
//class ParmGroup;
class PhaseRef;
class Request;
class Station;
//class StatUVW;
//class LMN;
//class JonesExpr;
//class BeamCoeff;

class SourceDescriptor
{
public:
//    typedef shared_ptr<SourceList>          Pointer;
//    typedef shared_ptr<const SourceList>    ConstPointer;

    enum Type
    {
        POINT = 0,
        GAUSSIAN,
        N_Type
    };

    SourceDescriptor(const string &name, Type type)
        :   name(name),
            type(type)
    {}
    
    string  name;
    Type    type;
};

class Model
{
public:
    typedef shared_ptr<Model>       Pointer;
    typedef shared_ptr<const Model> ConstPointer;

    enum ModelComponent
    {
        BANDPASS = 0,
        GAIN,
        DIRECTIONAL_GAIN,
        BEAM,
        N_ModelComponent
    };

    Model(const Instrument &instrument, const casa::MDirection &phaseRef);

    void clearExpressions();

    bool makeFwdExpressions(const ModelConfig &config,
        const vector<baseline_t> &baselines);

    void setPerturbedParms(const ParmGroup &solvables);
    void clearPerturbedParms();

    ParmGroup getParms() const;
    ParmGroup getPerturbedParms() const;

    void precalculate(const Request &request);
    JonesResult evaluate(const baseline_t &baseline, const Request &request);

private:
    void makeStationUvw();

    Expr makeExprParm(uint category, const string &name);
    
//    void makeSources(vector<Source::Pointer> &result, ParmGroup &group,
//        vector<string> names) const;
    Source::Pointer makeSource(const SourceDescriptor &source);

    void makeGainNodes(vector<JonesExpr> &result, const ModelConfig &config,
        bool inverse = false);

    void makeDirectionalGainNodes(boost::multi_array<JonesExpr, 2> &result,
        const ModelConfig &config, const vector<Source::Pointer> &sources,
        bool inverse = false);

//    void makeBeamNodes(const ModelConfig &config,
//        LOFAR::ParmDB::ParmDB *db, ParmGroup &group,
//        vector<vector<JonesExpr> > &result) const;

//    BeamCoeff readBeamCoeffFile(const string &filename) const;

    Instrument                      itsInstrument;
//    SourceList                      itsSourceList;
    PhaseRef::ConstPointer          itsPhaseRef;
    vector<StatUVW::ConstPointer>   itsStationUvw;
    map<baseline_t, JonesExpr>      itsExpressions;
    map<uint, Expr>                 itsParms;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
