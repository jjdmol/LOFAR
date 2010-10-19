#  $Id$
#
#  Copyright (C) 2008-2009
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

include(LofarPackageVersion)

# Extra preprocessor definitions
add_definitions(-DUSES_NEW_IOSTREAMS -DLOFARLOGGER_SUBPACKAGE="PVSS")

# Add current source directory to -I path. This is needed because GCFPVSS uses
# angle brackets for internal header files, instead of quotes.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

lofar_add_library(gcfpvss
  Package__Version.cc
  GCF_PValue.cc
  GCF_PVChar.cc
  GCF_PVBool.cc
  GCF_PVDateTime.cc
  GCF_PVDouble.cc
  GCF_PVString.cc
  GCF_PVInteger.cc
  GCF_PVUnsigned.cc
  GCF_PVDynArr.cc
  GCF_PVBlob.cc
  GSA_SCADAHandler.cc
  GSA_PvssApi.cc
  GSA_Resources.cc
  PVSSinfo.cc
  PVSSservice.cc)

lofar_add_bin_program(versiongcfpvss versiongcfpvss.cc)
