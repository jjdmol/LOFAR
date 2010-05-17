//# ExprValue.cc: The result of an expression.
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
#include <BBSKernel/Expr/ExprValue.h>

namespace LOFAR
{
namespace BBS
{

ExprValue::~ExprValue()
{
}

const Scalar::View Scalar::view() const
{
    View view;
    view.assign(itsElement.value());
    return view;
}

const Scalar::View Scalar::view(const PValueKey &key) const
{
    DBGASSERT(key.valid());

    bool found;
    View view;
    const Matrix &value = itsElement.value(key, found);
    view.assign(value, found);

    return view;
}

void Scalar::assign(const View &value)
{
    if(value.bound())
    {
        itsElement.assign(value());
    }
}

void Scalar::assign(const PValueKey &key, const View &value)
{
    if(value.bound())
    {
        itsElement.assign(key, value());
    }
}

const JonesMatrix::View JonesMatrix::view() const
{
    View view;
    view.assign(0, 0, itsElement[0].value());
    view.assign(0, 1, itsElement[1].value());
    view.assign(1, 0, itsElement[2].value());
    view.assign(1, 1, itsElement[3].value());

    return view;
}

const JonesMatrix::View JonesMatrix::view(const PValueKey &key) const
{
    DBGASSERT(key.valid());

    bool found;
    View view;

    const Matrix tmp00 = itsElement[0].value(key, found);
    view.assign(0, 0, tmp00, found);
    const Matrix tmp01 = itsElement[1].value(key, found);
    view.assign(0, 1, tmp01, found);
    const Matrix tmp10 = itsElement[2].value(key, found);
    view.assign(1, 0, tmp10, found);
    const Matrix tmp11 = itsElement[3].value(key, found);
    view.assign(1, 1, tmp11, found);

    return view;
}

void JonesMatrix::assign(const View &value)
{
    if(value.bound(0, 0))
    {
        itsElement[0].assign(value(0, 0));
    }

    if(value.bound(0, 1))
    {
        itsElement[1].assign(value(0, 1));
    }

    if(value.bound(1, 0))
    {
        itsElement[2].assign(value(1, 0));
    }

    if(value.bound(1, 1))
    {
        itsElement[3].assign(value(1, 1));
    }
}

void JonesMatrix::assign(const PValueKey &key, const View &value)
{
    if(value.bound(0, 0))
    {
        itsElement[0].assign(key, value(0, 0));
    }

    if(value.bound(0, 1))
    {
        itsElement[1].assign(key, value(0, 1));
    }

    if(value.bound(1, 0))
    {
        itsElement[2].assign(key, value(1, 0));
    }

    if(value.bound(1, 1))
    {
        itsElement[3].assign(key, value(1, 1));
    }
}

} //# namespace BBS
} //# namespace LOFAR
