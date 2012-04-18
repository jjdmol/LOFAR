//# IndexedSequence.h: Sequence that provides constant time element access and
//# logarithmic time look-up of the index of an element in the sequence.
//#
//# Copyright (C) 2009
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

#ifndef LOFAR_BBSKERNEL_INDEXEDSEQUENCE_H
#define LOFAR_BBSKERNEL_INDEXEDSEQUENCE_H

// \file
// Sequence that provides constant time element access and logarithmic time
// look-up of the index of an element in the sequence.

#include <Common/lofar_iostream.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

template <typename T>
class IndexedSequence
{
public:
    typedef map<T, size_t>                              IndexType;
    typedef vector<typename IndexType::const_iterator>  SequenceType;

    IndexedSequence();

    template <typename T_ITER>
    IndexedSequence(T_ITER first, T_ITER last);

    // Copy constructor.
    IndexedSequence(const IndexedSequence &rhs);

    // Assignment operator.
    IndexedSequence &operator=(const IndexedSequence &rhs);

    // Check for empty sequence.
    bool empty() const;

    // Return the size of the sequence.
    size_t size() const;

    // Erase all of the elements.
    void clear();

    // Access an element by index.
    const T &operator[](size_t i) const;

    // Find the index of the element in the sequence. If the element cannot be
    // found, size() is returned.
    size_t index(const T &element) const;

    // Append an element to the sequence.
    void append(const T &element);

    // Check if the sequence contains the provided element.
    bool contains(const T &element) const;

    template <typename U, typename T_OUTPUT_ITER>
    friend void makeIndexMap(const IndexedSequence<U> &lhs,
        const IndexedSequence<U> &rhs, T_OUTPUT_ITER out);

    template <typename U, typename T_PREDICATE, typename T_OUTPUT_ITER>
    friend void makeIndexMap(const IndexedSequence<U> &lhs,
        const IndexedSequence<U> &rhs, T_PREDICATE predicate,
        T_OUTPUT_ITER out);

private:
    IndexType       itsIndex;
    SequenceType    itsSequence;
};

// Write an IndexedSequence to an output stream in human readable form.
template <typename T>
ostream &operator<<(ostream &out, const IndexedSequence<T> &obj);

// Compute a mapping that maps the index of an element in lhs to the index of
// that same element in rhs. For each element that is contained in both
// sequences a pair<size_t, size_t> with the corresponding indices is written to
// the output iterator.
template <typename T, typename T_OUTPUT_ITER>
void makeIndexMap(const IndexedSequence<T> &lhs, const IndexedSequence<T> &rhs,
    T_OUTPUT_ITER out);

// Compute a mapping that maps the index of an element in lhs to the index of
// that same element in rhs. For each element that is contained in both
// sequences _and_ for which the predicate evaluates to true, a pair<size_t,
// size_t> with the corresponding indices is written to the output iterator.
template <typename T, typename T_PREDICATE, typename T_OUTPUT_ITER>
void makeIndexMap(const IndexedSequence<T> &lhs, const IndexedSequence<T> &rhs,
    T_PREDICATE predicate, T_OUTPUT_ITER out);

// Output a sequence that contains a copy of each element in the input sequence
// for which the predicate evaluates to true.
template <typename T, typename T_PREDICATE>
IndexedSequence<T> filter(const IndexedSequence<T> &in, T_PREDICATE predicate);

// @}

// -------------------------------------------------------------------------- //
// - IndexedSequence implementation                                         - //
// -------------------------------------------------------------------------- //

template <typename T>
IndexedSequence<T>::IndexedSequence()
{
}

template <typename T>
template <typename T_ITER>
IndexedSequence<T>::IndexedSequence(T_ITER first, T_ITER last)
{
    while(first != last)
    {
        append(*first++);
    }
}

template <typename T>
IndexedSequence<T>::IndexedSequence(const IndexedSequence &rhs)
{
    for(size_t i = 0; i < rhs.size(); ++i)
    {
        append(rhs[i]);
    }
}

