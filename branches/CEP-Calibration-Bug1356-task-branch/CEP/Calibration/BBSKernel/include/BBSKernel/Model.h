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

#include <BBSKernel/ParmManager.h>
#include <BBSKernel/Instrument.h>
#include <BBSKernel/VisData.h>

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/ExprParm.h>
#include <BBSKernel/Expr/Source.h>

#include <ParmDB/ParmDB.h>
#include <ParmDB/SourceDB.h>

#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>

#include <casa/Arrays.h>
#include <measures/Measures/MDirection.h>

namespace LOFAR
{
namespace BBS
{
class ModelConfig;
class Cache;

// \addtogroup BBSKernel
// @{

class Model
{
public:
    typedef shared_ptr<Model>       Ptr;
    typedef shared_ptr<const Model> ConstPtr;

    Model(const Instrument &instrument, const SourceDB &sourceDb,
        const casa::MDirection &reference);

    void clear();

    void makeForwardExpr(const ModelConfig &config, const VisData::Ptr &chunk,
        const vector<baseline_t> &baselines);

    void makeInverseExpr(const ModelConfig &config, const VisData::Ptr &chunk,
        const vector<baseline_t> &baselines);

    void setRequestGrid(const Grid &grid);
    const JonesMatrix evaluate(const baseline_t &baseline);

    ParmGroup getParms() const;

    ParmGroup getSolvableParms() const;
    void setSolvableParms(const ParmGroup &solvables);
    void clearSolvableParms();

private:
    vector<unsigned int>
        makeUsedStationList(const vector<baseline_t> &baselines) const;

    ExprParm::Ptr makeExprParm(uint category, const string &name);

    vector<Source::Ptr> makeSourceList(const vector<string> &patterns);
    Source::Ptr makeSource(const SourceInfo &source);
    Expr<Scalar>::Ptr makeSpectralIndexExpr(const SourceInfo &source);

    casa::Vector<Expr<Vector<3> >::Ptr>
        makeStationUVWExpr(const vector<unsigned int> &stations) const;

    casa::Matrix<Expr<Vector<2> >::Ptr>
        makeStationShiftExpr(const casa::Vector<Expr<Vector<3> >::Ptr> &uvw,
            const vector<Source::Ptr> &sources) const;

    casa::Vector<Expr<JonesMatrix>::Ptr>
        makeBandpassExpr(const vector<unsigned int> &stations);

    casa::Vector<Expr<JonesMatrix>::Ptr>
        makeIsotropicGainExpr(const ModelConfig &config,
            const vector<unsigned int> &stations);

    casa::Matrix<Expr<JonesMatrix>::Ptr>
        makeAnisotropicGainExpr(const ModelConfig &config,
            const vector<unsigned int> &stations,
            const vector<Source::Ptr> &sources);

    casa::Matrix<Expr<Vector<2> >::Ptr>
        makeAzElExpr(const vector<unsigned int> &stations,
            const vector<Source::Ptr> &sources) const;

    casa::Matrix<Expr<JonesMatrix>::Ptr>
        makeDipoleBeamExpr(const ModelConfig &config,
            const vector<unsigned int> &stations,
            const casa::Matrix<Expr<Vector<2> >::Ptr> &azel);

    casa::Matrix<Expr<JonesMatrix>::Ptr>
        makeIonosphereNodes(const ModelConfig &config,
            const vector<unsigned int> &stations,
            const casa::Matrix<Expr<Vector<2> >::Ptr> &azel);

    Expr<JonesMatrix>::Ptr corrupt(Expr<JonesMatrix>::Ptr &accumulator,
        Expr<JonesMatrix>::Ptr &effect);

    Instrument          itsInstrument;
    SourceDB            itsSourceDb;
    casa::MDirection    itsPhaseReference;
    Request             itsRequest;
    Cache               itsCache;

    map<baseline_t, Expr<JonesMatrix>::Ptr> itsExpr;
    map<unsigned int, ExprParm::Ptr>        itsParms;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
