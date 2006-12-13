//#  CommandQueueTransactors.h: Transaction functors for the BBS command queue
//#
//#  Copyright (C) 2002-2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_BBSCONTROL_COMMANDQUEUETRANSACTORS_H
#define LOFAR_BBSCONTROL_COMMANDQUEUETRANSACTORS_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// Transaction functors for the BBS command queue

//# Includes
#if defined(HAVE_PQXX)
# include <pqxx/transactor>
#else
# error libpqxx, the C++ API to PostgreSQL, is required
#endif

namespace LOFAR
{
  namespace BBS
  {
    //# Forward Declarations
    class BBSSingleStep;

    // \addtogroup BBSControl
    // @{

    // Functor class for adding a BBSSingleStep to the command queue of the
    // blackboard database.
    class AddStep : public pqxx::transactor<>
    {
    public:
      // Construct the functor, using the BBSSingleStep \a step as argument.
      explicit AddStep(const BBSSingleStep& step);

      // This method will be invoked by the perform() method of your
      // pqxx::connection class.
      // \note argument_type is defined in std::unary_function, which is the
      // parent class of pqxx::transactor<>.
      void operator()(argument_type& t);

//       // Pass query results back. This method will be called when the
//       // transaction succeeded.
//       void on_commit();
    };

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
