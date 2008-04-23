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

#ifndef LOFAR_BB_BBS_MESSAGES_H
#define LOFAR_BB_BBS_MESSAGES_H

#include <Blob/BlobStreamable.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_smartptr.h>
#include <BBSKernel/CoefficientIndex.h>

#include <scimath/Fitting/LSQFit.h>

namespace LOFAR
{
  //# Forward declarations.
  class BlobIStream;
  class BlobOStream;

namespace BBS
{
  //# Forward declarations.
  class MessageHandler;

  // Abstract base class for messages that will be exchanged between kernel
  // (prediffer) and solver. Message are handled by the MessageHandler, which
  // implements a double-dispatch mechanism for the Message class, using the
  // the Visitor pattern (Gamma, 1995).
  //
  // Messages are exchanged as Blobs using the Transport library, hence
  // derived classes must implement the BlobStreamable interface.
  class Message : public BlobStreamable
  {
  public:
    // Destructor.
    virtual ~Message() {}

    // Pass the message to the "visiting" MessageHandler. Derived classes must
    // implement this method such that it will make a callback to
    // handler.handle() passing themselves as argument.
    // \code
    //   handler.handle(*this);
    // \endcode
    virtual void passTo(MessageHandler &handler) const = 0;
    
  };

    class CoeffIndexMsg: public Message
    {
    public:
        typedef shared_ptr<CoeffIndexMsg>   Pointer;
        
        CoeffIndexMsg()
            : itsKernelId(0)
        {}

        CoeffIndexMsg(uint32 kernelId)
            : itsKernelId(kernelId)
        {}

        uint32 getKernelId() const
        { return itsKernelId; }
        
        CoefficientIndex &getContents()
        { return itsContents; }
        
        const CoefficientIndex &getContents() const
        { return itsContents; }

      //# -------- Message interface implementation --------
      virtual void passTo(MessageHandler &handler) const;

    private:
        uint32              itsKernelId;
        CoefficientIndex    itsContents;
    
        //# -------- BlobStreamable interface implementation -------- 
        static const string theirClassType;

        // Write the contents of \c *this into the blob output stream \a bos.
        virtual void write(BlobOStream& bos) const;

        // Read the contents from the blob input stream \a bis into \c *this.
        virtual void read(BlobIStream& bis);

        // Return the type of \c *this as a string.
        virtual const string& classType() const;
    };


// -------------------------------------------------------------------------- //
    class CellCoeff
    {
    public:
        CellCoeff()
            : id(0)
        {}
        
        CellCoeff(uint32 id)
            : id(id)
        {}

        uint32          id;
        vector<double>  coeff;
    };

    class CoefficientMsg: public Message
    {
    public:
        typedef shared_ptr<CoefficientMsg>  Pointer;

        CoefficientMsg()
            : itsKernelId(0)
        {}

        CoefficientMsg(uint32 kernelId)
            : itsKernelId(kernelId)
        {}

        CoefficientMsg(uint32 kernelId, size_t count)
            :   itsKernelId(kernelId),
                itsContents(count)
        {}

        uint32 getKernelId() const
        { return itsKernelId; }
        
        vector<CellCoeff> &getContents()
        { return itsContents; }
        
        const vector<CellCoeff> &getContents() const
        { return itsContents; }
        
      //# -------- Message interface implementation --------
      virtual void passTo(MessageHandler &handler) const;

    private:
        uint32              itsKernelId;
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


// -------------------------------------------------------------------------- //
    class CellEquation
    {
    public:
        CellEquation()
            : id(0)
        {}
        
        CellEquation(uint32 id)
            : id(id)
        {}

        uint32          id;
        casa::LSQFit    equation;
    };

    class EquationMsg: public Message
    {
    public:
        typedef shared_ptr<EquationMsg> Pointer;

        EquationMsg()
            : itsKernelId(0)
        {}

        EquationMsg(uint32 kernelId)
            : itsKernelId(kernelId)
        {}

        EquationMsg(uint32 kernelId, size_t count)
            :   itsKernelId(kernelId),
                itsContents(count)
        {}

        uint32 getKernelId() const
        { return itsKernelId; }
        
        vector<CellEquation> &getContents()
        { return itsContents; }
        
        const vector<CellEquation> &getContents() const
        { return itsContents; }
        
      //# -------- Message interface implementation --------
      virtual void passTo(MessageHandler &handler) const;

    private:
        uint32                  itsKernelId;
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


// -------------------------------------------------------------------------- //
    class CellSolution
    {
    public:
        CellSolution()
            : id(0)
        {}
        
        CellSolution(uint32 id)
            : id(id)
        {}

        uint32          id;
        vector<double>  coeff;
        uint32          result;
        string          resultText;
        uint32          rank;
        double          chiSqr;
        double          lmFactor;
    };

    class SolutionMsg: public Message
    {
    public:
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
      virtual void passTo(MessageHandler &handler) const;

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


// -------------------------------------------------------------------------- //
    class ChunkDoneMsg: public Message
    {
    public:
        typedef shared_ptr<ChunkDoneMsg> Pointer;

        ChunkDoneMsg()
            : itsKernelId(0)
        {}

        ChunkDoneMsg(uint32 kernelId)
            : itsKernelId(kernelId)
        {}

        uint32 getKernelId() const
        { return itsKernelId; }
        
      //# -------- Message interface implementation --------
      virtual void passTo(MessageHandler &handler) const;

    private:
        uint32  itsKernelId;
    
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
