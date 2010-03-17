#
#  setenv.csh: Source this file in order to "compile" the LaTeX file(s)
#              in this directory.
#
#  Copyright (C) 2007
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
#

# We would like to use "set docroot=`(cd ..; pwd)`", but that won't work if
# the user has defined the "cwdcmd" alias. Hence this workaround.
set docroot=`echo ${PWD} | sed 's,\(.*\)/[^/]*$,\1,'`
set texinputs="${docroot}/tex/inputs//"
set bibinputs="${docroot}/tex/bib"

if ( ${?TEXINPUTS} ) then
  if ( "${TEXINPUTS}" !~ *"${texinputs}"* ) then
    setenv TEXINPUTS "${texinputs}":"${TEXINPUTS}"
  endif
else
  setenv TEXINPUTS "${texinputs}:"
endif

if ( ${?BIBINPUTS} ) then
  if ( "${BIBINPUTS}" !~ *"${bibinputs}"* ) then
    setenv BIBINPUTS "${bibinputs}":"${BIBINPUTS}"
  endif
else
  setenv BIBINPUTS "${bibinputs}"
endif
