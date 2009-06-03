//# NoiseStep.cc: Options for noise operation.
//#
//# Copyright (C) 2008
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

#include <lofar_config.h>
#include <BBSControl/NoiseStep.h>
#include <BBSControl/CommandVisitor.h>
#include <BBSControl/StreamUtil.h>
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>

namespace LOFAR
{
namespace BBS 
{
using LOFAR::operator<<;

NoiseStep::NoiseStep(const Step* parent)
    : SingleStep(parent)
{
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
}    

NoiseStep::NoiseStep(const string& name,
    const ParameterSet& parset,
    const Step* parent)
    : SingleStep(name, parent)
{
    LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

    // Get the relevant parameters from the Parameter Set \a parset. 
    read(parset.makeSubset("Step." + name + "."));
}

NoiseStep::~NoiseStep()
{
    LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
}

CommandResult NoiseStep::accept(CommandVisitor &visitor) const
{
    return visitor.visit(*this);
}

const string& NoiseStep::operation() const
{
    static const string theType("Noise");
    return theType;
}

void NoiseStep::print(ostream& os) const
{
    LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    SingleStep::print(os);
    Indent id;
    os << endl << indent << "Additive Gaussian noise:";
    {
        Indent id;
        os << endl << indent << "Mean: " << itsMean
            << endl << indent << "Sigma: " << itsSigma;
//            << endl << indent << "Seed: " << itsSeed;
    }
}

const string& NoiseStep::type() const
{
    static const string theOperation("Noise");
    return theOperation;
}

//##--------   P r i v a t e   m e t h o d s   --------##//

void NoiseStep::write(ParameterSet& ps) const
{
    LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    SingleStep::write(ps);

    const string prefix = "Step." + name() + ".Noise.";
    ps.replace(prefix + "Mean", toString(itsMean));
    ps.replace(prefix + "Sigma", toString(itsSigma));
//    ps.replace(prefix + "Seed", toString(itsSeed));

    LOG_TRACE_VAR_STR("\nContents of ParameterSet ps:\n" << ps);
}

void NoiseStep::read(const ParameterSet& ps)
{
    LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    SingleStep::read(ps);
    ParameterSet pss(ps.makeSubset("Noise."));
    itsMean = pss.getDouble("Mean", 0.0);
    itsSigma = pss.getDouble("Sigma", 1.0);
    // Default seed used in boost::random::mersenne_twister.
//    itsSeed = pss.getUint32("Seed", 5489u);
}

} //# namespace BBS
} //# namespace LOFAR
