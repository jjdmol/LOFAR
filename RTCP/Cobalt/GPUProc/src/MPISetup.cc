//# MPISetup.cc: Controls MPI setup/teardown
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

#include "MPISetup.h"

#include <Common/LofarLogger.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <cstdlib>
#include <iostream>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

using namespace std;
using boost::format;

namespace LOFAR {
  namespace Cobalt {
    MPISetup::MPISetup()
    :
      /*
       * Extract rank/size from environment, because we need
       * to fork during initialisation, which we want to do
       * BEFORE calling MPI_Init_thread. Once MPI is initialised,
       * forking can lead to crashes.
       */
      rank(MPI_rank_from_environment()),
      size(MPI_size_from_environment())
    {
#ifdef HAVE_LOG4CPLUS
      // Set ${MPIRANK}, which is used by our log_prop file.
      if (setenv("MPIRANK", str(format("%02d") % rank).c_str(), 1) < 0)
      {
        perror("error setting MPIRANK");
        exit(1);
      }
#endif
    };

    void MPISetup::init(int argc, char **argv)
    {
#ifdef HAVE_MPI
      // Initialise and query MPI
      int provided_mpi_thread_support;

      LOG_INFO("----- Initialising MPI");

      if (MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided_mpi_thread_support) != MPI_SUCCESS) {
        LOG_FATAL("MPI_Init_thread failed");
        exit(1);
      }

      // Verify the rank/size settings we assumed earlier
      int real_rank;
      int real_size;

      MPI_Comm_rank(MPI_COMM_WORLD, &real_rank);
      MPI_Comm_size(MPI_COMM_WORLD, &real_size);

      ASSERT(rank == real_rank);
      ASSERT(size == real_size);
#else
      (void)argc;
      (void)argv;
#endif
    }

    void MPISetup::finalise()
    {
#ifdef HAVE_MPI
      MPI_Finalize();
#endif
    }

    int MPISetup::MPI_rank_from_environment() const
    {
      int rank = 0;

#ifdef HAVE_MPI
      const char *rankstr;

      // OpenMPI rank
      if ((rankstr = getenv("OMPI_COMM_WORLD_RANK")) != NULL)
        rank = boost::lexical_cast<int>(rankstr);

      // MVAPICH2 rank
      if ((rankstr = getenv("MV2_COMM_WORLD_RANK")) != NULL)
        rank = boost::lexical_cast<int>(rankstr);
#endif

      return rank;
    }

    int MPISetup::MPI_size_from_environment() const
    {
      int size = 1;

#ifdef HAVE_MPI
      const char *sizestr;

      // OpenMPI size
      if ((sizestr = getenv("OMPI_COMM_WORLD_SIZE")) != NULL)
        size = boost::lexical_cast<int>(sizestr);

      // MVAPICH2 size
      if ((sizestr = getenv("MV2_COMM_WORLD_SIZE")) != NULL)
        size = boost::lexical_cast<int>(sizestr);
#endif

      return size;
    }
  }
}