template <typename T>
IndexedSequence<T> &IndexedSequence<T>::operator=
    (const IndexedSequence &rhs)
{
    if(&rhs != this)
    {
        clear();
        for(size_t i = 0; i < rhs.size(); ++i)
        {
            append(rhs[i]);
        }
    }

    return *this;
}

template <typename T>
inline const T &IndexedSequence<T>::operator[](size_t i) const
{
    return itsSequence[i]->first;
}

template <typename T>
inline bool IndexedSequence<T>::empty() const
{
    return itsSequence.empty();
}

template <typename T>
inline size_t IndexedSequence<T>::size() const
{
    return itsSequence.size();
}

template <typename T>
void IndexedSequence<T>::clear()
{
    itsIndex.clear();
    itsSequence.clear();
}

template <typename T>
size_t IndexedSequence<T>::index(const T &element) const
{
    typename IndexType::const_iterator it = itsIndex.find(element);
    return (it == itsIndex.end() ? size() : it->second);
}

template <typename T>
void IndexedSequence<T>::append(const T &element)
{
    pair<typename IndexType::iterator, bool> status =
        itsIndex.insert(make_pair(element, size()));

    if(status.second)
    {
        itsSequence.push_back(status.first);
    }
}

template <typename T>
bool IndexedSequence<T>::contains(const T &element) const
{
    return (itsIndex.find(element) != itsIndex.end());
}

template <typename T>
ostream &operator<<(ostream &out, const IndexedSequence<T> &obj)
{
    out << "[";
    for(size_t i = 0; i < obj.size(); ++i)
    {
        out << " " << obj[i];
    }
    out << " ]";
    return out;
}

template <typename T, typename T_OUTPUT_ITER>
void makeIndexMap(const IndexedSequence<T> &lhs, const IndexedSequence<T> &rhs,
    T_OUTPUT_ITER out)
{
    typedef typename IndexedSequence<T>::IndexType::const_iterator ConstIt;

    ConstIt it_lhs = lhs.itsIndex.begin(), end_lhs = lhs.itsIndex.end();
    ConstIt it_rhs = rhs.itsIndex.begin(), end_rhs = rhs.itsIndex.end();

    while(it_lhs != end_lhs && it_rhs != end_rhs)
    {
        if(it_lhs->first == it_rhs->first)
        {
            *out++ = make_pair(it_lhs->second, it_rhs->second);
            ++it_lhs;
            ++it_rhs;
        }
        else if(it_lhs->first < it_rhs->first)
        {
            ++it_lhs;
        }
        else
        {
            ++it_rhs;
        }
    }
}

template <typename T, typename T_PREDICATE, typename T_OUTPUT_ITER>
void makeIndexMap(const IndexedSequence<T> &lhs, const IndexedSequence<T> &rhs,
    T_PREDICATE predicate, T_OUTPUT_ITER out)
{
    typedef typename IndexedSequence<T>::IndexType::const_iterator ConstIt;

    ConstIt it_lhs = lhs.itsIndex.begin(), end_lhs = lhs.itsIndex.end();
    ConstIt it_rhs = rhs.itsIndex.begin(), end_rhs = rhs.itsIndex.end();

    while(it_lhs != end_lhs && it_rhs != end_rhs)
    {
        if(it_lhs->first == it_rhs->first)
        {
            if(predicate(it_lhs->first))
            {
                *out++ = make_pair(it_lhs->second, it_rhs->second);
            }

            ++it_lhs;
            ++it_rhs;
        }
        else if(it_lhs->first < it_rhs->first)
        {
            ++it_lhs;
        }
        else
        {
            ++it_rhs;
        }
    }
}

template <typename T, typename T_PREDICATE>
IndexedSequence<T> filter(const IndexedSequence<T> &in, T_PREDICATE predicate)
{
    IndexedSequence<T> out;

    for(size_t i = 0; i < in.size(); ++i)
    {
        if(predicate(in[i]))
        {
            out.append(in[i]);
        }
    }

    return out;
}

} //# namespace BBS
} //# namespace LOFAR

#endif
