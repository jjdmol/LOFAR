//# MeasurementExprLOFARGen.cc: Helper functions to generate a LOFAR measurement
//# expression from a set of model configuration options.
//#
//# Copyright (C) 2011
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
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/MeasurementExprLOFARGen.h>
#include <BBSKernel/MeasurementExprLOFARUtil.h>
#include <BBSKernel/Expr/ConditionNumber.h>
#include <BBSKernel/Expr/EquatorialCentroid.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/FlagIf.h>
#include <BBSKernel/Expr/LinearToCircularRL.h>
#include <BBSKernel/Expr/Literal.h>
#include <BBSKernel/Expr/MatrixInverse.h>
#include <BBSKernel/Expr/MatrixSum.h>
#include <BBSKernel/Expr/MergeFlags.h>
#include <Common/lofar_algorithm.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_list.h>
#include <Common/lofar_set.h>
#include <Common/lofar_string.h>
#include <Common/StreamUtil.h>

namespace LOFAR
{
namespace BBS
{

namespace
{
using LOFAR::operator<<;

typedef set<unsigned int> SourceGroup;

// The supported direction dependent effects in the order in which they
// appear in the measurement equation.
enum DDE
{
    DIRECTIONAL_GAIN,
    BEAM,
    DIRECTIONAL_TEC,
    FARADAY_ROTATION,
    IONOSPHERE,
    N_DDE
};

class DDEIndex
{
public:
    DDEIndex(unsigned int id);

    unsigned int id() const;

    int &operator[](unsigned int dde);
    const int &operator[](unsigned int dde) const;

private:
    unsigned int    itsId;
    int             itsIndex[N_DDE];
};

class ExprDDE
{
public:
    typedef shared_ptr<ExprDDE>         Ptr;
    typedef shared_ptr<const ExprDDE>   ConstPtr;

    explicit ExprDDE(unsigned int size);
    size_t size() const;
    void setExpr(unsigned int i, const Expr<JonesMatrix>::Ptr &expr);
    Expr<JonesMatrix>::Ptr expr(unsigned int i) const;

private:
    vector<Expr<JonesMatrix>::Ptr>  itsExpr;
};

class ExprDDEStack
{
public:
    unsigned int depth() const;
    void push(const ExprDDE::Ptr &dde);
    void pop();
    ExprDDE::Ptr top();

private:
    vector<ExprDDE::Ptr>    itsStack;
};

ostream &operator<<(ostream &os, const DDEIndex &obj);

// comparison operator...
bool operator<(const DDEIndex &lhs, const DDEIndex &rhs);

ExprDDE::Ptr compose(const ExprDDE::Ptr &lhs, const ExprDDE::Ptr &rhs);

void applyDDE(const ExprDDE::Ptr &dde, const BaselineSeq &baselines, vector<Expr<JonesMatrix>::Ptr> &expr);

void generateExpr(const BaselineSeq &baselines, const vector<Source::Ptr> &sources, const vector<DDEIndex> &index,
    const vector<ExprDDE::Ptr> &dde, const ExprDDE::Ptr &transform, unsigned int level, unsigned int start,
    unsigned int end, const vector<Expr<Vector<3> >::Ptr> &uvw, ExprDDEStack &stack,
    vector<Expr<JonesMatrix>::Ptr> &expr);

ExprDDE::Ptr makeCircularRLTransform(const Instrument::ConstPtr &instrument);

ExprDDE::Ptr makeDirectionalGainExpr(Scope &scope,
    const Instrument::ConstPtr &instrument,
    const string &patch,
    const DirectionalGainConfig &config);

ExprDDE::Ptr makeBeamExpr(Scope &scope,
    const Instrument::ConstPtr &instrument,
    double refFreq,
    const Expr<Vector<2> >::Ptr &exprPosition,
    const Expr<Vector<3> >::Ptr &exprRefDelayITRF,
    const Expr<Vector<3> >::Ptr &exprRefTileITRF,
    const BeamConfig &config);

ExprDDE::Ptr makeDirectionalTECExpr(Scope &scope,
    const Instrument::ConstPtr &instrument, const string &patch);

ExprDDE::Ptr makeFaradayRotationExpr(Scope &scope,
    const Instrument::ConstPtr &instrument, const string &patch);

ExprDDE::Ptr makeIonosphereExpr(Scope &scope,
    const Instrument::ConstPtr &instrument,
    const Expr<Vector<2> >::Ptr &exprPosition,
    const IonosphereExpr::Ptr &exprIonosphere);

void updateIndex(vector<DDEIndex> &index, const SourceGroup &group, unsigned int i, unsigned int j);

string makeName(const vector<Source::Ptr> sources,
    const SourceGroup &group);

Expr<Vector<2> >::Ptr makePositionExpr(const vector<Source::Ptr> sources,
    const SourceGroup &group);


// -----------------------------------------------------------------------------
class ClusterDescriptor
{
public:
    ClusterDescriptor()
    {
        fill(enabled, enabled + N_DDE, false);
    }

