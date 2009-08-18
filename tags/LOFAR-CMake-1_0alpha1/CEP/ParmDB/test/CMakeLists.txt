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

include(LofarCTest)

lofar_add_test(tAxis tAxis.cc)
lofar_add_test(tAxisMapping tAxisMapping.cc)
lofar_add_test(tBox tBox.cc)
lofar_add_test(tGrid tGrid.cc)
lofar_add_test(tParmValue tParmValue.cc)
lofar_add_test(tParmDBCasa tParmDBCasa.cc)
lofar_add_test(tParmSet tParmSet.cc)
lofar_add_test(tParmCache tParmCache.cc)
lofar_add_test(tParm tParm.cc)
lofar_add_test(tParmFacade tParmFacade.cc DEPENDS parmdb_main)
lofar_add_test(tSourceDBCasa tSourceDBCasa.cc)
lofar_add_test(tTimeAxis tTimeAxis.cc)
lofar_add_test(tParmFacadeDistr DEPENDS parmdb_main)
lofar_add_test(tparmdb DEPENDS parmdb_main)
lofar_add_test(tmakesourcedb DEPENDS makesourcedb parmdb_main)
lofar_add_test(tsetupparmdb)
lofar_add_test(tsetupsourcedb)

