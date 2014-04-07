//# StationExprLOFAR.cc: Expression for the response (Jones matrix) of a set of
//# LOFAR stations.
//#
//# Copyright (C) 2010
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

#include <lofar_config.h>
#include <BBSKernel/StationExprLOFAR.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/MeasurementExprLOFARUtil.h>
#include <BBSKernel/Expr/AzEl.h>
#include <BBSKernel/Expr/CachePolicy.h>
#include <BBSKernel/Expr/Delay.h>
#include <BBSKernel/Expr/EquatorialCentroid.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/FaradayRotation.h>
#include <BBSKernel/Expr/ITRFDirection.h>
#include <BBSKernel/Expr/Literal.h>
#include <BBSKernel/Expr/MatrixInverse.h>
#include <BBSKernel/Expr/MatrixInverseMMSE.h>
#include <BBSKernel/Expr/MatrixMul2.h>
#include <BBSKernel/Expr/ScalarMatrixMul.h>
#include <BBSKernel/Expr/StationBeamFormer.h>
#include <BBSKernel/Expr/TileArrayFactor.h>

namespace LOFAR
{
namespace BBS
{

StationExprLOFAR::StationExprLOFAR(SourceDB &sourceDB, const BufferMap &buffers,
    const ModelConfig &config, const Instrument::ConstPtr &instrument,
    double refFreq, const casa::MDirection &refPhase,
    const casa::MDirection &refDelay, const casa::MDirection &refTile,
    bool inverse, bool useMMSE, double sigmaMMSE)
{
    initialize(sourceDB, buffers, config, instrument, refFreq, refPhase,
        refDelay, refTile, inverse, useMMSE, sigmaMMSE);
}

StationExprLOFAR::StationExprLOFAR(SourceDB &sourceDB, const BufferMap &buffers,
    const ModelConfig &config, const VisBuffer::Ptr &buffer, bool inverse,
    bool useMMSE, double sigmaMMSE)
{
    initialize(sourceDB, buffers, config, buffer->instrument(),
        buffer->getReferenceFreq(), buffer->getPhaseReference(),
        buffer->getDelayReference(), buffer->getTileReference(), inverse,
        useMMSE, sigmaMMSE);
}

void StationExprLOFAR::initialize(SourceDB &sourceDB, const BufferMap &buffers,
    const ModelConfig &config, const Instrument::ConstPtr &instrument,
    double refFreq, const casa::MDirection &refPhase,
    const casa::MDirection &refDelay, const casa::MDirection &refTile,
    bool inverse, bool useMMSE, double sigmaMMSE)
{
    // Allocate space for the station response expressions.
    itsExpr.resize(instrument->nStations());

    // Direction independent effects (DIE).
    for(size_t i = 0; i < itsExpr.size(); ++i)
    {
        // Create a clock delay expression per station.
        if(config.useClock())
        {
            itsExpr[i] = compose(itsExpr[i],
                makeClockExpr(itsScope, instrument->station(i)));
        }

        // Bandpass.
        if(config.useBandpass())
        {
            itsExpr[i] = compose(itsExpr[i],
                makeBandpassExpr(itsScope, instrument->station(i)));
        }

        // Create a direction independent gain expression per station.
        if(config.useGain())
        {
            itsExpr[i] = compose(itsExpr[i],
                makeGainExpr(itsScope, instrument->station(i),
                config.usePhasors()));
        }

        // Create a direction independent TEC expression per station.
        if(config.useTEC())
        {
            itsExpr[i] = compose(itsExpr[i],
                makeTECExpr(itsScope, instrument->station(i)));
        }
    }

    // Direction dependent effects (DDE).
    if(config.useDirectionalGain() || config.useBeam()
        || config.useDirectionalTEC() || config.useFaradayRotation()
        || config.useRotation() || config.useScalarPhase()
        || config.useIonosphere())
    {
        // Position of interest on the sky (given as patch name).
        if(config.sources().size() > 1)
        {
            THROW(BBSKernelException, "Multiple patches selected, yet a"
                " correction can only be applied for a single direction on the"
                " sky.");
        }

        // Beam reference position on the sky.
        Expr<Vector<2> >::Ptr exprRefDelay = makeDirectionExpr(refDelay);
        Expr<Vector<3> >::Ptr exprRefDelayITRF =
            makeITRFExpr(instrument->position(), exprRefDelay);

        // Tile beam reference position on the sky.
        Expr<Vector<2> >::Ptr exprRefTile = makeDirectionExpr(refTile);
        Expr<Vector<3> >::Ptr exprRefTileITRF =
            makeITRFExpr(instrument->position(), exprRefTile);

        // Functor for the creation of the ionosphere sub-expression.
        IonosphereExpr::Ptr exprIonosphere;
        if(config.useIonosphere())
        {
            exprIonosphere =
                IonosphereExpr::create(config.getIonosphereConfig(), itsScope);
        }

        if(config.sources().empty())
        {
            LOG_DEBUG_STR("Applying a correction for the phase reference of the"
                " observation.");

            if(config.useDirectionalGain() || config.useDirectionalTEC()
                || config.useFaradayRotation())
            {
                THROW(BBSKernelException, "Cannot correct for DirectionalGain,"
                    " DirectionalTEC, and/or FaradayRotation when correcting"
                    " for the (unnamed) phase reference direction.");
            }

            // Phase reference position on the sky.
            Expr<Vector<2> >::Ptr exprRefPhase = makeDirectionExpr(refPhase);
            Expr<Vector<3> >::Ptr exprRefPhaseITRF =
                makeITRFExpr(instrument->position(), exprRefPhase);

            for(size_t i = 0; i < itsExpr.size(); ++i)
            {
                // Beam.
                if(config.useBeam())
                {
                    itsExpr[i] = compose(itsExpr[i],
                        makeBeamExpr(instrument->station(i), refFreq,
                        exprRefPhaseITRF, exprRefDelayITRF, exprRefTileITRF,
                        config.getBeamConfig()));
                }

                // Ionosphere.
                if(config.useIonosphere())
                {
                    // Create an AZ, EL expression for the phase reference
                    // direction.
                    Expr<Vector<2> >::Ptr exprAzEl =
                        makeAzElExpr(instrument->station(i)->position(),
                        exprRefPhase);

                    itsExpr[i] = compose(itsExpr[i],
                        makeIonosphereExpr(instrument->station(i),
                        instrument->position(), exprAzEl, exprIonosphere));
                }
            }
        }
        else
        {
            const string &patch = config.sources().front();
            LOG_DEBUG_STR("Applying a correction for the centroid of patch: "
                << patch);

            PatchExprBase::Ptr exprPatch = makePatchExpr(patch, refPhase,
                sourceDB, buffers);

            // Patch position (ITRF direction vector).
            Expr<Vector<3> >::Ptr exprPatchPositionITRF =
                makeITRFExpr(instrument->position(), exprPatch->position());

            for(size_t i = 0; i < itsExpr.size(); ++i)
            {
                // Directional gain.
                if(config.useDirectionalGain())
                {
                    itsExpr[i] = compose(itsExpr[i],
                        makeDirectionalGainExpr(itsScope,
                        instrument->station(i), patch, config.usePhasors()));
                }

                // Beam.
                if(config.useBeam())
                {
                    itsExpr[i] = compose(itsExpr[i],
                        makeBeamExpr(instrument->station(i), refFreq,
                        exprPatchPositionITRF, exprRefDelayITRF,
                        exprRefTileITRF, config.getBeamConfig()));
                }

                // Directional TEC.
                if(config.useDirectionalTEC())
                {
                    itsExpr[i] = compose(itsExpr[i],
                        makeDirectionalTECExpr(itsScope, instrument->station(i),
                        patch));
                }

                // Faraday rotation.
                if(config.useFaradayRotation())
                {
                    itsExpr[i] = compose(itsExpr[i],
                        makeFaradayRotationExpr(itsScope,
                        instrument->station(i), patch));
                }

                // Polarization rotation.
                if(config.useRotation())
                {
                    itsExpr[i] = compose(itsExpr[i],
                        makeRotationExpr(itsScope, instrument->station(i),
                        patch));
                }

                // Scalar phase.
                if(config.useScalarPhase())
                {
                    itsExpr[i] = compose(itsExpr[i],
                        makeScalarPhaseExpr(itsScope, instrument->station(i),
                        patch));
                }

                // Ionosphere.
                if(config.useIonosphere())
                {
                    // Create an AZ, EL expression for the centroid direction of
                    // the patch.
                    Expr<Vector<2> >::Ptr exprAzEl =
                        makeAzElExpr(instrument->station(i)->position(),
                        exprPatch->position());

                    itsExpr[i] = compose(itsExpr[i],
                        makeIonosphereExpr(instrument->station(i),
                        instrument->position(), exprAzEl, exprIonosphere));
                }
            }
        }
    }

    Expr<Scalar>::Ptr exprOne(new Literal(1.0));
    Expr<JonesMatrix>::Ptr exprIdentity(new AsDiagonalMatrix(exprOne, exprOne));
    for(size_t i = 0; i < itsExpr.size(); ++i)
    {
        if(!itsExpr[i])
        {
            itsExpr[i] = exprIdentity;
        }

        if(inverse)
        {
            if(useMMSE && sigmaMMSE > 0.0)
            {
                itsExpr[i] =
                    Expr<JonesMatrix>::Ptr(new MatrixInverseMMSE(itsExpr[i],
                    sigmaMMSE));
            }
            else
            {
                itsExpr[i] =
                    Expr<JonesMatrix>::Ptr(new MatrixInverse(itsExpr[i]));
            }
        }
    }

    // Set caching policy.
    itsCachePolicy = CachePolicy::Ptr(new DefaultCachePolicy());
    if(config.useCache())
    {
        itsCachePolicy = CachePolicy::Ptr(new ExperimentalCachePolicy());
    }

    itsCachePolicy->apply(itsExpr.begin(), itsExpr.end());
}

size_t StationExprLOFAR::size() const
{
    return itsExpr.size();
}

Box StationExprLOFAR::domain() const
{
    return ParmManager::instance().domain();
}

ParmGroup StationExprLOFAR::parms() const
{
    ParmGroup result;

    for(Scope::const_iterator parm_it = itsScope.begin(),
        parm_end = itsScope.end(); parm_it != parm_end; ++parm_it)
    {
        result.insert(parm_it->first);
    }

    return result;
}

ParmGroup StationExprLOFAR::solvables() const
{
    ParmGroup result;

    for(Scope::const_iterator parm_it = itsScope.begin(),
        parm_end = itsScope.end(); parm_it != parm_end; ++parm_it)
    {
        if(parm_it->second->getPValueFlag())
        {
            result.insert(parm_it->first);
        }
    }

    return result;
}

size_t StationExprLOFAR::nParms() const
{
    return itsScope.size();
}

void StationExprLOFAR::setSolvables(const ParmGroup &solvables)
{
    // Clear the flag that controls whether or not partial derivatives are
    // computed for all parameters.
    for(Scope::const_iterator parm_it = itsScope.begin(),
        parm_end = itsScope.end(); parm_it != parm_end; ++parm_it)
    {
        parm_it->second->clearPValueFlag();
    }

    // Make sure a partial derivative is computed for each solvable that is
    // part of this expression (i.e. that can be found in itsScope).
    for(ParmGroup::const_iterator sol_it = solvables.begin(),
        sol_end = solvables.end(); sol_it != sol_end; ++sol_it)
    {
        Scope::iterator parm_it = itsScope.find(*sol_it);
        if(parm_it != itsScope.end())
        {
            parm_it->second->setPValueFlag();
        }
    }

    // Clear any cached results and reinitialize the caching policy.
    itsCache.clear();
    itsCache.clearStats();
    itsCachePolicy->apply(itsExpr.begin(), itsExpr.end());
}

void StationExprLOFAR::clearSolvables()
{
    // Clear the flag that controls whether or not partial derivatives are
    // computed for all parameters.
    for(Scope::const_iterator parm_it = itsScope.begin(),
        parm_end = itsScope.end(); parm_it != parm_end; ++parm_it)
    {
        parm_it->second->clearPValueFlag();
    }

    // Clear any cached results and reinitialize the caching policy.
    itsCache.clear();
    itsCache.clearStats();
    itsCachePolicy->apply(itsExpr.begin(), itsExpr.end());
}

void StationExprLOFAR::solvablesChanged()
{
    itsCache.clear(Cache::VOLATILE);
    itsCache.clearStats();
}

void StationExprLOFAR::setEvalGrid(const Grid &grid)
{
    itsRequest = Request(grid);

    itsCache.clear();
    itsCache.clearStats();

    // TODO: Set cache size in number of Matrix instances... ?
}

const JonesMatrix StationExprLOFAR::evaluate(unsigned int i)
{
    JonesMatrix result;

    // Evaluate the model.
    ASSERT(i < itsExpr.size());
    const JonesMatrix model = itsExpr[i]->evaluate(itsRequest, itsCache, 0);

    // Pass-through the flags.
    result.setFlags(model.flags());

    // Pass-through the model value.
    const JonesMatrix::View value(model.view());
    result.assign(value);

    // Compute (approximate) partial derivatives using forward differences.
    PValueKey key;
    JonesMatrix::Iterator it(model);
    while(!it.atEnd())
    {
        key = it.key();

        // Get the perturbed value associated with the current (parameter,
        // coefficient) pair.
        const JonesMatrix::View pert(it.value(key));

        // Get perturbation.
        ParmProxy::ConstPtr parm = ParmManager::instance().get(key.parmId);
        const double inversePert = 1.0 / parm->getPerturbation(key.coeffId);

        // Approximate partial derivative using forward differences.
        JonesMatrix::View partial;
        partial.assign(0, 0, (pert(0, 0) - value(0, 0)) * inversePert);
        partial.assign(0, 1, (pert(0, 1) - value(0, 1)) * inversePert);
        partial.assign(1, 0, (pert(1, 0) - value(1, 0)) * inversePert);
        partial.assign(1, 1, (pert(1, 1) - value(1, 1)) * inversePert);

        result.assign(key, partial);
        it.advance(key);
    }

    return result;
}

PatchExprBase::Ptr StationExprLOFAR::makePatchExpr(const string &name,
    const casa::MDirection &refPhase,
    SourceDB &sourceDB,
    const BufferMap &buffers)
{
    if(!name.empty() && name[0] == '@')
    {
        BufferMap::const_iterator it = buffers.find(name);
        ASSERT(it != buffers.end());

        return PatchExprBase::Ptr(new StoredPatchExpr(name.substr(1),
            it->second));
    }

    return PatchExprBase::Ptr(new PatchExpr(itsScope, sourceDB, name,
        refPhase));
}

} //# namespace BBS
} //# namespace LOFAR
