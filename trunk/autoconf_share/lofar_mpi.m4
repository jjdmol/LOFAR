#  lofar_mpi.m4
#
#  Copyright (C) 2002
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#  $Id$


# lofar_MPI
#
# Macro to check for MPICH or ScaMPI mpi.h header file
#
AC_DEFUN(lofar_MPI,dnl
[dnl
AC_ARG_ENABLE(mpi-profiler,
	[  --enable-mpi-profiler   enable MPI profiler (default=no)],
	[mpi_profiler=yes], [mpi_profiler=no])
lofar_HEADER_MPICH([])dnl
lofar_HEADER_LAM([])dnl
lofar_HEADER_SCAMPI()dnl
[
enable_mpi=0
if test "$enable_mpich" = "yes"; then
  enable_mpi=${enable_mpi}1
fi
if test "$enable_lam" = "yes"; then
  enable_mpi=${enable_mpi}1
fi
if test "$enable_scampi" = "yes"; then
  enable_mpi=${enable_mpi}1
fi
if test $enable_mpi -gt 1; then
]
    AC_MSG_ERROR([Cannot use more than one MPI implementation.])
[fi
if test $enable_mpi -eq 1; then]
AC_DEFINE(HAVE_MPI,dnl
	1, [Define if we have an MPI implementation installed])dnl
[fi]
[if test "$mpi_profiler" = "yes"; then]
  [if test $enable_mpi = 0; then]
    AC_MSG_ERROR([Cannot enable MPI profiler without enabling MPI])
  [fi
   LIBS="$LIBS /opt/scali/contrib/mpe/lib/liblmpe.a /opt/scali/contrib/mpe/lib/libmpe.a";
   ]
   AC_SUBST(LIBS)
   AC_DEFINE(HAVE_MPI_PROFILER,dnl
	     1, [Define if MPI profiler should be enabled])
[fi]
])
#
#
# lofar_HEADER_MPICH([VERSION])
#
# Macro to check for MPICH mpi.h header
# -------------------------------------------------
#
AC_DEFUN(lofar_HEADER_MPICH,
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], define(MPICH_VERSION,[1.2.3]), define(MPICH_VERSION,$1))
AC_ARG_WITH(mpich,
	[  --with-mpich[=PFX]      prefix where MPICH is installed (default=/usr/local/mpich-]MPICH_VERSION[)],
	[mpich_prefix="$withval"],
	[mpich_prefix="no"])
[
if test "$mpich_prefix" = "no" ; then
  enable_mpich=no
else
  if test "$mpich_prefix" = "yes"; then
    mpich_prefix=/usr/local/mpich-]MPICH_VERSION
[
  fi
  enable_mpich=yes
]
dnl
AC_CHECK_FILE([$mpich_prefix/include/mpi.h],
	[lofar_cv_header_mpich=yes],
	[lofar_cv_header_mpich=no])
[
	if test $lofar_cv_header_mpich = yes ; then

		MPICH_CC="$mpich_prefix/bin/mpicc"
		MPICH_CXX="$mpich_prefix/bin/mpiCC"

		CC="$MPICH_CC"
		CXX="$MPICH_CXX"
]
AC_SUBST(CC)dnl
AC_SUBST(CXX)dnl
AC_DEFINE(HAVE_MPICH,dnl
	1, [Define if MPICH is installed])dnl
[
	else]
AC_MSG_ERROR([Could not find MPICH in $mpich_prefix])
[		enable_mpich=no
	fi
fi]
])
#
#
# lofar_HEADER_LAM([VERSION])
#
# Macro to check for LAM mpi.h header
# -------------------------------------------------
#
AC_DEFUN(lofar_HEADER_LAM,
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], define(LAM_VERSION,[6.5.6]), define(LAM_VERSION,$1))
AC_ARG_WITH(lam,
	[  --with-lam[=PFX]        prefix where LAM is installed (default=/usr/local/lam-]LAM_VERSION[)],
	[lam_prefix="$withval"],
	[lam_prefix="no"])
[
if test "$lam_prefix" = "no" ; then
  enable_lam=no
else
  if test "$lam_prefix" = "yes"; then
    lam_prefix=/usr/local/lam-]LAM_VERSION
[
  fi
  enable_lam=yes
]
dnl
AC_CHECK_FILE([$lam_prefix/include/mpi.h],
	[lofar_cv_header_lam=yes],
	[lofar_cv_header_lam=no])
[
	if test $lofar_cv_header_lam = yes ; then

		LAM_CC="$lam_prefix/bin/mpicc"
		LAM_CXX="$lam_prefix/bin/mpiCC"

		CC="$LAM_CC"
		CXX="$LAM_CXX"
]
AC_SUBST(CC)dnl
AC_SUBST(CXX)dnl
AC_DEFINE(HAVE_LAM,dnl
	1, [Define if LAM is installed])dnl
[
	else]
AC_MSG_ERROR([Could not find LAM in $lam_prefix])
[		enable_lam=no
	fi
fi]
])
#
#
# lofar_HEADER_SCAMPI([DEFAULT_PREFIX])
#
# Macro to check for ScaMPI mpi.h header
# -------------------------------------------------
#
AC_DEFUN(lofar_HEADER_SCAMPI,
[dnl
AC_PREREQ(2.13)dnl
ifelse($1, [], define(SCAMPI_DEFAULT_PREFIX,[/opt/scali]), define(SCAMPI_DEFAULT_PREFIX,$1))
AC_ARG_WITH(scampi,
	[  --with-scampi[=PFX]     prefix where ScaMPI is installed (default=]SCAMPI_DEFAULT_PREFIX[)],
	[scampi_prefix="$withval"],
	[scampi_prefix="no"])
[
if test "$scampi_prefix" = "no"; then
  enable_scampi=no
else
  if test "$scampi_prefix" = "yes"; then
    scampi_prefix=]SCAMPI_DEFAULT_PREFIX
[
  fi
  enable_scampi=yes
]
dnl
AC_CHECK_FILE([$scampi_prefix/include/mpi.h],
	[lofar_cv_header_scampi=yes],
	[lofar_cv_header_scampi=no])
[
	if test $lofar_cv_header_scampi = yes ; then

		SCAMPI_CPPFLAGS="-I$scampi_prefix/include"
		SCAMPI_LDFLAGS="-L$scampi_prefix/lib"
		SCAMPI_LIBS="-lmpi"

		CPPFLAGS="$CPPFLAGS $SCAMPI_CPPFLAGS"
		LDFLAGS="$LDFLAGS $SCAMPI_LDFLAGS"
		LIBS="$LIBS $SCAMPI_LIBS"
]
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(LIBS)
AC_DEFINE(HAVE_SCAMPI,dnl
	1, [Define if ScaMPI is installed])dnl
[
	else]
AC_MSG_ERROR([Could not find ScaMPI in $scampi_prefix])
	  [enable_scampi=no
	fi
fi]
])
