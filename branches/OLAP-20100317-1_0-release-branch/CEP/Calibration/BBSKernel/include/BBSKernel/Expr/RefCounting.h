//# RefCounting.h: Templated base class to reference count "enable" classes.
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

#ifndef LOFAR_BBSKERNEL_EXPR_REFCOUNTING_H
#define LOFAR_BBSKERNEL_EXPR_REFCOUNTING_H

// \file
// Templated base class to reference count "enable" classes.

#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class RefCountable
{
public:
    RefCountable();
    explicit RefCountable(const RefCountable &other);
    // TODO: If using boost::intrusive_ptr make this destructor virtual.
    ~RefCountable();

    void upRefCount() const;
    unsigned int downRefCount() const;
    unsigned int getRefCount() const;

private:
    RefCountable &operator=(const RefCountable &other);

    mutable unsigned int    itsRefCount;
};

template <typename T_IMPL>
class RefCounted
{
public:
    RefCounted();
    explicit RefCounted(T_IMPL *impl);
    ~RefCounted();

    RefCounted(const RefCounted &other);
    RefCounted &operator=(const RefCounted &rhs);

    unsigned int getRefCount() const;
    bool initialized() const;

protected:
    T_IMPL &instance();
    const T_IMPL &instance() const;

private:
    T_IMPL  *itsImpl;
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: RefCountable                                           - //
// -------------------------------------------------------------------------- //

//TODO: Will an initial count of 0 cause problems?
inline RefCountable::RefCountable()
    : itsRefCount(0)
{
//    LOG_DEBUG("RefCountable()");
}

inline RefCountable::RefCountable(const RefCountable&)
    : itsRefCount(0)
{
    // The reference count is not copied, because a call to this copy
    // constructor implies a fresh copy has been made of the counted instance,
    // which should therefore start out with a count of 0.
}

inline RefCountable::~RefCountable()
{
//    LOG_DEBUG("RefCountable destructor");
    ASSERT(itsRefCount == 0);
}

inline void RefCountable::upRefCount() const
{
    ++itsRefCount;
}

inline unsigned int RefCountable::downRefCount() const
{
    return --itsRefCount;
}

inline unsigned int RefCountable::getRefCount() const
{
    return itsRefCount;
}

// -------------------------------------------------------------------------- //
// - Implementation: RefCounted                                             - //
// -------------------------------------------------------------------------- //

template <typename T_IMPL>
inline RefCounted<T_IMPL>::RefCounted()
    :   itsImpl(0)
{
}

template <typename T_IMPL>
inline RefCounted<T_IMPL>::RefCounted(T_IMPL *impl)
    :   itsImpl(impl)
{
    ASSERT(itsImpl);
    itsImpl->upRefCount();
}

template <typename T_IMPL>
inline RefCounted<T_IMPL>::RefCounted(const RefCounted &other)
{
//    cout << "RefCounted copy constructor" << endl;

    itsImpl = other.itsImpl;
    if(itsImpl)
    {
        itsImpl->upRefCount();
    }
}

template <typename T_IMPL>
inline RefCounted<T_IMPL>::~RefCounted()
{
    if(itsImpl && itsImpl->downRefCount() == 0)
    {
        delete itsImpl;
    }
}

template <typename T_IMPL>
inline RefCounted<T_IMPL> &RefCounted<T_IMPL>::operator=
    (const RefCounted<T_IMPL> &rhs)
{
//    cout << "RefCounted assignment operator" << endl;

    if(this != &rhs)
    {
        if(itsImpl && itsImpl->downRefCount() == 0)
        {
            delete itsImpl;
        }

        itsImpl = rhs.itsImpl;
        if(itsImpl)
        {
            itsImpl->upRefCount();
        }
    }

    return *this;
}

template <typename T_IMPL>
inline bool RefCounted<T_IMPL>::initialized() const
{
    return itsImpl != 0;
}

template <typename T_IMPL>
inline unsigned int RefCounted<T_IMPL>::getRefCount() const
{
    return itsImpl ? itsImpl->getRefCount() : 0;
}

template <typename T_IMPL>
inline T_IMPL &RefCounted<T_IMPL>::instance()
{
    DBGASSERT(initialized());
    return *itsImpl;
}

template <typename T_IMPL>
inline const T_IMPL &RefCounted<T_IMPL>::instance() const
{
    DBGASSERT(initialized());
    return *itsImpl;
}

// -------------------------------------------------------------------------- //
// - Implementation: boost::intrusive_ptr support                           - //
// -------------------------------------------------------------------------- //

//inline void intrusive_ptr_add_ref(RefCountable *obj)
//{
//    ASSERT(obj);
//    obj->upRefCount();
//}

//inline void intrusive_ptr_release(RefCountable *obj)
//{
//    ASSERT(obj);
//    if(obj->downRefCount() == 0)
//    {
//        delete obj;
//    }
//}

} //# namespace BBS
} //# namespace LOFAR

#endif
