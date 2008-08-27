/// @file
/// @brief A WorkerProxy to handle BBSKernel solver commands.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Ger van Diepen (diepen AT astron nl)
///
//# $Id$

#ifndef LOFAR_MWCONTROL_SOLVERBBS_H
#define LOFAR_MWCONTROL_SOLVERBBS_H

#include <MWControl/SolverProxy.h>

//# Forward Declarations.
namespace LOFAR { namespace BBS {
  class Solver;
}}


namespace LOFAR { namespace CEP {

  /// @ingroup mwcontrol
  /// @brief A WorkerProxy to handle BBSKernel solver commands.

  /// This class handles the commands the WorkerControl has received.
  /// The first command is a call of the \a setInitInfo function.
  /// Thereafter \a doProcess is called which reads the message data from
  /// the blob and calls the correct BBS Solver function.
  ///
  /// Note that a similar class is made as a test class, which only prints the
  /// command. The \a create function registered in the WorkerFactory determines
  /// which proxy solver object is actually used.

  class SolverBBS: public SolverProxy
  {
  public:
    SolverBBS();

    ~SolverBBS();

    /// Create a new object (to be registered in WorkerFactory).
    static WorkerProxy::ShPtr create();

    /// Set the initial Solver info.
    virtual void setInitInfo (const ParameterSet&, const std::string&);

    /// Process the given operation. The associated data is read from the blob.
    /// An optional result can be written into the output blob.
    /// @todo Currently only one streamId is supported.
    /// A map<streamId, Prediffer*> should be used and a new Prediffer
    /// created for a new streamId.
    virtual int process (int operation, int streamId,
			 LOFAR::BlobIStream& in,
			 LOFAR::BlobOStream& out);

  private:
    LOFAR::BBS::Solver* itsSolver;
  };

}} /// end namespaces

#endif
