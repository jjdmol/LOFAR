### meptools.g: tools for manipulating MEPs
###
### Copyright (C) 2002
### ASTRON (Netherlands Foundation for Research in Astronomy)
### P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
###
### This program is free software; you can redistribute it and/or modify
### it under the terms of the GNU General Public License as published by
### the Free Software Foundation; either version 2 of the License, or
### (at your option) any later version.
###
### This program is distributed in the hope that it will be useful,
### but WITHOUT ANY WARRANTY; without even the implied warranty of
### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
### GNU General Public License for more details.
###
### You should have received a copy of the GNU General Public License
### along with this program; if not, write to the Free Software
### Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
###
### $Id$

# These can be uncommented (or set elsewhre prior to include) for debugging
#
   use_suspend  := T;
#   use_nostart  := T;
#   use_valgrind := T;
   use_valgrind_opts := [ "",
#     "--gdb-attach=yes",          # use either this...
     "--logfile=meqserver",       # ...or this, not both
#     "--gdb-path=/usr/bin/ddd", 
   ""];


pragma include once

# print software version
if( has_field(lofar_software,'print_versions') &&
    lofar_software.print_versions )
{
  print '$Id$';
}

include 'meq/meqserver.g'
include 'meq/meptable.g'

# Given a meqdomain x, returns x
# Given a meqpolc x, returns domain of polc, or default ([0:1,0:1]) if
# polc does not have a domain
const getdomain := function (x)
{
  if( is_dmi_type(x,'MeqDomain') )
    return x;
  else if( is_dmi_type(x,'MeqPolc') )
  {
    if( has_field(x,'domain') )
      return x.domain;
    else
      return meqdomain(0,1,0,1);
  }
  else
    fail 'getdomain(): argument is not a meqdomain or a meqpolc';
}

const merge2domains := function (d1,d2)
{
  if( !is_dmi_type(d1,'MeqDomain') )
    return d2;
  d1[1] := min(d1[1],d2[1]);
  d1[2] := max(d1[2],d2[2]);
  d1[3] := min(d1[3],d2[3]);
  d1[4] := max(d1[4],d2[4]);
  return d1;
}

# Given a number of meqdomains or meqpolcs, returns a meqdomain containing
# all the domains
const superdomain := function (...)
{
  dom := F;
  for( i in 1:num_args(...) )
  {
    x := nth_arg(i,...);
    if( is_record(x) && !has_field(x::,'dmi_actual_type') )
    {
      for( i in 1:len(x) )
        dom := merge2domains(dom,getdomain(x[i]));
    }
    else
      dom := merge2domains(dom,getdomain(x));
  }
  return dom;
}


# Fits the give polcs with a new nx x ny polc, computed over the specified 
# domain. If no domain is specified, then a superdomain of the polcs is used.
# A polc scale may be specified as [f0,fscale,t0,tscale]. If it is not specified,
# then a scale corresponding to [0:1] over the domain is used.
const fitpolcs := function (polcs,nx=1,ny=1,domain=F,scale=F,verbose=1,gui=F)
{
  # polcs must be a vector of polcs; if it's a single polc, then make the vector
  if( is_dmi_type(polcs,'MeqPolc') )
  {
    p := [=];
    p[1] := polcs;
    polcs := p;
  }
  # compute domain of target polc
  if( is_boolean(domain) )
    domain := superdomain(polcs);
  if( verbose>0 )
  {
    print 'Merging ',len(polcs),' polcs; destination domain is ',domain;
    for( i in 1:len(polcs) )
      print '  polc ',i,': domain ',polcs[i].domain;
  }
  # create initial new polc
  if( is_boolean(scale) )
    scale := [ domain[1],domain[2]-domain[1],domain[3],domain[4]-domain[3] ];
  newpolc := meqpolc(array(0,nx,ny),domain=domain,
                    freq0=scale[1],freqsc=scale[2],
                    time0=scale[3],timesc=scale[4]);
  # start meqserver and build a tree
  global mqs;
  mqs := ref default_meqserver(verbose=verbose,gui=gui,debug=[MeqParm=5]);
  # clear the forest -- tough luck if something's there
  # TODO: think about private forests, anonymous nodes, etc.
  mqs.meq('Clear.Forest');
  
  print mqs.createnode(meqparm('fitpolc_p1',polc=polcs));
  print mqs.createnode(meqparm('fitpolc_p2',polc=newpolc,config_groups='Solvable.Parm'));
  print mqs.createnode(meqnode('MeqCondeq','fitpolc_eq',children="fitpolc_p1 fitpolc_p2"));
  
  rec := meqnode('MeqSolver','fitpolc_solver',children="fitpolc_eq");
  rec.num_steps := 5;
  rec.solvable_parm := [ by_list=meqinitstatelist() ];
  meqaddstatelist(rec.solvable_parm.by_list,"fitpolc_p2",[solvable=T]); 
  meqaddstatelist(rec.solvable_parm.by_list,"*",[solvable=F]); 
  print mqs.createnode(rec);
  
  # resolve children
  print mqs.meq('Resolve.Children',[name='fitpolc_solver']);
  
  if( verbose>1 )
  {
    for( nm in "fitpolc_p1 fitpolc_p2 fitpolc_eq" )
      print mqs.meq('Node.Publish.Results',[name=nm]);
  }
  
  # perform a fit. 
  global cells,request,res;
  # figure out an appropriate cells first
  ntimes := 2*ny;
  timestep := (domain[4]-domain[3])/ntimes;
  times := (1:(ny*2)-0.5)*timestep;
  time_steps := array(timestep,ntimes);
  cells := meqcells(domain,num_freq=2*nx,times=times,time_steps=time_steps);
  request := meqrequest(cells,calc_deriv=2);
  res := mqs.meq('Node.Execute',[name='fitpolc_solver',request=request],T);
  
  if( has_field(res.result.vellsets[1],'fail') )
  {
    fl := res.result.vellsets[1]['fail'];
    print 'Solve failed: ',fl;
    fail fl[1].message;
  }
  
  # print out the new polc
  st2 := mqs.getnodestate('fitpolc_p2');
  print st2;
  return st2;
}


const fitpolcs_test := function ()
{
  global polcs,polc;
  polcs := [=];
  polcs[1] := meqpolc(array([0,1],2,1),domain=meqdomain(0,1,0,1));
  polcs[2] := meqpolc(array([2,-1],2,1),domain=meqdomain(1,2,0,1));
  polc := fitpolcs(polcs,nx=3,ny=1,scale=[0,1,0,1],verbose=3);
  print polc;
}
