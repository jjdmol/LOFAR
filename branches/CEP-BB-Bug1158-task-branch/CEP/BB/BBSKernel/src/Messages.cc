//# Messages.cc:
//#
//# Copyright (C) 2008
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

#include <BBSKernel/Messages.h>
#include <BBSKernel/MessageHandler.h>
#include <BBSKernel/Exceptions.h>

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobAipsIO.h>
#include <Blob/BlobSTL.h>

#include <casa/Containers/Record.h>
#include <casa/IO/AipsIO.h>
#include <casa/BasicSL/String.h>

namespace LOFAR
{
namespace BBS
{
    using LOFAR::operator<<;
    using LOFAR::operator>>;

// -------------------------------------------------------------------------- //
    const string CoeffIndexMsg::theirClassType = "CoeffIndexMsg";
    const string CoefficientMsg::theirClassType = "CoefficientMsg";
    const string EquationMsg::theirClassType = "EquationMsg";
    const string SolutionMsg::theirClassType = "SolutionMsg";
    const string ChunkDoneMsg::theirClassType = "ChunkDoneMsg";

    // Register messages with the BlobStreamableFactory. Use an anonymous
    // namespace. This ensures that the variables `dummy*' get their own private
    // storage area and are only visible inside this compilation unit.
    namespace
    {
        bool dummy1 = BlobStreamableFactory::instance().registerClass<CoeffIndexMsg>("CoeffIndexMsg");
        bool dummy2 = BlobStreamableFactory::instance().registerClass<CoefficientMsg>("CoefficientMsg");
        bool dummy3 = BlobStreamableFactory::instance().registerClass<EquationMsg>("EquationMsg");
        bool dummy4 = BlobStreamableFactory::instance().registerClass<SolutionMsg>("SolutionMsg");
        bool dummy5 = BlobStreamableFactory::instance().registerClass<ChunkDoneMsg>("ChunkDoneMsg");
    }


// -------------------------------------------------------------------------- //
  void CoeffIndexMsg::passTo(MessageHandler &handler) const
  {
    handler.handle(*this);
  }

    void CoeffIndexMsg::write(BlobOStream& bos) const
    {
        bos << itsKernelId << itsContents;
    }

    void CoeffIndexMsg::read(BlobIStream& bis)
    {
        bis >> itsKernelId >> itsContents;
    }

    const string& CoeffIndexMsg::classType() const
    {
        return CoeffIndexMsg::theirClassType;
    }

// -------------------------------------------------------------------------- //
    BlobIStream &operator>>(BlobIStream &in, CellCoeff &obj)
    {
        return (in >> obj.id >> obj.coeff);
    }
    
    BlobOStream &operator<<(BlobOStream &out, const CellCoeff &obj)
    {
        return (out << obj.id << obj.coeff);
    }
    
  void CoefficientMsg::passTo(MessageHandler &handler) const
  {
    handler.handle(*this);
  }
  
    void CoefficientMsg::write(BlobOStream& bos) const
    {
        bos << itsKernelId << itsContents;
    }

    void CoefficientMsg::read(BlobIStream& bis)
    {
        bis >> itsKernelId >> itsContents;
    }

    const string& CoefficientMsg::classType() const
    {
        return CoefficientMsg::theirClassType;
    }

// -------------------------------------------------------------------------- //
    BlobIStream &operator>>(BlobIStream &in, CellEquation &obj)
    {
        in >> obj.id;
        
        BlobAipsIO aipsBuffer(in);
        casa::AipsIO aipsStream(&aipsBuffer);
        casa::String aipsErrorMessage;
        casa::Record aipsRecord;

        aipsStream >> aipsRecord;        
        if(!obj.equation.fromRecord(aipsErrorMessage, aipsRecord))
        {
            THROW(BBSKernelException, "Unable to deserialise normal equations ("
                << aipsErrorMessage << ")");
        }
        
        return in;
    }
    
    BlobOStream &operator<<(BlobOStream &out, const CellEquation &obj)
    {
        out << obj.id;
        
        BlobAipsIO aipsBuffer(out);
        casa::AipsIO aipsStream(&aipsBuffer);
        casa::String aipsErrorMessage;
        casa::Record aipsRecord;
        
        if(!obj.equation.toRecord(aipsErrorMessage, aipsRecord))
        {
            THROW(BBSKernelException, "Unable to serialise normal equations ("
                << aipsErrorMessage << ")");
        }        
        
        aipsStream << aipsRecord;
        return out;
    }
    
  void EquationMsg::passTo(MessageHandler &handler) const
  {
    handler.handle(*this);
  }
  
    void EquationMsg::write(BlobOStream& bos) const
    {
        bos << itsKernelId << itsContents;
    }

    void EquationMsg::read(BlobIStream& bis)
    {
        bis >> itsKernelId >> itsContents;
    }

    const string& EquationMsg::classType() const
    {
        return EquationMsg::theirClassType;
    }


// -------------------------------------------------------------------------- //
    BlobIStream &operator>>(BlobIStream &in, CellSolution &obj)
    {
        return (in >> obj.id
            >> obj.coeff
            >> obj.result
            >> obj.resultText
            >> obj.rank
            >> obj.chiSqr
            >> obj.lmFactor);
    }
    
    BlobOStream &operator<<(BlobOStream &out, const CellSolution &obj)
    {
        return (out << obj.id
            << obj.coeff
            << obj.result
            << obj.resultText
            << obj.rank
            << obj.chiSqr
            << obj.lmFactor);
    }
    
  void SolutionMsg::passTo(MessageHandler &handler) const
  {
    handler.handle(*this);
  }
  
    void SolutionMsg::write(BlobOStream& bos) const
    {
        bos << itsContents;
    }

    void SolutionMsg::read(BlobIStream& bis)
    {
        bis >> itsContents;
    }

    const string& SolutionMsg::classType() const
    {
        return SolutionMsg::theirClassType;
    }


// -------------------------------------------------------------------------- //

  void ChunkDoneMsg::passTo(MessageHandler &handler) const
  {
    handler.handle(*this);
  }

    void ChunkDoneMsg::write(BlobOStream& bos) const
    {
        bos << itsKernelId;
    }

    void ChunkDoneMsg::read(BlobIStream& bis)
    {
        bis >> itsKernelId;
    }

    const string& ChunkDoneMsg::classType() const
    {
        return ChunkDoneMsg::theirClassType;
    }
    
} // namespace BBS
} // namespace LOFAR
