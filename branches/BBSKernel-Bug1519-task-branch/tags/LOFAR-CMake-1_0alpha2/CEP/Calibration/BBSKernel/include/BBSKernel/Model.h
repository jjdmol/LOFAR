//# Model.h:
//#
//# Copyright (C) 2007
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
#include <BBSKernel/Expr/ExprParm.h>
#include <BBSKernel/Expr/PhaseRef.h>
#include <BBSKernel/Expr/JonesResult.h>
#include <BBSKernel/Expr/JonesExpr.h>
#include <BBSKernel/Expr/Source.h>
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
class BeamCoeff;

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

    bool makeFwdExpressions(const ModelConfig &config,
        const vector<baseline_t> &baselines);

    bool makeInvExpressions(const ModelConfig &config,
        const VisData::Pointer &chunk, const vector<baseline_t> &baselines);

    void setPerturbedParms(const ParmGroup &solvables);
    void clearPerturbedParms();

    ParmGroup getParms() const;
    ParmGroup getPerturbedParms() const;

    void precalculate(const Request &request);
    JonesResult evaluate(const baseline_t &baseline, const Request &request);

private:
    vector<bool> parseComponents(const vector<string> &components) const;

    Expr makeExprParm(uint category, const string &name);

    void makeSources(vector<Source::Pointer> &result,
        const vector<string> &patterns);

    Source::Pointer makeSource(const SourceInfo &source);

    Expr makeSpectralIndex(const SourceInfo &source);

    void makeStationUvw();

    void makeAzElNodes(boost::multi_array<Expr, 2> &result,
        const vector<Source::Pointer> &sources) const;

    void makeStationShiftNodes(boost::multi_array<Expr, 2> &result,
        const vector<Source::Pointer> &sources) const;

    void makeBandpassNodes(vector<JonesExpr> &result);

    void makeGainNodes(vector<JonesExpr> &result, const ModelConfig &config);

    void makeDirectionalGainNodes(boost::multi_array<JonesExpr, 2> &result,
        const ModelConfig &config, const vector<Source::Pointer> &sources);

    void makeDipoleBeamNodes(boost::multi_array<JonesExpr, 2> &result,
        const ModelConfig &config, const boost::multi_array<Expr, 2> &azel);

    void makeIonosphereNodes(boost::multi_array<JonesExpr, 2> &result,
        const ModelConfig &config, const boost::multi_array<Expr, 2> &azel);

    BeamCoeff readBeamCoeffFile(const string &filename) const;

    Instrument                      itsInstrument;
    SourceDB                        itsSourceDb;
    PhaseRef::ConstPointer          itsPhaseRef;
    vector<StatUVW::ConstPointer>   itsStationUvw;
    map<baseline_t, JonesExpr>      itsExpressions;
    map<uint, Expr>                 itsParms;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
