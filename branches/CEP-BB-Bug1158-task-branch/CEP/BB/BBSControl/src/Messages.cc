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
    const string CoeffIndexMsg::theirClassType = "CoeffIndexMsg";
    const string CoeffMsg::theirClassType = "CoeffMsg";
    const string EquationMsg::theirClassType = "EquationMsg";
    const string SolutionMsg::theirClassType = "SolutionMsg";
    const string ChunkDoneMsg::theirClassType = "ChunkDoneMsg";

    // Register messages with the BlobStreamableFactory. Use an anonymous
    // namespace. This ensures that the variables `dummy*' get their own
    // private storage area and are only visible inside this compilation unit.
    namespace
    {
      bool dummy1 = BlobStreamableFactory::instance().
        registerClass<CoeffIndexMsg>("CoeffIndexMsg");
      bool dummy2 = BlobStreamableFactory::instance().
        registerClass<CoeffMsg>("CoeffMsg");
      bool dummy3 = BlobStreamableFactory::instance().
        registerClass<EquationMsg>("EquationMsg");
      bool dummy4 = BlobStreamableFactory::instance().
        registerClass<SolutionMsg>("SolutionMsg");
      bool dummy5 = BlobStreamableFactory::instance().
        registerClass<ChunkDoneMsg>("ChunkDoneMsg");
    }

    // -------------------------------------------------------------------- //
    void KernelMessage::write(BlobOStream& bos) const
    {
      bos << itsKernelId;
    }

    void KernelMessage::read(BlobIStream& bis)
    {
      bis >> itsKernelId;
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
