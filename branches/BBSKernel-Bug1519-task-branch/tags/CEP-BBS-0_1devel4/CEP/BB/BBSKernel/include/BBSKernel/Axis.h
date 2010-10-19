//# Axis.h:
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

#ifndef LOFAR_BBS_BBSKERNEL_GRID_H
#define LOFAR_BBS_BBSKERNEL_GRID_H

#include <Common/lofar_vector.h>
#include <Common/lofar_algorithm.h>
#include <functional>
#include <utility>

namespace LOFAR
{
namespace BBS
{
using std::unary_function;
using std::pair;
using std::make_pair;
using std::max;
using std::min;

class regular_series: public unary_function<size_t, double>
{
public:
    regular_series()
        :   m_begin(0.0),
            m_delta(1.0)
    {}

    regular_series(result_type begin, result_type delta)
        :   m_begin(begin),
            m_delta(delta)
    {}

    result_type operator()(argument_type n) const
    { return m_begin + n * m_delta; }
 
private:
    result_type    m_begin, m_delta;
};


class irregular_series: public unary_function<size_t, double>
{
public:
    irregular_series()
    {}

    irregular_series(const vector<result_type> &terms)
        : m_terms(terms)
    {}

    result_type operator()(argument_type n) const
    { return m_terms[n]; }

private:
    vector<result_type>    m_terms;
};


template <typename SeriesType>
class cell_centered_axis
{
public:
    cell_centered_axis()
        :   m_series(SeriesType()),
            m_size(0)
    {}

    cell_centered_axis(SeriesType series, size_t size)
        :   m_series(series),
            m_size(size)
    {}

    double operator()(size_t n) const
    { return 0.5 * (m_series(n) + m_series(n + 1)); }

    double lower(size_t n) const
    { return m_series(n); }

    double upper(size_t n) const
    { return m_series(n + 1); }

    double width(size_t n) const
    { return m_series(n + 1) - m_series(n); }

    size_t size() const
    { return m_size; }

    pair<double, double> range() const
    { return make_pair(lower(0), upper(size() - 1)); }

private:
    SeriesType         m_series;
    size_t             m_size;
};


template <typename SeriesType>
class node_centered_axis
{
public:
    node_centered_axis()
        :   m_series(SeriesType()),
            m_size(0)
    {}

    node_centered_axis(SeriesType series, size_t size)
        :   m_series(series),
            m_size(size)
    {}

    double operator()(size_t n) const
    { return m_series(n); }

    double lower(size_t n) const
    { return m_series(n); }

    double upper(size_t n) const
    { return m_series(n); }

    double width(size_t n) const
    { return 0.0; }

    size_t size() const
    { return m_size; }

    pair<double, double> range() const
    { return make_pair(lower(0), upper(size() - 1)); }

private:
    SeriesType         m_series;
    size_t             m_size;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
