# lofarlogger.sh
#
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

# Usage: source lofarlogger.sh
# then e.g.: log INFO "foo bar"
# logs e.g.: 2015-10-16 16:00:46,186 INFO foo bar

log() {
  loglevel=$1  # one of: DEBUG INFO WARNING ERROR CRITICAL
  message=$2
  ts=`date --utc '+%F %T,%3N'`  # e.g. 2015-10-16 16:00:46,186
  echo "$ts $loglevel $message" >&2
}

