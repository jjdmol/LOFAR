###  defrecs.g: defines default init-records for known nodes
###
###  Copyright (C) 2002-2003
###  ASTRON (Netherlands Foundation for Research in Astronomy)
###  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
###
###  This program is free software; you can redistribute it and/or modify
###  it under the terms of the GNU General Public License as published by
###  the Free Software Foundation; either version 2 of the License, or
###  (at your option) any later version.
###
###  This program is distributed in the hope that it will be useful,
###  but WITHOUT ANY WARRANTY; without even the implied warranty of
###  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
###  GNU General Public License for more details.
###
###  You should have received a copy of the GNU General Public License
###  along with this program; if not, write to the Free Software
###  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
###
###  $Id$
pragma include once

# print software version
if( has_field(lofar_software,'print_versions') &&
    lofar_software.print_versions )
{
  print '$Id$';
}

_meqdefrec_map := F;

const meqdefrec := function (classname)
{
  global _meqdefrec_map;
  # generate the map at first call
  if( !is_record(_meqdefrec_map) )
  {
    # generate default record for base MeqNode class
    r := [=];
    r::description := 'abstract node class, base class for all nodes';
    r.class := '';
    r.class::description := 'class name goes here';
    r.name := '';
    r.name::description := 'node name goes here';
    r.children := '';
    r.children::description := 'list of child nodes. May be specified as a \
          string array of child names, or as an integer array of node indices. \
          May also be specified as a record, in which case each field name \
          must match a node argument name, and must contain either a node name, \
          a node index, or another init-record (for the child to be created \
          on-the-fly)';
    _meqdefrec_map := [ MeqNode=r ];
    
    # now pull in definitions from other packages
    
    include 'meq/defrecs_MEQ.g'
    include 'meq/defrecs_MeqServer.g'
  }
  # lookup in map
  if( has_field(_meqdefrec_map,classname) )
    return _meqdefrec_map[classname];
  else # return default
    return _meqdefrec_map.MeqNode;
}
