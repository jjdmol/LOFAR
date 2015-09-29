# - Try to find SLURM: the Simple Linux Resource Manager
# Variables used by this module:
#  SLURM_ROOT_DIR     - SLURM root directory
# Variables defined by this module:
#  SLURM_FOUND               - system has SLURM
#  SLURM_SRUN_EXECUTABLE     - the full path of srun
#  SLURM_SALLOC_EXECUTABLE   - the full path of salloc
#  SLURM_SQUEUE_EXECUTABLE   - the full path of squeue
#  SLURM_SACCT_EXECUTABLE    - the full path of sacct
#  SLURM_SCONTROL_EXECUTABLE - the full path of scontrol
#  SLURM_SINFO_EXECUTABLE    - the full path of sinfo

# Copyright (C) 2015
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$

if(NOT SLURM_FOUND)
  find_program(SLURM_SRUN_EXECUTABLE srun
    HINTS ${SLURM_ROOT_DIR} PATH_SUFFIXES bin)
  find_program(SLURM_SALLOC_EXECUTABLE salloc
    HINTS ${SLURM_ROOT_DIR} PATH_SUFFIXES bin)
  find_program(SLURM_SQUEUE_EXECUTABLE squeue
    HINTS ${SLURM_ROOT_DIR} PATH_SUFFIXES bin)
  find_program(SLURM_SACCT_EXECUTABLE sacct
    HINTS ${SLURM_ROOT_DIR} PATH_SUFFIXES bin)
  find_program(SLURM_SCONTROL_EXECUTABLE scontrol
    HINTS ${SLURM_ROOT_DIR} PATH_SUFFIXES bin)
  find_program(SLURM_SINFO_EXECUTABLE sinfo
    HINTS ${SLURM_ROOT_DIR} PATH_SUFFIXES bin)

  if(SLURM_SRUN_EXECUTABLE)
    set(SLURM_FOUND TRUE)
  endif(SLURM_SRUN_EXECUTABLE)
endif(NOT SLURM_FOUND)

IF(NOT SLURM_FOUND)
  IF(SLURM_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "SLURM was not found.")
  ELSE(SLURM_FIND_REQUIRED)
    IF(NOT SLURM_FIND_QUIETLY)
      MESSAGE(STATUS "SLURM was not found.")
    ENDIF(NOT SLURM_FIND_QUIETLY)
  ENDIF(SLURM_FIND_REQUIRED)
ENDIF(NOT SLURM_FOUND)
