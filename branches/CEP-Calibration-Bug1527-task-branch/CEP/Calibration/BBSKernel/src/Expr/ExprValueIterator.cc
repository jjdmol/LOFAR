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
    itsElement = value.getElement(0);
    itsIterator = itsElement.begin();
    itsKey =
        itsIterator == itsElement.end() ? PValueKey() : itsIterator->first;
}

const ExprValueView<Scalar>
ExprValueIterator<Scalar>::value(const PValueKey &key) const
{
    ExprValueView<Scalar> view;

    bool bound = itsIterator != itsElement.end()
        && key == itsIterator->first;
    view.assign(bound ? itsIterator->second : itsElement.value(), bound);

    return view;
}

void ExprValueIterator<Scalar>::advance(const PValueKey &key)
{
    if(!atEnd() && key == itsIterator->first)
    {
        ++itsIterator;
        itsKey =
            itsIterator == itsElement.end() ? PValueKey() : itsIterator->first;
        DBGASSERT(atEnd() != this->key().valid());
    }
}

ExprValueIterator<JonesMatrix>::ExprValueIterator(const JonesMatrix &value)
{
    itsAtEnd = true;
    for(unsigned int i = 0; i < 4; ++i)
    {
        itsElement[i] = value.getElement(i);
        itsIterator[i] = itsElement[i].begin();

        if(itsIterator[i] != itsElement[i].end())
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

    bool bound = itsIterator[0] != itsElement[0].end()
        && key == itsIterator[0]->first;
    view.assign(0, 0,
        bound ? itsIterator[0]->second : itsElement[0].value(), bound);
    bound = itsIterator[1] != itsElement[1].end()
        && key == itsIterator[1]->first;
    view.assign(0, 1,
        bound ? itsIterator[1]->second : itsElement[1].value(), bound);
    bound = itsIterator[2] != itsElement[2].end()
        && key == itsIterator[2]->first;
    view.assign(1, 0,
        bound ? itsIterator[2]->second : itsElement[2].value(), bound);
    bound = itsIterator[3] != itsElement[3].end()
        && key == itsIterator[3]->first;
    view.assign(1, 1,
        bound ? itsIterator[3]->second : itsElement[3].value(), bound);

    return view;
}

void ExprValueIterator<JonesMatrix>::advance(const PValueKey &key)
{
    if(!atEnd() && key == itsKey)
    {
        itsAtEnd = true;
        itsKey = PValueKey();
        for(unsigned int i = 0; i < 4; ++i)
        {
            if(itsIterator[i] != itsElement[i].end()
                && key == itsIterator[i]->first)
            {
                ++itsIterator[i];
            }

            if(itsIterator[i] != itsElement[i].end())
            {
                itsAtEnd = false;
                itsKey = std::min(itsKey, itsIterator[i]->first);
            }
        }

        DBGASSERT(itsAtEnd != itsKey.valid());
    }
}

} //# namespace BBS
} //# namespace LOFAR
