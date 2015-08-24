# - Try to find libxml++: A library to parse XML DOM trees
# Variables used by this module:
#  ENV{PKG_CONFIG_PATH}  - a colon-separated list of directories to search for .pc files
# Variables defined by this module:
#  LIBXMLXX_FOUND        - system has LIBXMLXX
#  LIBXMLXX_INCLUDE_DIRS - the LIBXMLXX include directories
#  LIBXMLXX_LIBRARIES    - the LIBXMLXX libraries

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

if(NOT LIBXMLXX_FOUND)
  include(FindPkgConfig)

  # pkg_search_module puts LIBXMLXX_FOUND, LIBXMLXX_INCLUDE_DIRS, LIBXMLXX_LIBRARIES into the cache
  pkg_search_module(LIBXMLXX libxml++-2.8 libxml++-2.7 libxml++-2.6 libxml++-2.5)

  # Remove LIBXMLXX_FOUND from cache, to stay compatible with fphsa (LofarFindPackage depends on this)
  unset(LIBXMLXX_FOUND CACHE)

  # run fphsa to:
  #   1. Report "Found" or "Could NOT find" message,
  #   2. Set LIBXMLXX_FOUND (not cached!)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(LibXMLxx DEFAULT_MSG
    LIBXMLXX_LIBRARIES LIBXMLXX_INCLUDE_DIRS)

endif(NOT LIBXMLXX_FOUND)
