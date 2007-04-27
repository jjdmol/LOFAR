//# DomainRegistrationRequest.h: request sent from kernel to solver to
//#     register a solve domain and set the initial values of the unknowns.
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

#ifndef LOFAR_BBSCONTROL_DOMAINREGISTRATIONREQUEST_H
#define LOFAR_BBSCONTROL_DOMAINREGISTRATIONREQUEST_H

// \file
// Request sent from kernel to solver to register a solve domain and set the initial values of the unknowns.


#include <Blob/BlobStreamable.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
    class BlobIStream;
    class BlobOStream;

namespace BBS
{
    // \addtogroup BBSControl
    // @{

    // Request sent from kernel to solver to register a solve domain and set the initial values of the
    // unknowns.
    //
    // Note that the initial values are not necessary for computing a solution, because a single iteration consists
    // of computing the solution to a set of linear (normal) equations (see AIPS++ note 224). However, they seem to
    // be needed to check for convergence (inside LSQFit).
    class DomainRegistrationRequest : public BlobStreamable
    {
    public:
        DomainRegistrationRequest(uint32 domainIndex = 0)
            :   itsDomainIndex(domainIndex),
                itsEpsilon(1e-8),
                itsMaxIter(0)
        {
        }
    
        DomainRegistrationRequest(uint32 domainIndex, double epsilon, uint32 maxIter, const vector<double> &unknowns)
            :   itsDomainIndex(domainIndex),
                itsEpsilon(epsilon),
                itsMaxIter(maxIter),
                itsUnknowns(unknowns)
        {
        }
        
        uint32 getDomainIndex() const
        {
            return itsDomainIndex;
        }
    
        double getEpsilon() const
        {
            return itsEpsilon;
        }
        
        uint32 getMaxIter() const
        {
            return itsMaxIter;
        }
        
        const vector<double> &getUnknowns() const
        {
            return itsUnknowns;
        }

        static const string theirClassType;
   
    private:
        //# -------- BlobStreamable interface implementation -------- 
    
        // Write the contents of \c *this into the blob output stream \a bos.
        virtual void write(BlobOStream& bos) const;

        // Read the contents from the blob input stream \a bis into \c *this.
        virtual void read(BlobIStream& bis);

        // Return the type of \c *this as a string.
        virtual const string& classType() const;
        
        //# -------- Attributes -------- 
        uint32              itsDomainIndex;
        double              itsEpsilon;
        uint32              itsMaxIter;
        vector<double>      itsUnknowns;
    };

    // @}
} //# namespace BBS
} //# namespace LOFAR

#endif
