# __init__.py: Module initialization file.
#
# Copyright (C) 2015
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: __init__.py 1568 2015-09-18 15:21:11Z loose $

"""
Module initialization file.
"""

import os

def isProductionEnvironment():
    '''check if the program is running in a lofar producution environment'''
    return os.environ.get('LOFARENV', '') == 'PRODUCTION'

def isTestEnvironment():
    '''check if the program is running in a lofar test environment'''
    return os.environ.get('LOFARENV', '') == 'TEST'

def isDevelopmentEnvironment():
    '''check if the program is running in a lofar development (not production or test) environment'''
    return not (isProductionEnvironment() or isTestEnvironment())