    bool                enabled[N_DDE];
    string              label[N_DDE];
};

typedef set<unsigned int>   ClusterID;
typedef map<ClusterID, ClusterDescriptor> ClusterMap;
typedef vector<Source::Ptr> SourceList;

void buildClusterMap(DDE dde, const DDEPartition &partition,
    const SourceList &sources, ClusterMap &map)
{
    vector<bool> assigned(sources.size(), false);
    for(unsigned int i = 0; i < partition.size(); ++i)
    {
        ClusterID groupID;
        for(unsigned int j = 0; j < sources.size(); ++j)
        {
            if(partition.matches(i, sources[j]->name()))
            {
                ASSERTSTR(!assigned[j], "Source already assigned to patch: "
                    << sources[j]->name());
                assigned[j] = true;

//                LOG_DEBUG_STR("pattern: " << i << " matched: " << sources[j]->name());
                if(partition.group(i))
                {
                    groupID.insert(j);
                }
                else
                {
                    ClusterID id;
                    id.insert(j);

                    ClusterDescriptor &descriptor = map[id];
                    descriptor.label[dde] = sources[j]->name();
                    descriptor.enabled[dde] = true;
                }
            }
        }

        if(!groupID.empty())
        {
            ASSERT(partition.group(i));
            ClusterDescriptor &descriptor = map[groupID];
            descriptor.label[dde] = partition.name(i);
            descriptor.enabled[dde] = true;
        }
    }

    if(partition.matchesRemainder())
    {
//        LOG_DEBUG_STR("MATCHING REMAINDER...");
        ClusterID groupID;
        for(unsigned int i = 0; i < sources.size(); ++i)
        {
            if(!assigned[i])
            {
                if(partition.groupRemainder())
                {
                    groupID.insert(i);
                }
                else
                {
                    ClusterID id;
                    id.insert(i);

                    ClusterDescriptor &descriptor = map[id];
                    descriptor.label[dde] = sources[i]->name();
                    descriptor.enabled[dde] = true;
                }
            }
        }

        if(!groupID.empty())
        {
//            LOG_DEBUG_STR("REMAINDER IS GROUPED");
            ASSERT(partition.groupRemainder());
            ClusterDescriptor &descriptor = map[groupID];
            descriptor.label[dde] = partition.remainderGroupName();
            descriptor.enabled[dde] = true;
        }
    }
}

} //# unnamed namespace

vector<Expr<JonesMatrix>::Ptr>
makeMeasurementExpr(Scope &scope, const vector<Source::Ptr> &sources,
    const Instrument::ConstPtr &instrument, const BaselineSeq &baselines,
    const ModelConfig &config, double refFreq, const casa::MDirection &refPhase,
    const casa::MDirection &refDelay, const casa::MDirection &refTile,
    bool circular)
{
    ClusterMap clusters;
    LOG_DEBUG_STR("Building patch map...");
    buildClusterMap(DIRECTIONAL_GAIN,
        config.getDirectionalGainConfig().partition(), sources, clusters);
    buildClusterMap(BEAM, config.getBeamConfig().partition(), sources, clusters);
    buildClusterMap(DIRECTIONAL_TEC, config.getDirectionalTECConfig().partition(),
        sources, clusters);
    buildClusterMap(FARADAY_ROTATION, config.getFaradayRotationConfig().partition(),
        sources, clusters);
    buildClusterMap(IONOSPHERE, config.getIonosphereConfig().partition(), sources,
        clusters);
    LOG_DEBUG_STR("Building patch map... done.");
    LOG_DEBUG_STR("No. of unique patches: " << clusters.size());

    vector<DDEIndex> index;
    for(unsigned int i = 0; i < sources.size(); i++)
    {
        index.push_back(DDEIndex(i));
    }

    // Beam reference position on the sky.
    Expr<Vector<2> >::Ptr exprRefDelay = makeDirectionExpr(refDelay);
    Expr<Vector<3> >::Ptr exprRefDelayITRF =
        makeITRFExpr(instrument->position(), exprRefDelay);

    // Tile beam reference position on the sky.
    Expr<Vector<2> >::Ptr exprRefTile = makeDirectionExpr(refTile);
    Expr<Vector<3> >::Ptr exprRefTileITRF =
        makeITRFExpr(instrument->position(), exprRefTile);

    // Create an UVW expression per station.
    vector<Expr<Vector<3> >::Ptr> exprUVW(instrument->nStations());
    for(size_t i = 0; i < instrument->nStations(); ++i)
    {
        exprUVW[i] = makeStationUVWExpr(instrument->position(),
            instrument->station(i)->position(), refPhase);
    }

    IonosphereExpr::Ptr exprIonosphere;
    if(config.useIonosphere())
    {
        exprIonosphere = IonosphereExpr::create(config.getIonosphereConfig(),
            scope);
    }

    // Direction dependent effects (DDE).
    vector<ExprDDE::Ptr> exprDDE;
    for(ClusterMap::const_iterator it = clusters.begin(), end = clusters.end();
        it != end; ++it)
    {
//        string name = makeName(sources, it->first);
        Expr<Vector<2> >::Ptr exprPosition = makePositionExpr(sources,
            it->first);

//        DDEMask mask = it->second;
        const ClusterDescriptor &descriptor = it->second;

        // Directional gain.
        if(descriptor.enabled[DIRECTIONAL_GAIN])
        {
            exprDDE.push_back(makeDirectionalGainExpr(scope, instrument,
                descriptor.label[DIRECTIONAL_GAIN],
                config.getDirectionalGainConfig()));
            updateIndex(index, it->first, DIRECTIONAL_GAIN, exprDDE.size() - 1);
        }

        // Beam.
        if(descriptor.enabled[BEAM])
        {
            exprDDE.push_back(makeBeamExpr(scope, instrument, refFreq,
                exprPosition, exprRefDelayITRF, exprRefTileITRF,
                config.getBeamConfig()));
            updateIndex(index, it->first, BEAM, exprDDE.size() - 1);
        }

        // Directional TEC.
        if(descriptor.enabled[DIRECTIONAL_TEC])
        {
            exprDDE.push_back(makeDirectionalTECExpr(scope, instrument,
                descriptor.label[DIRECTIONAL_TEC]));
            updateIndex(index, it->first, DIRECTIONAL_TEC, exprDDE.size() - 1);
        }

        // Faraday rotation.
        if(descriptor.enabled[FARADAY_ROTATION])
        {
            exprDDE.push_back(makeFaradayRotationExpr(scope, instrument,
                descriptor.label[FARADAY_ROTATION]));
            updateIndex(index, it->first, FARADAY_ROTATION, exprDDE.size() - 1);
        }

        // Ionosphere.
        if(descriptor.enabled[IONOSPHERE])
        {
            exprDDE.push_back(makeIonosphereExpr(scope, instrument,
                exprPosition, exprIonosphere));
            updateIndex(index, it->first, IONOSPHERE, exprDDE.size() - 1);
        }
    }

    // Sort DDE index.
    sort(index.begin(), index.end());

    LOG_DEBUG_STR("index: " << index);

    // Create a linear to circular-RL transformation Jones matrix.
    ExprDDE::Ptr transform;
    if(circular)
    {
        transform = makeCircularRLTransform(instrument);
    }

    // Generate direction dependent part of the measurement expression.
    vector<Expr<JonesMatrix>::Ptr> expr(baselines.size());

    ExprDDEStack stack;
    generateExpr(baselines, sources, index, exprDDE, transform, 0, 0,
        sources.size(), exprUVW, stack, expr);

    // Direction independent effects (DIE).
    vector<Expr<JonesMatrix>::Ptr> exprDIE(instrument->nStations());
    for(unsigned int i = 0; i < instrument->nStations(); ++i)
    {
        // Create a clock delay expression per station.
        if(config.useClock())
        {
            exprDIE[i] = compose(exprDIE[i],
                makeClockExpr(scope, instrument->station(i),
                config.getClockConfig()));
        }

        // Bandpass.
        if(config.useBandpass())
        {
            exprDIE[i] = compose(exprDIE[i],
                makeBandpassExpr(scope, instrument->station(i)));
        }

        // Create a direction independent gain expression per station.
        if(config.useGain())
        {
            exprDIE[i] = compose(exprDIE[i],
                makeGainExpr(scope, instrument->station(i),
                config.getGainConfig()));
        }

        // Create a direction independent TEC expression per station. Note that
        // TEC is a scalar effect, so it commutes.
        if(config.useTEC())
        {
            exprDIE[i] = compose(exprDIE[i],
                makeTECExpr(scope, instrument->station(i)));
        }
    }

    for(unsigned int i = 0; i < baselines.size(); ++i)
    {
        const baseline_t baseline = baselines[i];
        expr[i] = apply(exprDIE[baseline.first], expr[i],
            exprDIE[baseline.second]);
    }

    return expr;
}

vector<Expr<JonesMatrix>::Ptr>
makeStationExpr(Scope &scope, const casa::MDirection &direction,
    const Instrument::ConstPtr &instrument, const ModelConfig &config,
    double refFreq, const casa::MDirection &refDelay,
    const casa::MDirection &refTile, bool inverse, bool useMMSE,
    double sigmaMMSE)
{
    if(config.useDirectionalGain() || config.useDirectionalTEC()
        || config.useFaradayRotation())
    {
        THROW(BBSKernelException, "Cannot generate an expression for"
            " DirectionalGain, DirectionalTEC, and/or FaradayRotation in an"
            " (unnamed) direction.");
    }

    // Position of interest on the sky.
    Expr<Vector<2> >::Ptr exprDirection = makeDirectionExpr(direction);
    Expr<Vector<3> >::Ptr exprDirectionITRF =
        makeITRFExpr(instrument->position(), exprDirection);

    // Beam reference position on the sky.
    Expr<Vector<2> >::Ptr exprRefDelay = makeDirectionExpr(refDelay);
    Expr<Vector<3> >::Ptr exprRefDelayITRF =
        makeITRFExpr(instrument->position(), exprRefDelay);

    // Tile beam reference position on the sky.
    Expr<Vector<2> >::Ptr exprRefTile = makeDirectionExpr(refTile);
    Expr<Vector<3> >::Ptr exprRefTileITRF =
        makeITRFExpr(instrument->position(), exprRefTile);

    IonosphereExpr::Ptr exprIonosphere;
    if(config.useIonosphere())
    {
        exprIonosphere = IonosphereExpr::create(config.getIonosphereConfig(),
            scope);
    }

    Expr<Scalar>::Ptr exprOne(new Literal(1.0));
    Expr<JonesMatrix>::Ptr exprIdentity(new AsDiagonalMatrix(exprOne, exprOne));

    vector<Expr<JonesMatrix>::Ptr> expr(instrument->nStations());
    for(unsigned int i = 0; i < instrument->nStations(); ++i)
    {
        // Create a clock delay expression per station.
        if(config.useClock())
        {
            expr[i] = compose(expr[i],
                makeClockExpr(scope, instrument->station(i),
                config.getClockConfig()));
        }

        // Bandpass.
        if(config.useBandpass())
        {
            expr[i] = compose(expr[i],
                makeBandpassExpr(scope, instrument->station(i)));
        }

        // Create a direction independent gain expression per station.
        if(config.useGain())
        {
            expr[i] = compose(expr[i],
                makeGainExpr(scope, instrument->station(i),
                config.getGainConfig()));
        }

        // Create a direction independent TEC expression per station.
        if(config.useTEC())
        {
            expr[i] = compose(expr[i],
                makeTECExpr(scope, instrument->station(i)));
        }

        // Beam.
        if(config.useBeam())
        {
            expr[i] = compose(expr[i], makeBeamExpr(scope,
                instrument->station(i), refFreq, exprDirectionITRF,
                exprRefDelayITRF, exprRefTileITRF, config.getBeamConfig()));
        }

        // Ionosphere.
        if(config.useIonosphere())
        {
            expr[i] = compose(expr[i], makeIonosphereExpr(scope,
                instrument->station(i), instrument->position(),
                exprDirectionITRF, exprIonosphere));
        }

        if(expr[i])
        {
            if(config.useFlagger())
            {
                const FlaggerConfig &flagConfig = config.getFlaggerConfig();

                Expr<Scalar>::Ptr exprCond =
                    Expr<Scalar>::Ptr(new ConditionNumber(expr[i]));
                Expr<Scalar>::Ptr exprThreshold(makeFlagIf(exprCond,
                    std::bind2nd(std::greater_equal<double>(),
                    flagConfig.threshold())));

                typedef MergeFlags<JonesMatrix, Scalar> T_MERGEFLAGS;
                expr[i] = T_MERGEFLAGS::Ptr(new T_MERGEFLAGS(expr[i],
                    exprThreshold));
            }

            if(inverse)
            {
                expr[i] = Expr<JonesMatrix>::Ptr(new MatrixInverse(expr[i]));
            }
        }
        else
        {
            expr[i] = exprIdentity;
        }
    }

    return expr;
}

vector<Expr<JonesMatrix>::Ptr>
makeStationExpr(Scope &scope, const vector<Source::Ptr> &sources,
    const Instrument::ConstPtr &instrument, const ModelConfig &config,
    double refFreq, const casa::MDirection &refDelay,
    const casa::MDirection &refTile, bool inverse, bool useMMSE,
    double sigmaMMSE)
{
    ClusterMap clusters;
    LOG_DEBUG_STR("Building patch map...");
    buildClusterMap(DIRECTIONAL_GAIN,
        config.getDirectionalGainConfig().partition(), sources, clusters);
    buildClusterMap(BEAM, config.getBeamConfig().partition(), sources, clusters);
    buildClusterMap(DIRECTIONAL_TEC, config.getDirectionalTECConfig().partition(),
        sources, clusters);
    buildClusterMap(FARADAY_ROTATION, config.getFaradayRotationConfig().partition(),
        sources, clusters);
    buildClusterMap(IONOSPHERE, config.getIonosphereConfig().partition(), sources,
        clusters);
    LOG_DEBUG_STR("Building patch map... done.");
    LOG_DEBUG_STR("No. of unique patches: " << clusters.size());

    // Beam reference position on the sky.
    Expr<Vector<2> >::Ptr exprRefDelay = makeDirectionExpr(refDelay);
    Expr<Vector<3> >::Ptr exprRefDelayITRF =
        makeITRFExpr(instrument->position(), exprRefDelay);

    // Tile beam reference position on the sky.
    Expr<Vector<2> >::Ptr exprRefTile = makeDirectionExpr(refTile);
    Expr<Vector<3> >::Ptr exprRefTileITRF =
        makeITRFExpr(instrument->position(), exprRefTile);

    IonosphereExpr::Ptr exprIonosphere;
    if(config.useIonosphere())
    {
        exprIonosphere = IonosphereExpr::create(config.getIonosphereConfig(),
            scope);
    }

    vector<ExprDDE::Ptr> exprDDE(N_DDE);
    for(ClusterMap::const_iterator it = clusters.begin(), end = clusters.end();
        it != end; ++it)
    {
//        string name = makeName(sources, it->first);
        Expr<Vector<2> >::Ptr exprPosition = makePositionExpr(sources,
            it->first);

//        DDEMask mask = it->second;
        const ClusterDescriptor &descriptor = it->second;

        // Directional gain.
        if(descriptor.enabled[DIRECTIONAL_GAIN])
        {
            ASSERT(!exprDDE[DIRECTIONAL_GAIN]);
            exprDDE[DIRECTIONAL_GAIN] = makeDirectionalGainExpr(scope,
                instrument, descriptor.label[DIRECTIONAL_GAIN],
                config.getDirectionalGainConfig());
        }

        // Beam.
        if(descriptor.enabled[BEAM])
        {
            ASSERT(!exprDDE[BEAM]);
            exprDDE[BEAM] = makeBeamExpr(scope, instrument, refFreq,
                exprPosition, exprRefDelayITRF, exprRefTileITRF,
                config.getBeamConfig());
        }

        // Directional TEC.
        if(descriptor.enabled[DIRECTIONAL_TEC])
        {
            ASSERT(!exprDDE[DIRECTIONAL_TEC]);
            exprDDE[DIRECTIONAL_TEC] = makeDirectionalTECExpr(scope, instrument,
                descriptor.label[DIRECTIONAL_TEC]);
        }

        // Faraday rotation.
        if(descriptor.enabled[FARADAY_ROTATION])
        {
            ASSERT(!exprDDE[FARADAY_ROTATION]);
            exprDDE[FARADAY_ROTATION] = makeFaradayRotationExpr(scope,
                instrument, descriptor.label[FARADAY_ROTATION]);
        }

        // Ionosphere.
        if(descriptor.enabled[IONOSPHERE])
        {
            ASSERT(!exprDDE[IONOSPHERE]);
            exprDDE[IONOSPHERE] = makeIonosphereExpr(scope, instrument,
                exprPosition, exprIonosphere);
        }
    }

    ExprDDE::Ptr exprDDEComposition;
    for(unsigned int i = 0; i < N_DDE; ++i)
    {
        if(exprDDE[i])
        {
            exprDDEComposition = compose(exprDDEComposition, exprDDE[i]);
        }
    }

    // Direction independent effects (DIE).
    Expr<Scalar>::Ptr exprOne(new Literal(1.0));
    Expr<JonesMatrix>::Ptr exprIdentity(new AsDiagonalMatrix(exprOne, exprOne));

    vector<Expr<JonesMatrix>::Ptr> expr(instrument->nStations());
    for(unsigned int i = 0; i < instrument->nStations(); ++i)
    {
        // Create a clock delay expression per station.
        if(config.useClock())
        {
            expr[i] = compose(expr[i],
                makeClockExpr(scope, instrument->station(i),
                config.getClockConfig()));
        }

        // Bandpass.
        if(config.useBandpass())
        {
            expr[i] = compose(expr[i],
                makeBandpassExpr(scope, instrument->station(i)));
        }

        // Create a direction independent gain expression per station.
        if(config.useGain())
        {
            expr[i] = compose(expr[i],
                makeGainExpr(scope, instrument->station(i),
                config.getGainConfig()));
        }

        // Create a direction independent TEC expression per station.
        if(config.useTEC())
        {
            expr[i] = compose(expr[i],
                makeTECExpr(scope, instrument->station(i)));
        }

        // Right multiply by the composition of direction dependent effects.
        if(exprDDEComposition)
        {
            expr[i] = compose(expr[i], exprDDEComposition->expr(i));
        }

        if(expr[i])
        {
            if(config.useFlagger())
            {
                const FlaggerConfig &flagConfig = config.getFlaggerConfig();

                Expr<Scalar>::Ptr exprCond =
                    Expr<Scalar>::Ptr(new ConditionNumber(expr[i]));
                Expr<Scalar>::Ptr exprThreshold(makeFlagIf(exprCond,
                    std::bind2nd(std::greater_equal<double>(),
                    flagConfig.threshold())));

                typedef MergeFlags<JonesMatrix, Scalar> T_MERGEFLAGS;
                expr[i] = T_MERGEFLAGS::Ptr(new T_MERGEFLAGS(expr[i],
                    exprThreshold));
            }

            if(inverse)
            {
                expr[i] = Expr<JonesMatrix>::Ptr(new MatrixInverse(expr[i]));
            }
        }
        else
        {
            expr[i] = exprIdentity;
        }
    }

    return expr;
}

namespace
{

DDEIndex::DDEIndex(unsigned int id)
    :   itsId(id)
{
    fill(itsIndex, itsIndex + N_DDE, -1);
}

unsigned int DDEIndex::id() const
{
    return itsId;
}

int &DDEIndex::operator[](unsigned int dde)
{
    return itsIndex[dde];
}

const int &DDEIndex::operator[](unsigned int dde) const
{
    return itsIndex[dde];
}

// comparison operator...
bool operator<(const DDEIndex &lhs, const DDEIndex &rhs)
{
    for(unsigned int i = 0; i < N_DDE; ++i)
    {
        if(lhs[i] != rhs[i])
        {
            return lhs[i] < rhs[i];
        }
    }

    return false;
}

ostream &operator<<(ostream &os, const DDEIndex &obj)
{
    os << "[" << obj.id() << ":";
    for(unsigned int i = 0; i < N_DDE; ++i)
    {
        os << " ";
        os << obj[i];
    }
    os << "]";

    return os;
}

ExprDDE::ExprDDE(unsigned int size)
    :   itsExpr(size)
{
}

size_t ExprDDE::size() const
{
    return itsExpr.size();
}

void ExprDDE::setExpr(unsigned int i, const Expr<JonesMatrix>::Ptr &expr)
{
    itsExpr[i] = expr;
}

Expr<JonesMatrix>::Ptr ExprDDE::expr(unsigned int i) const
{
    return itsExpr[i];
}

ExprDDE::Ptr compose(const ExprDDE::Ptr &lhs, const ExprDDE::Ptr &rhs)
{
    ASSERT(rhs);
    if(lhs)
    {
        unsigned int size = std::min(lhs->size(), rhs->size());

        ExprDDE::Ptr composition(new ExprDDE(size));
        for(unsigned int i = 0; i < size; ++i)
        {
            composition->setExpr(i, compose(lhs->expr(i), rhs->expr(i)));
        }

        return composition;
    }

    return rhs;
}

unsigned int ExprDDEStack::depth() const
{
    return itsStack.size();
}

void ExprDDEStack::push(const ExprDDE::Ptr &dde)
{
    if(depth() == 0)
    {
        itsStack.push_back(dde);
        return;
    }

    itsStack.push_back(compose(top(), dde));
}

void ExprDDEStack::pop()
{
    itsStack.pop_back();
}

ExprDDE::Ptr ExprDDEStack::top()
{
    return itsStack.back();
}

void applyDDE(const ExprDDE::Ptr &dde, const BaselineSeq &baselines, vector<Expr<JonesMatrix>::Ptr> &expr)
{
    for(unsigned int i = 0; i < baselines.size(); ++i)
    {
        expr[i] = apply(dde->expr(baselines[i].first), expr[i],
            dde->expr(baselines[i].second));
    }
}

void generateExpr(const BaselineSeq &baselines, const vector<Source::Ptr> &sources, const vector<DDEIndex> &index,
    const vector<ExprDDE::Ptr> &dde, const ExprDDE::Ptr &transform, unsigned int level, unsigned int start,
    unsigned int end, const vector<Expr<Vector<3> >::Ptr> &uvw, ExprDDEStack &stack,
    vector<Expr<JonesMatrix>::Ptr> &expr)
{
    if(level == N_DDE)
    {
//        cout << "generate coherencies for sources: [";
//        for(unsigned int i = start; i < end; ++i)
//        {
//            cout << " " << index[i].id() << ":"
//                << sources[index[i].id()]->name();
//        }
//        cout << " ]" << endl;

        if(end - start == 1)
        {
            Source::Ptr source = sources[index[start].id()];
            for(unsigned int i = 0; i < baselines.size(); ++i)
            {
                expr[i] = source->coherence(baselines[i],
                    uvw[baselines[i].first], uvw[baselines[i].second]);
            }
        }
        else
        {
            for(unsigned int i = 0; i < baselines.size(); ++i)
            {
                MatrixSum::Ptr sum(new MatrixSum());
                for(unsigned int j = start; j < end; ++j)
                {
                    Source::Ptr source = sources[index[j].id()];
                    sum->connect(source->coherence(baselines[i],
                        uvw[baselines[i].first], uvw[baselines[i].second]));
                }

                expr[i] = sum;
            }
        }

        if(transform)
        {
            stack.push(transform);
        }

        // Apply DDE.
        if(stack.depth() > 0)
        {
            applyDDE(stack.top(), baselines, expr);
        }
    }
    else if(index[start][level] == index[end - 1][level])
    {
//        cout << "level: " << level << " no sum, continue..." << endl;

        if(index[start][level] >= 0)
        {
            stack.push(dde[index[start][level]]);
        }

        generateExpr(baselines, sources, index, dde, transform, level + 1, start, end, uvw, stack, expr);

        if(index[start][level] >= 0)
        {
            stack.pop();
        }
    }
    else
    {
        for(unsigned int i = 0; i < baselines.size(); ++i)
        {
            expr[i] = MatrixSum::Ptr(new MatrixSum());
        }

        ExprDDEStack childStack;

        unsigned int pivot = start;
        while(start != end)
        {
            while(pivot < end && index[pivot][level] == index[start][level])
            {
                ++pivot;
            }

//            cout << "level: " << level << " generate for subrange [" << start << "," << pivot << "]" << endl;

            if(index[start][level] >= 0)
            {
                childStack.push(dde[index[start][level]]);
            }

            vector<Expr<JonesMatrix>::Ptr> childExpr(baselines.size());
            generateExpr(baselines, sources, index, dde, transform, level + 1, start, pivot, uvw, childStack,
                childExpr);

            if(index[start][level] >= 0)
            {
                childStack.pop();
            }

            for(unsigned int i = 0; i < baselines.size(); ++i)
            {
                static_pointer_cast<MatrixSum>(expr[i])->connect(childExpr[i]);
            }

            start = pivot;
        }

        if(stack.depth() > 0)
        {
            applyDDE(stack.top(), baselines, expr);
        }
    }
}

ExprDDE::Ptr makeCircularRLTransform(const Instrument::ConstPtr &instrument)
{
    Expr<JonesMatrix>::Ptr H(new LinearToCircularRL());

    ExprDDE::Ptr exprDDE(new ExprDDE(instrument->nStations()));
    for(unsigned int i = 0; i < instrument->nStations(); ++i)
    {
        exprDDE->setExpr(i, H);
    }

    return exprDDE;
}

ExprDDE::Ptr makeDirectionalGainExpr(Scope &scope,
    const Instrument::ConstPtr &instrument,
    const string &patch,
    const DirectionalGainConfig &config)
{
    ExprDDE::Ptr exprDDE(new ExprDDE(instrument->nStations()));
    for(unsigned int i = 0; i < instrument->nStations(); ++i)
    {
        exprDDE->setExpr(i, makeDirectionalGainExpr(scope,
            instrument->station(i), patch, config));
    }

    return exprDDE;
}

ExprDDE::Ptr makeBeamExpr(Scope &scope, const Instrument::ConstPtr &instrument,
    double refFreq,
    const Expr<Vector<2> >::Ptr &exprPosition,
    const Expr<Vector<3> >::Ptr &exprRefDelayITRF,
    const Expr<Vector<3> >::Ptr &exprRefTileITRF,
    const BeamConfig &config)
{
    Expr<Vector<3> >::Ptr exprPositionITRF =
        makeITRFExpr(instrument->position(), exprPosition);

    ExprDDE::Ptr exprDDE(new ExprDDE(instrument->nStations()));
    for(unsigned int i = 0; i < instrument->nStations(); ++i)
    {
        exprDDE->setExpr(i, makeBeamExpr(scope, instrument->station(i), refFreq,
                exprPositionITRF, exprRefDelayITRF, exprRefTileITRF, config));
    }

    return exprDDE;
}

ExprDDE::Ptr makeDirectionalTECExpr(Scope &scope,
    const Instrument::ConstPtr &instrument, const string &patch)
{
    ExprDDE::Ptr exprDDE(new ExprDDE(instrument->nStations()));
    for(unsigned int i = 0; i < instrument->nStations(); ++i)
    {
        exprDDE->setExpr(i, makeDirectionalTECExpr(scope,
            instrument->station(i), patch));
    }

    return exprDDE;
}

ExprDDE::Ptr makeFaradayRotationExpr(Scope &scope,
    const Instrument::ConstPtr &instrument, const string &patch)
{
    ExprDDE::Ptr exprDDE(new ExprDDE(instrument->nStations()));
    for(unsigned int i = 0; i < instrument->nStations(); ++i)
    {
        exprDDE->setExpr(i, makeFaradayRotationExpr(scope,
            instrument->station(i), patch));
    }

    return exprDDE;
}

ExprDDE::Ptr makeIonosphereExpr(Scope &scope,
    const Instrument::ConstPtr &instrument,
    const Expr<Vector<2> >::Ptr &exprPosition,
    const IonosphereExpr::Ptr &exprIonosphere)
{
    ExprDDE::Ptr exprDDE(new ExprDDE(instrument->nStations()));
    for(unsigned int i = 0; i < instrument->nStations(); ++i)
    {
        Expr<Vector<3> >::Ptr exprPositionITRF =
            makeITRFExpr(instrument->position(), exprPosition);

        exprDDE->setExpr(i, makeIonosphereExpr(scope, instrument->station(i),
            instrument->position(), exprPositionITRF, exprIonosphere));
    }

    return exprDDE;
}

void updateIndex(vector<DDEIndex> &index, const SourceGroup &group, unsigned int i, unsigned int j)
{
    for(SourceGroup::const_iterator it = group.begin(), end = group.end();
        it != end; ++it)
    {
        index[*it][i] = j;
    }
}

string makeName(const vector<Source::Ptr> sources,
    const SourceGroup &group)
{
    ASSERT(group.size() > 0);

    if(group.size() == 1)
    {
        return sources[*(group.begin())]->name();
    }

    return "Patch-" + sources[*(group.begin())]->name();
}

Expr<Vector<2> >::Ptr makePositionExpr(const vector<Source::Ptr> sources,
    const SourceGroup &group)
{
    ASSERT(group.size() > 0);

    if(group.size() == 1)
    {
        return sources[*(group.begin())]->position();
    }

    EquatorialCentroid::Ptr centroid(new EquatorialCentroid());
    for(SourceGroup::const_iterator it = group.begin(), end = group.end();
        it != end; ++it)
    {
        centroid->connect(sources[*it]->position());
    }

    return centroid;
}

} //# unnamed namespace

} //# namespace BBS
} //# namespace LOFAR
