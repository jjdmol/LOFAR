#!/usr/bin/python

# Copyright (C) 2012-2015    ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.

# $Id$

from datetime import datetime, timedelta
import sys
import os


def monthRanges(min_date, max_date):
    ranges = []

    min_month_start = datetime(min_date.year, min_date.month, 1, tzinfo=min_date.tzinfo)

    month_start = min_month_start
    while month_start < max_date:
        if month_start.month < 12:
            month_end = datetime(month_start.year, month_start.month+1, 1, tzinfo=month_start.tzinfo) - timedelta(milliseconds=1)
        else:
            month_end = datetime(month_start.year+1, month_start.month-11, 1, tzinfo=month_start.tzinfo) - timedelta(milliseconds=1)

        ranges.append((month_start, month_end))

        if month_start.month < 12:
            month_start = datetime(month_start.year, month_start.month+1, 1, tzinfo=min_date.tzinfo)
        else:
            month_start = datetime(month_start.year+1, month_start.month-11, 1, tzinfo=min_date.tzinfo)

    return ranges

def totalSeconds(td_value):
    '''Return the total number of fractional seconds contained in the duration.
    For Python < 2.7 compute it, else use total_seconds() method.
    '''
    if hasattr(td_value,"total_seconds"):
        return td_value.total_seconds()

    return (td_value.microseconds + (td_value.seconds + td_value.days * 86400) * 1000000) / 1000000.0
