//# Messages.h:
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

#ifndef LOFAR_BBSCONTROL_MESSAGES_H
#define LOFAR_BBSCONTROL_MESSAGES_H

#include <Blob/BlobStreamable.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_smartptr.h>
#include <BBSKernel/SolverInterfaceTypes.h>

namespace LOFAR
{
  //# Forward declarations.
  class BlobIStream;
  class BlobOStream;

  namespace BBS
  {
    //# Forward declarations.
    class KernelMessageHandler;
    class SolverMessageHandler;

    //## --------  A b s t r a c t   b a s e   c l a s s e s   -------- ##//

    // Abstract base classes for kernel and solver messages that will be
    // exchanged between kernel (prediffer) and solver. Message are handled by
    // the MessageHandler, which implements a double-dispatch mechanism for
    // the Message class, using the the Visitor pattern (Gamma, 1995).
    //
    // Messages are exchanged as Blobs using the Transport library, hence
    // derived classes must implement the BlobStreamable interface.

    // @{

    class KernelMessage : public BlobStreamable
    {
    public:
      // Destructor.
      virtual ~KernelMessage() {}

      // Pass the message to the "visiting" KernelMessageHandler. Derived
      // classes must implement this method such that it will make a
      // callback to handler.handle() passing themselves as argument.
      // \code
      //   handler.handle(*this);
      // \endcode
      virtual void passTo(KernelMessageHandler &handler) const = 0;

      // Return the message type as a string.
      string type() const { return classType(); }

      // Return the kernel-id of the kernel that sent the message.
      uint32 getKernelId() const
      { return itsKernelId; }

    protected:
      // Construct a KernelMessage object.
      KernelMessage(uint32 id = 0) : itsKernelId(id) {}

      //# -------- BlobStreamable interface implementation -------- 

      // Write the contents of \c *this into the blob output stream \a bos.
      virtual void write(BlobOStream& bos) const;

      // Read the contents from the blob input stream \a bis into \c *this.
      virtual void read(BlobIStream& bis);

    private:
      // Kernel-id of the kernel that sent the message.
      uint32      itsKernelId;
    };


    class SolverMessage : public BlobStreamable
    {
    public:
      // Destructor.
      virtual ~SolverMessage() {}

      // Pass the message to the "visiting" SolverMessageHandler. Derived
      // classes must implement this method such that it will make a
      // callback to handler.handle() passing themselves as argument.
      // \code
      //   handler.handle(*this);
      // \endcode
      virtual void passTo(SolverMessageHandler &handler) const = 0;
#if 0
      // Return the message type as a string.
      string type() const { return classType(); }

    protected:
      //# -------- BlobStreamable interface implementation -------- 

      // Write the contents of \c *this into the blob output stream \a bos.
      virtual void write(BlobOStream& bos) const;

      // Read the contents from the blob input stream \a bis into \c *this.
      virtual void read(BlobIStream& bis);
#endif
    };

    // @}


    //## --------  C o n c r e t e   c l a s s e s  -------- ##//

    // Message for passing coefficient indices between kernel and solver.
    class CoeffIndexMsg: public KernelMessage
    {
    public:
      typedef KernelMessage super;
      typedef shared_ptr<CoeffIndexMsg>   Pointer;
        
      CoeffIndexMsg()
        : KernelMessage()
      {}

      CoeffIndexMsg(uint32 kernelId)
        : KernelMessage(kernelId)
      {}

      CoeffIndex &getContents()
      { return itsContents; }
        
      const CoeffIndex &getContents() const
      { return itsContents; }

      //# -------- Message interface implementation --------
      virtual void passTo(KernelMessageHandler &handler) const;

    private:
      CoeffIndex  itsContents;
    
      //# -------- BlobStreamable interface implementation -------- 
      static const string theirClassType;

      // Write the contents of \c *this into the blob output stream \a bos.
      virtual void write(BlobOStream& bos) const;

      // Read the contents from the blob input stream \a bis into \c *this.
      virtual void read(BlobIStream& bis);

      // Return the type of \c *this as a string.
      virtual const string& classType() const;
    };


    // Message for passing coefficients from kernel to solver.
    class CoeffMsg: public KernelMessage
    {
    public:
      typedef KernelMessage super;
      typedef shared_ptr<CoeffMsg>  Pointer;

      CoeffMsg()
        : KernelMessage()
      {}

      CoeffMsg(uint32 kernelId)
        : KernelMessage(kernelId)
      {}

      CoeffMsg(uint32 kernelId, size_t count)
        :   KernelMessage(kernelId),
            itsContents(count)
      {}

