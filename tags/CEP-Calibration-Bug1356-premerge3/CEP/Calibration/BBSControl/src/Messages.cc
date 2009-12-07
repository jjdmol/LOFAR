//# Messages.cc:
//#
//# Copyright (C) 2008
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

#include <lofar_config.h>

#include <BBSControl/Messages.h>
#include <BBSControl/MessageHandlers.h>
#include <BBSControl/Exceptions.h>

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

    // -------------------------------------------------------------------- //
//    const string KernelIdMsg::theirClassType = "KernelIdMsg";
    const string ProcessIdMsg::theirClassType = "ProcessIdMsg";
    const string CoeffIndexMsg::theirClassType = "CoeffIndexMsg";
    const string MergedCoeffIndexMsg::theirClassType = "MergedCoeffIndexMsg";
    const string CoeffMsg::theirClassType = "CoeffMsg";
    const string EquationMsg::theirClassType = "EquationMsg";
    const string SolutionMsg::theirClassType = "SolutionMsg";
    const string ChunkDoneMsg::theirClassType = "ChunkDoneMsg";

    // Register messages with the BlobStreamableFactory. Use an anonymous
    // namespace. This ensures that the variables `dummy*' get their own
    // private storage area and are only visible inside this compilation unit.
    namespace
    {
//      bool dummy1 = BlobStreamableFactory::instance().
//        registerClass<KernelIdMsg>("KernelIdMsg");
      bool dummy1 = BlobStreamableFactory::instance().
        registerClass<ProcessIdMsg>("ProcessIdMsg");
      bool dummy2 = BlobStreamableFactory::instance().
        registerClass<CoeffIndexMsg>("CoeffIndexMsg");
      bool dummy3 = BlobStreamableFactory::instance().
        registerClass<MergedCoeffIndexMsg>("MergedCoeffIndexMsg");
      bool dummy4 = BlobStreamableFactory::instance().
        registerClass<CoeffMsg>("CoeffMsg");
      bool dummy5 = BlobStreamableFactory::instance().
        registerClass<EquationMsg>("EquationMsg");
      bool dummy6 = BlobStreamableFactory::instance().
        registerClass<SolutionMsg>("SolutionMsg");
      bool dummy7 = BlobStreamableFactory::instance().
        registerClass<ChunkDoneMsg>("ChunkDoneMsg");
    }

    // -------------------------------------------------------------------- //
    void KernelMessage::write(BlobOStream& bos) const
    {
      bos << static_cast<int32>(itsKernelIndex);
    }

    void KernelMessage::read(BlobIStream& bis)
    {
      int32 index;
      bis >> index;
      itsKernelIndex = static_cast<KernelIndex>(index);
    }

    // -------------------------------------------------------------------- //
/*
    void KernelIdMsg::passTo(KernelMessageHandler &handler) const
    {
      handler.handle(*this);
    }

    void KernelIdMsg::write(BlobOStream& bos) const
    {
      super::write(bos);
    }

    void KernelIdMsg::read(BlobIStream& bis)
    {
      super::read(bis);
    }

    const string& KernelIdMsg::classType() const
    {
      return KernelIdMsg::theirClassType;
    }
*/

    // -------------------------------------------------------------------- //
    void ProcessIdMsg::passTo(KernelMessageHandler &handler) const
    {
      handler.handle(*this);
    }

    void ProcessIdMsg::write(BlobOStream& bos) const
    {
      super::write(bos);
      bos << itsProcessId.hostname << itsProcessId.pid;
    }

    void ProcessIdMsg::read(BlobIStream& bis)
    {
      super::read(bis);
      bis >> itsProcessId.hostname >> itsProcessId.pid;
    }

    const string& ProcessIdMsg::classType() const
    {
      return ProcessIdMsg::theirClassType;
    }

    // -------------------------------------------------------------------- //
    void CoeffIndexMsg::passTo(KernelMessageHandler &handler) const
    {
      handler.handle(*this);
    }

    void CoeffIndexMsg::write(BlobOStream& bos) const
    {
      super::write(bos);
      bos << itsContents;
    }

    void CoeffIndexMsg::read(BlobIStream& bis)
    {
      super::read(bis);
      bis >> itsContents;
    }

    const string& CoeffIndexMsg::classType() const
    {
      return CoeffIndexMsg::theirClassType;
    }

    // -------------------------------------------------------------------- //
    void MergedCoeffIndexMsg::passTo(SolverMessageHandler &handler) const
    {
      handler.handle(*this);
    }

    void MergedCoeffIndexMsg::write(BlobOStream& bos) const
    {
      bos << itsContents;
    }

    void MergedCoeffIndexMsg::read(BlobIStream& bis)
    {
      bis >> itsContents;
    }

    const string& MergedCoeffIndexMsg::classType() const
    {
      return MergedCoeffIndexMsg::theirClassType;
    }

    // -------------------------------------------------------------------- //
    void CoeffMsg::passTo(KernelMessageHandler &handler) const
    {
      handler.handle(*this);
    }
  
    void CoeffMsg::write(BlobOStream& bos) const
    {
      super::write(bos);
      bos << itsContents;
    }

    void CoeffMsg::read(BlobIStream& bis)
    {
      super::read(bis);
      bis >> itsContents;
    }

    const string& CoeffMsg::classType() const
    {
      return CoeffMsg::theirClassType;
    }

    // -------------------------------------------------------------------- //
    void EquationMsg::passTo(KernelMessageHandler &handler) const
    {
      handler.handle(*this);
    }
  
    void EquationMsg::write(BlobOStream& bos) const
    {
      super::write(bos);
      bos << itsContents;
    }

    void EquationMsg::read(BlobIStream& bis)
    {
      super::read(bis);
      bis >> itsContents;
    }

    const string& EquationMsg::classType() const
    {
      return EquationMsg::theirClassType;
    }


    // -------------------------------------------------------------------- //
    void SolutionMsg::passTo(SolverMessageHandler &handler) const
    {
      handler.handle(*this);
    }
  
    void SolutionMsg::write(BlobOStream& bos) const
    {
      // super::write(bos);
      bos << itsContents;
    }

    void SolutionMsg::read(BlobIStream& bis)
    {
      // super::read(bis);
      bis >> itsContents;
    }

    const string& SolutionMsg::classType() const
    {
      return SolutionMsg::theirClassType;
    }


    // -------------------------------------------------------------------- //
    void ChunkDoneMsg::passTo(KernelMessageHandler &handler) const
    {
      handler.handle(*this);
    }

    void ChunkDoneMsg::write(BlobOStream& bos) const
    {
      super::write(bos);
    }

    void ChunkDoneMsg::read(BlobIStream& bis)
    {
      super::read(bis);
    }

    const string& ChunkDoneMsg::classType() const
    {
      return ChunkDoneMsg::theirClassType;
    }
    
  } // namespace BBS

} // namespace LOFAR
