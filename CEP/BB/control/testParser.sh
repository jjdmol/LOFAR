#! /bin/bash
#//  testParser.sh: one line description
#//
#//  Copyright (C) 2002
#//  ASTRON (Netherlands Foundation for Research in Astronomy)
#//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#//
#//  This program is free software; you can redistribute it and/or modify
#//  it under the terms of the GNU General Public License as published by
#//  the Free Software Foundation; either version 2 of the License, or
#//  (at your option) any later version.
#//
#//  This program is distributed in the hope that it will be useful,
#//  but WITHOUT ANY WARRANTY; without even the implied warranty of
#//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#//  GNU General Public License for more details.
#//
#//  You should have received a copy of the GNU General Public License
#//  along with this program; if not, write to the Free Software
#//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#//
#//  $Id$

PARSER=`eval pwd`/read_script
DATADIR=`eval pwd`/confdata
TESTDIR=`mktemp -d test.XXXXXX`
CLEAN=0

makeScriptLst ()
{

  local dirlst=""
  if ls *.selfcal >/dev/null 2>&1
  then
    dirlst=`ls *.selfcal`
  fi
  local madelst=""
  for i in $dirlst
  do
    madelst="$madelst `basename $i .selfcal`"
  done
  echo $madelst
}

function testScript ()
{
  while [ ! -z $1 ]
  do
    mkdir $1.d
    cd $1.d
    echo "pwd `pwd`"
    ${PARSER} ../$1.selfcal >/dev/null 2>&1
    echo test result\: $?
    local scriptlst=`makeScriptLst`
    echo "list of scripts to parse: $scriptlst"
    [ "$scriptlst" != "" ] && testScript $scriptlst
    [ $CLEAN == 1 ] && rm *
    cd ..
    [ $CLEAN == 1 ] && rmdir $1.d
    shift
  done
}

#main:
echo ${TESTDIR}
cd ${TESTDIR}

#lst= collaboration competition complex deep devide empty empty.syntax hello part script script.syntax syntax
cp ${DATADIR}/*.selfcal .

lst=`makeScriptLst`

echo "parsing the following list: $lst"

[ "$lst" != "" ] && testScript $lst

cd ..

[ $CLEAN == 1 ] && rmdir ${TESTDIR}

