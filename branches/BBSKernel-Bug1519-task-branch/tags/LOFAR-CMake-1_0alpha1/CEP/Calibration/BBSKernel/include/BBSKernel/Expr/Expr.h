//# Expr.h: The base class of an expression.
//#
//# Copyright (C) 2002
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

#ifndef EXPR_EXPR_H
#define EXPR_EXPR_H

// \file
// The base class of an expression

//# Includes
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/Result.h>
#include <BBSKernel/Expr/ResultVec.h>
#include <vector>


namespace LOFAR
{
namespace BBS
{

//# Forward Declarations.
class Expr;

// \ingroup Expr
// @{


// This class is the (abstract) base class for an expression. The classes
// derived from it implement an expression which must be evaluated for
// a given request.
//
// There are a few virtual methods which must or can be implemented in derived
// classes. They are:
// <ul>
//  <li> \b getResult calculates the main value and the
//       perturbed values of the solvable parameters.
//       The default implementation uses getResultValue for all of them.
//       Sometimes a derived class can do it in a better way by reusing
//       intermediate results. In that case it might be better implement
//       to implement getResult in the derived class.
//  <li> \b getResultVec returns the result as a vector. Only in very
//       exceptional cases it needs to be implemented in a derived class.
//  <li> getResultValue calculates the value of the expression represented
//       by the derived class. It must be implemented if <tt>getResult</tt>
//       is not implemented.
// </ul>
// So in short: either <tt>getResultValue</tt> or <tt>getResult</tt> must be
// implemented in a derived class. Usually the simpler <tt>getResultValue</tt>
// will do.

class ExprRep
{
public:
  // The default constructor.
  ExprRep();

  virtual ~ExprRep();

  // Link to this node (incrementing the refcount).
  void link()
    { itsCount++; }

  // Link to this node (decrementing the refcount).
  // The object is deleted if the refcount gets zero.
  static void unlink (ExprRep* rep)
    { if (rep != 0  &&  --rep->itsCount == 0) delete rep; }

  // Increment number of parents.
  void incrNParents()
  {
      itsNParents++;
  }

  // Decrement the number of parents.
  void decrNParents()
  {
      itsNParents--;
  }

  // Get the current number of parents
  int getParentCount() const
  {
      return itsNParents;
  }

  size_t getChildCount() const
  {
      return itsChildren.size();
  }

  // Recursively set the lowest and highest level of the node as used
  // in the tree.
  // Return the number of levels used so far.
  int setLevel (int level);

  // Clear the done flag recursively.
  void clearDone();

  // At which level is the node done?
  int levelDone() const
    { return itsLevelDone; }

  // Get the nodes at the given level.
  // All nodes or only nodes with multiple parents can be retrieved.
  // It is used to find the nodes with results to be cached.
  void getCachingNodes (std::vector<ExprRep*>& nodes,
            int level, bool all=false);

  // Get the single result of the expression for the given domain.
  // The return value is a reference to the true result. This can either
  // be a reference to the cached value (if the object maintains a cache)
  // or to the result object in the parameter list (if no cache).
  // If the result is stored in the cache, it is done in a thread-safe way.
  // Note that a cache is used if the expression has multiple parents.
  const Result& getResultSynced (const Request& request,
                    Result& result)
    { return itsReqId == request.getId()  ?
    *itsResult : calcResult(request,result); }

  // Get the actual result.
  // The default implementation throw an exception.
  virtual Result getResult (const Request&);

  // Get the multi result of the expression for the given domain.
  // The default implementation calls getResult.
  const ResultVec& getResultVecSynced (const Request& request,
                      ResultVec& result)
    { return itsReqId == request.getId()  ?
    *itsResVec : calcResultVec(request,result); }

  virtual ResultVec getResultVec (const Request&);
  // </group>

  // Precalculate the result and store it in the cache.
  virtual void precalculate (const Request&);


protected:
  // Add a child to this node.
  // It also increases NParents in the child.
  void addChild(Expr child);
  void removeChild(Expr child);
  Expr getChild(size_t index);

private:
  // Forbid copy and assignment.
  ExprRep (const ExprRep&);
  ExprRep& operator= (const ExprRep&);

