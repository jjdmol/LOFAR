//# ExprValueIterator.cc: Iterate the bound values of an ExprValue instance.
//#
//# Copyright (C) 2009
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
#include <BBSKernel/Expr/ExprValueIterator.h>
#include <BBSKernel/Expr/ExprValue.h>

namespace LOFAR
{
namespace BBS
{

ExprValueIterator<Scalar>::ExprValueIterator(const Scalar &value)
{
    itsValueSet = value.getValueSet(0);
    itsIterator = itsValueSet.begin();
    itsKey =
        itsIterator == itsValueSet.end() ? PValueKey() : itsIterator->first;
}

const ExprValueView<Scalar>
ExprValueIterator<Scalar>::value(const PValueKey &key) const
{
    ExprValueView<Scalar> view;

    bool bound = itsIterator != itsValueSet.end()
        && key == itsIterator->first;
    view.assign(bound ? itsIterator->second : itsValueSet.value(), bound);

    return view;
}

void ExprValueIterator<Scalar>::advance(const PValueKey &key)
{
    if(!atEnd() && key == itsIterator->first)
    {
        ++itsIterator;
        itsKey =
            itsIterator == itsValueSet.end() ? PValueKey() : itsIterator->first;
        DBGASSERT(atEnd() != this->key().valid());
    }
}

ExprValueIterator<JonesMatrix>::ExprValueIterator(const JonesMatrix &value)
{
    itsAtEnd = true;
    for(unsigned int i = 0; i < 4; ++i)
    {
        itsValueSet[i] = value.getValueSet(i);
        itsIterator[i] = itsValueSet[i].begin();

        if(itsIterator[i] != itsValueSet[i].end())
        {
            itsAtEnd = false;
            itsKey = std::min(itsKey, itsIterator[i]->first);
        }
    }
}

const ExprValueView<JonesMatrix>
ExprValueIterator<JonesMatrix>::value(const PValueKey &key) const
{
    ExprValueView<JonesMatrix> view;

    bool bound = itsIterator[0] != itsValueSet[0].end()
        && key == itsIterator[0]->first;
    view.assign(0, 0,
        bound ? itsIterator[0]->second : itsValueSet[0].value(), bound);
    bound = itsIterator[1] != itsValueSet[1].end()
        && key == itsIterator[1]->first;
    view.assign(0, 1,
        bound ? itsIterator[1]->second : itsValueSet[1].value(), bound);
    bound = itsIterator[2] != itsValueSet[2].end()
        && key == itsIterator[2]->first;
    view.assign(1, 0,
        bound ? itsIterator[2]->second : itsValueSet[2].value(), bound);
    bound = itsIterator[3] != itsValueSet[3].end()
        && key == itsIterator[3]->first;
    view.assign(1, 1,
        bound ? itsIterator[3]->second : itsValueSet[3].value(), bound);

    return view;
}

void ExprValueIterator<JonesMatrix>::advance(const PValueKey &key)
{
    if(!itsAtEnd && key == itsKey)
    {
        PValueKey nextKey;
        bool nextAtEnd = true;

        for(unsigned int i = 0; i < 4; ++i)
        {
            if(itsIterator[i] != itsValueSet[i].end()
                && key == itsIterator[i]->first)
            {
                ++itsIterator[i];
            }

            if(itsIterator[i] != itsValueSet[i].end())
            {
                nextAtEnd = false;
                nextKey = std::min(nextKey, itsIterator[i]->first);
            }
        }

        // Update iterator state. This is placed here such that this method
        // works correctly even when key is a reference to itsKey (as in
        // advance(key()).
        itsKey = nextKey;
        itsAtEnd = nextAtEnd;
        DBGASSERT(itsAtEnd != itsKey.valid());
    }
}

} //# namespace BBS
} //# namespace LOFAR
