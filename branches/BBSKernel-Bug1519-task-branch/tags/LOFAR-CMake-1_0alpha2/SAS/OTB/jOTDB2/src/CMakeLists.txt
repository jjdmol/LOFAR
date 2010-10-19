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

set(jotdb2_LIB_SRCS
  Package__Version.cc
  nl_astron_lofar_sas_otb_jotdb2_jCommon.cc
  nl_astron_lofar_sas_otb_jotdb2_jOTDBconnection.cc
  nl_astron_lofar_sas_otb_jotdb2_jTreeMaintenance.cc
  nl_astron_lofar_sas_otb_jotdb2_jClassifConv.cc
  nl_astron_lofar_sas_otb_jotdb2_jParamTypeConv.cc
  nl_astron_lofar_sas_otb_jotdb2_jTreeStateConv.cc
  nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv.cc
  nl_astron_lofar_sas_otb_jotdb2_jUnitConv.cc
  nl_astron_lofar_sas_otb_jotdb2_jInitCPPLogger.cc
  nl_astron_lofar_sas_otb_jotdb2_jTreeValue.cc)

lofar_add_library(jotdb2 ${jotdb2_LIB_SRCS})

add_executable(versionjotdb2 versionjotdb2.cc)
target_link_libraries(versionjotdb2 jotdb2 ${LOFAR_LIBRARIES})
install(TARGETS versionjotdb2 DESTINATION bin)

