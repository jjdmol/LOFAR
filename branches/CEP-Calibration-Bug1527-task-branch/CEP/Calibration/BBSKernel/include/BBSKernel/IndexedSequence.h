//# IndexedSequence.h: List that combines constant time element access and
//# logarithmic time look-up of the index of any of its elements.
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

#ifndef LOFAR_BBSKERNEL_INDEXEDLIST_H
#define LOFAR_BBSKERNEL_INDEXEDLIST_H

// \file
// List that combines constant time element access and logarithmic time look-up
// of the index of any of its elements.

#include <Common/lofar_iostream.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>
#include <iterator>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

template <typename T_ELEMENT>
class IndexedSequence
{
public:
    typedef map<T_ELEMENT, size_t>                      IndexType;
    typedef vector<typename IndexType::const_iterator>  SequenceType;

    class const_iterator
    {
    public:
        // Try to be STL compliant.
        typedef std::forward_iterator_tag   iterator_category;
        typedef T_ELEMENT                   value_type;
        typedef std::ptrdiff_t              difference_type;
        typedef const T_ELEMENT*            pointer;
        typedef const T_ELEMENT&            reference;

        const_iterator(typename SequenceType::const_iterator it)
            :   itsIterator(it)
        {
        }

        // Element access.
        // @{
        reference operator*() const
        {
            return (*itsIterator)->first;
        }

        pointer operator->() const
        {
            return &((*itsIterator)->first);
        }
        // @}

        // Test for (in)equality.
        // @{
        bool operator==(const const_iterator &rhs) const
        {
            return itsIterator == rhs.itsIterator;
        }

        bool operator!=(const const_iterator &rhs) const
        {
            return itsIterator != rhs.itsIterator;
        }
        // @}

        // Advance the iterator.
        // @{
        const_iterator &operator++(void)
        {
            ++itsIterator;
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator tmp(*this);
            ++itsIterator;
            return tmp;
        }
        // @}

    private:
        typename SequenceType::const_iterator   itsIterator;
    };

    IndexedSequence();
    template <typename T_ITER>
    IndexedSequence(T_ITER first, T_ITER last);

    // Copy constructor.
    IndexedSequence(const IndexedSequence &rhs);
    // Assignment operator.
    IndexedSequence &operator=(const IndexedSequence &rhs);

    // Return the size of the sequence.
    size_t size() const;

    // Erase all of the elements.
    void clear();

    // Access an element by index.
    const T_ELEMENT &operator[](size_t i) const;

    // Find the index of the element in the sequence. If the element cannot be
    // found, size() is returned.
    size_t index(const T_ELEMENT &element) const;

    // Append an element to the sequence.
    void append(const T_ELEMENT &element);

    // Check if the sequence contains the provided element.
    bool contains(const T_ELEMENT &element) const;

    // Return a const_iterator that points to the beginning of the sequence.
    const_iterator begin() const;
    // Return a const_iterator that points to the end of the sequence.
    const_iterator end() const;

private:
    map<T_ELEMENT, size_t>                                  itsIndex;
    vector<typename map<T_ELEMENT, size_t>::const_iterator> itsSequence;
};

// Write an IndexedSequence to an output stream in human readable form.
template <typename T_ELEMENT>
ostream &operator<<(ostream &out, const IndexedSequence<T_ELEMENT> &obj)
{
    out << "[";
    for(typename IndexedSequence<T_ELEMENT>::const_iterator it = obj.begin(),
        end = obj.end(); it != end; ++it)
    {
        out << " " << *it;
    }
    out << " ]";
    return out;
}

// @}

// -------------------------------------------------------------------------- //
// - IndexedSequence implementation                                         - //
// -------------------------------------------------------------------------- //

template <typename T_ELEMENT>
IndexedSequence<T_ELEMENT>::IndexedSequence()
{
}


template <typename T_ELEMENT>
template <typename T_ITER>
IndexedSequence<T_ELEMENT>::IndexedSequence(T_ITER first, T_ITER last)
{
    while(first != last)
    {
        append(*first++);
    }
}

template <typename T_ELEMENT>
IndexedSequence<T_ELEMENT>::IndexedSequence(const IndexedSequence &rhs)
{
    for(const_iterator it = rhs.begin(), end = rhs.end(); it != end; ++it)
    {
        append(*it);
    }
}

template <typename T_ELEMENT>
IndexedSequence<T_ELEMENT> &IndexedSequence<T_ELEMENT>::operator=
    (const IndexedSequence &rhs)
{
    if(&rhs != this)
    {
        clear();
        for(const_iterator it = rhs.begin(), end = rhs.end(); it != end;
            ++it)
        {
            append(*it);
        }
    }

    return *this;
}

template <typename T_ELEMENT>
inline const T_ELEMENT &IndexedSequence<T_ELEMENT>::operator[](size_t i) const
{
    return itsSequence[i]->first;
}

template <typename T_ELEMENT>
inline size_t IndexedSequence<T_ELEMENT>::size() const
{
    return itsSequence.size();
}

template <typename T_ELEMENT>
size_t IndexedSequence<T_ELEMENT>::index(const T_ELEMENT &element) const
{
    typename map<T_ELEMENT, size_t>::const_iterator it = itsIndex.find(element);
    return (it == itsIndex.end() ? size() : it->second);
}

template <typename T_ELEMENT>
void IndexedSequence<T_ELEMENT>::append(const T_ELEMENT &element)
{
    pair<typename map<T_ELEMENT, size_t>::iterator, bool> status =
        itsIndex.insert(make_pair(element, size()));

    if(status.second)
    {
        itsSequence.push_back(status.first);
    }
}

template <typename T_ELEMENT>
bool IndexedSequence<T_ELEMENT>::contains(const T_ELEMENT &element) const
{
    return (itsIndex.find(element) != itsIndex.end());
}

template <typename T_ELEMENT>
inline typename IndexedSequence<T_ELEMENT>::const_iterator
    IndexedSequence<T_ELEMENT>::begin() const
{
    return const_iterator(itsSequence.begin());
}

template <typename T_ELEMENT>
inline typename IndexedSequence<T_ELEMENT>::const_iterator
    IndexedSequence<T_ELEMENT>::end() const
{
    return const_iterator(itsSequence.end());
}

template <typename T_ELEMENT>
void IndexedSequence<T_ELEMENT>::clear()
{
    itsIndex.clear();
    itsSequence.clear();
}

} //# namespace BBS
} //# namespace LOFAR

#endif
