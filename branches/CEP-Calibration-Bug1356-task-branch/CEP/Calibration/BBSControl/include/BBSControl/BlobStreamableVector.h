//# BlobStreamableVector.h: Helper class to send a vector of BlobStreamable
//#     objects (or derivatives) through a DH_BlobStreamable.
//#
//# Copyright (C) 2006
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

#ifndef LOFAR_BBSCONTROL_BLOBSTREAMABLEVECTOR_H
#define LOFAR_BBSCONTROL_BLOBSTREAMABLEVECTOR_H

// \file
// Helper class to send a vector of BlobStreamable objects (or derivatives) through a DH_BlobStreamable.

#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Blob/BlobStreamable.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <boost/shared_ptr.hpp>

namespace LOFAR
{
namespace BBS
{
    using LOFAR::operator<<;
    
    // \addtogroup BBSControl
    // @{

    // Helper class to send a vector of BlobStreamable objects (or derivatives) through a DH_BlobStreamable.
    template <typename T>
    class BlobStreamableVector : public BlobStreamable
    {
    public:
        BlobStreamableVector()
            : BlobStreamable()
        {
        }
        
        ~BlobStreamableVector()
        {
            for(typename vector<T*>::iterator it = itsVector.begin();
                it != itsVector.end();
                ++it)
            {
                delete *it;
            }
        }

                
        vector<T*> &getVector()
        {
            return itsVector;
        }

        
        const vector<T*> &getVector() const
        {
            return itsVector;
        }
    
        
    private:
        BlobStreamableVector(const BlobStreamableVector&);
        BlobStreamableVector& operator=(const BlobStreamableVector&);

        // Read the contents from the blob input stream \a bis into \c *this.
        virtual void read(BlobIStream& bis)
        {
            uint32 size;

            bis >> size;
            for(uint32 i = 0; i < size; ++i)
            {
                itsVector.push_back(dynamic_cast<T*>(BlobStreamable::deserialize(bis)));
            }
        }

        // Write the contents of \c *this into the blob output stream \a bos.
        virtual void write(BlobOStream& bos) const
        {
            bos << static_cast<uint32>(itsVector.size());

            for(typename vector<T*>::const_iterator it = itsVector.begin();
                it != itsVector.end();
                ++it)
            {
                (*it)->serialize(bos);
            }
        }

        // Return the class type of \c *this as a string.
        virtual const string& classType() const
        {
            static const string theType("BlobStreamableVector<" + T::theirClassType + ">");
            return theType;
        }
        
        vector<T*> itsVector;
    };
    // @}
} //# namespace BBS
} //# namespace LOFAR

#endif
