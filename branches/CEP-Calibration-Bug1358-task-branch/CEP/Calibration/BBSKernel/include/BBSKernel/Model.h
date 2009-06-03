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
#include <BBSKernel/VisData.h>

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/ExprResult.h>
#include <BBSKernel/Expr/ExprParm.h>
#include <BBSKernel/Expr/PhaseRef.h>
//#include <BBSKernel/Expr/JonesResult.h>
//#include <BBSKernel/Expr/JonesExpr.h>
#include <BBSKernel/Expr/Source.h>
#include <BBSKernel/Expr/Cache.h>

#include <BBSKernel/Expr/StatUVW.h>

#include <ParmDB/ParmDB.h>
#include <ParmDB/SourceDB.h>

#include <boost/multi_array.hpp>


namespace LOFAR
{
namespace BBS
{
class ModelConfig;
class Instrument;
class PhaseRef;
class Request;
//class BeamCoeff;

class Model
{
public:
    typedef shared_ptr<Model>       Pointer;
    typedef shared_ptr<const Model> ConstPointer;

    enum ModelComponent
    {
        BANDPASS,
        GAIN,
        DIRECTIONAL_GAIN,
        BEAM,
        IONOSPHERE,
        N_ModelComponent
    };

    Model(const Instrument &instrument, const SourceDB &sourceDb,
        const casa::MDirection &phaseRef);

    void clearExpressions();

    void makeFwdExpressions(const ModelConfig &config,
        const VisData::Pointer &chunk, const vector<baseline_t> &baselines);

//    bool makeInvExpressions(const ModelConfig &config,
//        const VisData::Pointer &chunk, const vector<baseline_t> &baselines);

    void setPerturbedParms(const ParmGroup &solvables);
    void clearPerturbedParms();

    ParmGroup getParms() const;
    ParmGroup getPerturbedParms() const;

//    void precalculate(const Request &request);
//    JonesResult evaluate(const baseline_t &baseline, const Request &request);

//    void setRequest(const Request &request);
    void setRequestGrid(const Grid &grid);

    ExprResult::ConstPtr evaluate(const baseline_t &baseline);
    map<PValueKey, ExprResult::ConstPtr>
        evalPerturbed(const baseline_t &baseline);

//    JonesMatrix<complex_t>::ConstPtr evaluate(const baseline_t &baseline,
//        Cache &cache);

//    map<PValueKey, JonesMatrix<complex_t>::ConstPtr>
//        evalPerturbed(const baseline_t &baseline, Cache &cache);

private:
    vector<bool> parseComponents(const vector<string> &components) const;

    vector<Expr::Ptr> makeUVWExpr(const VisData::Pointer &chunk,
        const vector<baseline_t> &baselines);
    
    ExprParm::Ptr makeExprParm(uint category, const string &name);

    boost::multi_array<Expr::Ptr, 2>
    makeDirectionDependentGainExpr(const ModelConfig &config,
        const vector<Source::Ptr> &sources);
    
    vector<Source::Ptr> makeSourceList(const vector<string> &patterns);
    Source::Ptr makeSource(const SourceInfo &source);
    
//    void makeSources(vector<Source::Pointer> &result,
//        const vector<string> &patterns);
//    
//    Source::Pointer makeSource(const SourceInfo &source);

    void makeStationUvw();
    
//    void makeAzElNodes(boost::multi_array<Expr, 2> &result,
//        const vector<Source::Pointer> &sources) const;

    void makeStationShiftNodes(boost::multi_array<Expr::Ptr, 2> &result,
        const vector<Source::Ptr> &sources) const;

//    void makeBandpassNodes(vector<JonesExpr> &result);
//    
//    void makeGainNodes(vector<JonesExpr> &result, const ModelConfig &config);

//    void makeDirectionalGainNodes(boost::multi_array<JonesExpr, 2> &result,
//        const ModelConfig &config, const vector<Source::Pointer> &sources);

//    void makeDipoleBeamNodes(boost::multi_array<JonesExpr, 2> &result,
//        const ModelConfig &config, const boost::multi_array<Expr, 2> &azel);

//    void makeIonosphereNodes(boost::multi_array<JonesExpr, 2> &result,
//        const ModelConfig &config, const boost::multi_array<Expr, 2> &azel);

//    BeamCoeff readBeamCoeffFile(const string &filename) const;

    Instrument                      itsInstrument;
    SourceDB                        itsSourceDb;
    PhaseRef::ConstPointer          itsPhaseRef;
//    Grid                            itsRequestGrid;
    Request                         itsRequest;
    Cache                           itsCache;
    
    vector<StatUVW::ConstPointer>   itsStationUvw;
    map<baseline_t, Expr::Ptr>      itsExpressions;
//    map<baseline_t, vector<PValueKey> >                 itsDependencies;
    map<unsigned int, ExprParm::Ptr>                    itsParms;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
