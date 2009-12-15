//# Model.h: Measurement equation for the LOFAR telescope and its environment.
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

#ifndef LOFAR_BBSKERNEL_MODEL_H
#define LOFAR_BBSKERNEL_MODEL_H

#include <BBSKernel/ExprSet.h>
#include <BBSKernel/Instrument.h>
#include <BBSKernel/ModelConfig.h>
#include <BBSKernel/ParmManager.h>
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

// \addtogroup BBSKernel
// @{

class Model: public ExprSet<JonesMatrix>
{
public:
    typedef shared_ptr<Model>       Ptr;
    typedef shared_ptr<const Model> ConstPtr;

    Model(const Instrument &instrument, const SourceDB &sourceDb,
        const casa::MDirection &reference, double referenceFreq);

    void clear();

    void makeForwardExpr(const ModelConfig &config, const VisData::Ptr &chunk,
        const vector<baseline_t> &baselines);

    void makeInverseExpr(const ModelConfig &config, const VisData::Ptr &chunk,
        const vector<baseline_t> &baselines);

    virtual unsigned int size() const;
    virtual Box domain() const;

    virtual ParmGroup getParms() const;
    virtual ParmGroup getSolvableParms() const;
    virtual void setSolvableParms(const ParmGroup &solvables);
    virtual void clearSolvableParms();

    virtual void setEvalGrid(const Grid &grid);
    virtual const JonesMatrix evaluate(unsigned int i);

private:
    vector<unsigned int>
        makeUsedStationList(const vector<baseline_t> &baselines) const;

    ExprParm::Ptr makeExprParm(unsigned int category, const string &name);

    vector<Source::Ptr> makeSourceList(const vector<string> &patterns);

    Source::Ptr makeSource(const SourceInfo &source);

    Expr<Scalar>::Ptr makeSpectralIndexExpr(const SourceInfo &source,
        const string &stokesParm);

    void makeStationUVW();

    casa::Matrix<Expr<Vector<2> >::Ptr>
        makeStationShiftExpr(const vector<unsigned int> &stations,
            const vector<Source::Ptr> &sources) const;

    casa::Vector<Expr<JonesMatrix>::Ptr>
        makeBandpassExpr(const vector<unsigned int> &stations);

    casa::Vector<Expr<JonesMatrix>::Ptr>
        makeGainExpr(const ModelConfig &config,
          const vector<unsigned int> &stations);

    casa::Matrix<Expr<JonesMatrix>::Ptr>
        makeDirectionalGainExpr(const ModelConfig &config,
            const vector<unsigned int> &stations,
            const vector<Source::Ptr> &sources);

    casa::Matrix<Expr<Vector<2> >::Ptr>
        makeAzElExpr(const vector<unsigned int> &stations,
            const vector<Source::Ptr> &sources) const;

    casa::Matrix<Expr<JonesMatrix>::Ptr>
        makeBeamExpr(const BeamConfig &config,
            const vector<unsigned int> &stations,
            const casa::Matrix<Expr<Vector<2> >::Ptr> &azel);

//    casa::Matrix<Expr<JonesMatrix>::Ptr>
//        makeDipoleBeamExpr(const ModelConfig &config,
//            const vector<unsigned int> &stations,
//            const casa::Matrix<Expr<Vector<2> >::Ptr> &azel);

    casa::Matrix<Expr<JonesMatrix>::Ptr>
        makeIonosphereExpr(const IonosphereConfig &config,
            const vector<unsigned int> &stations,
            const casa::Matrix<Expr<Vector<2> >::Ptr> &azel);

    Expr<JonesMatrix>::Ptr corrupt(Expr<JonesMatrix>::Ptr &accumulator,
        Expr<JonesMatrix>::Ptr &effect);

    Instrument                              itsInstrument;
    SourceDB                                itsSourceDb;
    casa::MDirection                        itsPhaseReference;
    double                                  itsReferenceFreq;
    Request                                 itsRequest;
    Cache                                   itsCache;

    vector<Expr<JonesMatrix>::Ptr>          itsExpr;
    map<unsigned int, ExprParm::Ptr>        itsParms;
    vector<Expr<Vector<3> >::Ptr>           itsStationUVW;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