  // Calculate the actual result in a cache thread-safe way.
  // <group>
  const Result& calcResult (const Request&, Result&);
  const ResultVec& calcResultVec (const Request&, ResultVec&,
                     bool useCache=false);
  // </group>

  // Let a derived class calculate the resulting value.
  virtual Matrix getResultValue (const Request &,
    const std::vector<const Matrix*>&);


  int           itsCount;      //# Reference count
  int           itsMinLevel;   //# Minimum level of node as used in tree.
  int           itsMaxLevel;   //# Maximum level of node as used in tree.
  int           itsLevelDone;  //# Level the node is handled by some parent.
  Result*    itsResult;     //# Possibly cached result
  ResultVec* itsResVec;     //# Possibly cached result vector
  std::vector<Expr> itsChildren;   //# All children

protected:
  int           itsNParents;   //# Nr of parents
  RequestId  itsReqId;      //# Request-id of cached result.
};



class Expr
{
friend class ExprRep;

public:
  // Construct from a rep object.
  // It takes over the pointer, so it takes care of deleting the object.
  Expr (ExprRep* expr = 0)
    : itsRep(expr)
    { if (itsRep) itsRep->link(); }

  // Copy constructor (reference semantics).
  Expr (const Expr&);

  ~Expr()
    { ExprRep::unlink (itsRep); }

  // Assignment (reference semantics).
  Expr& operator= (const Expr&);

  // Is the node empty?
  bool isNull() const
    { return itsRep == 0; }

  // A Expr is essentially a reference counted shared
  // pointer. Two Expr's can be considered equal if both
  // reference the same underlying ExprRep.
  bool operator ==(const Expr &other)
  {
    return (itsRep == other.itsRep);
  }

  ExprRep *getPtr()
  { return itsRep; }
  const ExprRep *getPtr() const
  { return itsRep; }

  //# -- DELEGATED METHODS --
  // Get the current number of parents
  int getParentCount() const
  {
      return itsRep->getParentCount();
  }

  // Get the current number of children
  size_t getChildCount() const
  {
      return itsRep->getChildCount();
  }

  // Recursively set the lowest and highest level of the node as used
  // in the tree.
  // Return the number of levels used so far.
  int setLevel (int level)
    { return itsRep->setLevel (level); }

  // Clear the done flag recursively.
  void clearDone()
    { itsRep->clearDone(); }

  // At which level is the node done?
  int levelDone() const
    { return itsRep->levelDone(); }

  // Get the nodes at the given level.
  // By default only nodes with multiple parents are retrieved.
  // It is used to find the nodes with results to be cached.
  void getCachingNodes (std::vector<ExprRep*>& nodes,
               int level, bool all=false)
    { itsRep->getCachingNodes (nodes, level, all); }

  // Precalculate the result and store it in the cache.
  void precalculate (const Request& request)
    { itsRep->precalculate (request); }

  // Get the result of the expression for the given domain.
  // getResult will throw an exception if the node has a multi result.
  // getResultVec will always succeed.
  // <group>
  const Result getResult (const Request& request)
    { return itsRep->getResult (request); }
  const Result& getResultSynced (const Request& request,
                    Result& result)
    { return itsRep->getResultSynced (request, result); }
  const ResultVec getResultVec (const Request& request)
    { return itsRep->getResultVec (request); }
  const ResultVec& getResultVecSynced (const Request& request,
                      ResultVec& result)
    { return itsRep->getResultVecSynced (request, result); }
  // </group>


protected:
  ExprRep* itsRep;
};

class ExprToComplex: public ExprRep
{
public:
  ExprToComplex (const Expr& real, const Expr& imag);

  virtual ~ExprToComplex();

  virtual Result getResult (const Request&);

private:

  Expr itsReal;
  Expr itsImag;
};

class ExprAPToComplex: public ExprRep
{
public:
  ExprAPToComplex (const Expr& ampl, const Expr& phase);

  virtual ~ExprAPToComplex();

  virtual Result getResult (const Request&);

private:

  Expr itsAmpl;
  Expr itsPhase;
};

class ExprPhaseToComplex: public ExprRep
{
public:
    ExprPhaseToComplex(const Expr &phase);
    virtual ~ExprPhaseToComplex();

    virtual Result getResult(const Request &request);

private:

    Expr itsPhase;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