      vector<CellCoeff> &getContents()
      { return itsContents; }
        
      const vector<CellCoeff> &getContents() const
      { return itsContents; }
        
      //# -------- Message interface implementation --------
      virtual void passTo(KernelMessageHandler &handler) const;

    private:
      vector<CellCoeff>   itsContents;
    
      //# -------- BlobStreamable interface implementation -------- 
      static const string theirClassType;

      // Write the contents of \c *this into the blob output stream \a bos.
      virtual void write(BlobOStream& bos) const;

      // Read the contents from the blob input stream \a bis into \c *this.
      virtual void read(BlobIStream& bis);

      // Return the type of \c *this as a string.
      virtual const string& classType() const;
    };

    // BlobStream I/O
    BlobIStream &operator>>(BlobIStream &in, CellCoeff &obj);    
    BlobOStream &operator<<(BlobOStream &out, const CellCoeff &obj);


    // Message for passing equations from kernel to solver.
    class EquationMsg: public KernelMessage
    {
    public:
      typedef KernelMessage super;
      typedef shared_ptr<EquationMsg> Pointer;

      EquationMsg()
        : KernelMessage()
      {}

      EquationMsg(uint32 kernelId)
        : KernelMessage(kernelId)
      {}

      EquationMsg(uint32 kernelId, size_t count)
        :   KernelMessage(kernelId),
            itsContents(count)
      {}

      vector<CellEquation> &getContents()
      { return itsContents; }
        
      const vector<CellEquation> &getContents() const
      { return itsContents; }
        
      //# -------- Message interface implementation --------
      virtual void passTo(KernelMessageHandler &handler) const;

    private:
      vector<CellEquation>    itsContents;
    
      //# -------- BlobStreamable interface implementation -------- 
      static const string theirClassType;

      // Write the contents of \c *this into the blob output stream \a bos.
      virtual void write(BlobOStream& bos) const;

      // Read the contents from the blob input stream \a bis into \c *this.
      virtual void read(BlobIStream& bis);

      // Return the type of \c *this as a string.
      virtual const string& classType() const;
    };

    // BlobStream I/O
    BlobIStream &operator>>(BlobIStream &in, CellEquation &obj);    
    BlobOStream &operator<<(BlobOStream &out, const CellEquation &obj);


    // Message for passing a solution from solver to kernel.
    class SolutionMsg: public SolverMessage
    {
    public:
      typedef SolverMessage super;
      typedef shared_ptr<SolutionMsg> Pointer;

      SolutionMsg()
      {}

      SolutionMsg(size_t count)
        : itsContents(count)
      {}

      vector<CellSolution> &getContents()
      { return itsContents; }
        
      const vector<CellSolution> &getContents() const
      { return itsContents; }
        
      //# -------- Message interface implementation --------
      virtual void passTo(SolverMessageHandler &handler) const;

    private:
      vector<CellSolution>    itsContents;
    
      //# -------- BlobStreamable interface implementation -------- 
      static const string theirClassType;

      // Write the contents of \c *this into the blob output stream \a bos.
      virtual void write(BlobOStream& bos) const;

      // Read the contents from the blob input stream \a bis into \c *this.
      virtual void read(BlobIStream& bis);

      // Return the type of \c *this as a string.
      virtual const string& classType() const;
    };

    // BlobStream I/O
    BlobIStream &operator>>(BlobIStream &in, CellSolution &obj);    
    BlobOStream &operator<<(BlobOStream &out, const CellSolution &obj);


    // Message for indicating to the solver control process that a data chunk
    // is done.
    class ChunkDoneMsg: public KernelMessage
    {
    public:
      typedef KernelMessage super;
      typedef shared_ptr<ChunkDoneMsg> Pointer;

      ChunkDoneMsg()
        : KernelMessage()
      {}

      ChunkDoneMsg(uint32 kernelId)
        : KernelMessage(kernelId)
      {}

      //# -------- Message interface implementation --------
      virtual void passTo(KernelMessageHandler &handler) const;

    private:
      //# -------- BlobStreamable interface implementation -------- 
      static const string theirClassType;

      // Write the contents of \c *this into the blob output stream \a bos.
      virtual void write(BlobOStream& bos) const;

      // Read the contents from the blob input stream \a bis into \c *this.
      virtual void read(BlobIStream& bis);

      // Return the type of \c *this as a string.
      virtual const string& classType() const;
    };

  } //# namespace BBS

} //# namespace LOFAR

#endif
