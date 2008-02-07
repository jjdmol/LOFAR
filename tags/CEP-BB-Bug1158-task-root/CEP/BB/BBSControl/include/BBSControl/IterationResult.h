//# IterationResult.h: Result of a single non-Levenberg-Marquardt iteration
//#     sent from solver to kernel.
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

#ifndef LOFAR_BBSCONTROL_ITERATIONRESULT_H
#define LOFAR_BBSCONTROL_ITERATIONRESULT_H

// \file
// Result of a single Levenberg-Marquardt iteration sent from solver to kernel.

#include <Blob/BlobStreamable.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
    class BlobIStream;
    class BlobOStream;

namespace BBS
{
    // \addtogroup BBSControl
    // @{

    // Result of a single Levenberg-Marquardt iteration sent from solver to kernel.
    class IterationResult : public BlobStreamable
    {
    public:
        IterationResult(uint32 domainIndex = 0)
            :   itsDomainIndex(domainIndex)
        {
        }
    
        IterationResult(uint32 domainIndex,
            uint32 resultCode,
            string resultText,
            const vector<double> &unknowns,
            uint32 rank,
            double chiSquared,
            double lmFactor)
            :   itsDomainIndex(domainIndex),
                itsResultCode(resultCode),
                itsResultText(resultText),
                itsUnknowns(unknowns),
                itsRank(rank),
                itsChiSquared(chiSquared),
                itsLMFactor(lmFactor)
        {
        }
        
        uint32 getDomainIndex() const
        {
            return itsDomainIndex;
        }
    
        uint32 getResultCode() const
        {
            return itsResultCode;
        }
        
        const string &getResultText() const
        {
            return itsResultText;
        }
        
        const vector<double> &getUnknowns() const
        {
            return itsUnknowns;
        }
        
        uint32 getRank() const
        {
            return itsRank;
        }
        
        double getChiSquared() const
        {
            return itsChiSquared;
        }
        
        double getLMFactor() const
        {
            return itsLMFactor;
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
        uint32              itsResultCode;
        string              itsResultText;
        vector<double>      itsUnknowns;
        uint32              itsRank;
        double              itsChiSquared;
        double              itsLMFactor;
    };
    // @}
} //# namespace BBS
} //# namespace LOFAR

#endif
