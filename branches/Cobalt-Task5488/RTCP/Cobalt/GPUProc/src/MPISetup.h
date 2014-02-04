//# MPISetup.h: Controls MPI setup/teardown
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

#ifndef LOFAR_GPUPROC_MPISETUP_H
#define LOFAR_GPUPROC_MPISETUP_H

namespace LOFAR
{
  namespace Cobalt
  {
    /*
     * MPISetup controls MPI setup and tear down,
     * while providing a fall back if MPI is not used.
     *
     * The rank & size are available upon construction of MPISetup,
     * but calls to init() and finalise() should be made to trigger
     * MPI_Init and MPI_Finalise, respectively.
     */
    class MPISetup
    {
    public:
      const int rank;
      const int size;

      MPISetup();

      void init(int argc, char **argv);
      void finalise();

    private:
      int MPI_rank_from_environment() const;
      int MPI_size_from_environment() const;
    };
  }
}

#endif

