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
#   use_suspend  := T;
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

#------ getdomain()
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

#------ env2domains()
# Computes the envelope of two meqdomains. If the first domain is
# undefined, simply returns the second domain.
const env2domains := function (d1,d2)
{
  if( !is_dmi_type(d1,'MeqDomain') )
    return d2;
  d1[1] := min(d1[1],d2[1]);
  d1[2] := max(d1[2],d2[2]);
  d1[3] := min(d1[3],d2[3]);
  d1[4] := max(d1[4],d2[4]);
  return d1;
}

#------ envelope_domain()
# Given a number of meqdomains or meqpolcs, returns a meqdomain that is 
# an envelope of all the supplied domains.
# The following arguments can be used:
#   * meqpolcs or meqdomains
#   * records (i.e. vectors) of meqpolcs or meqdomains
const envelope_domain := function (...)
{
  dom := F;
  for( i in 1:num_args(...) )
  {
    x := nth_arg(i,...);
    if( is_record(x) && !has_field(x::,'dmi_actual_type') )
    {
      for( i in 1:len(x) )
        dom := env2domains(dom,getdomain(x[i]));
    }
    else
      dom := env2domains(dom,getdomain(x));
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
    domain := envelope_domain(polcs);
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
  cells := meqcells(domain,num_freq=2*nx,num_time=2*ny);
  request := meqrequest(cells,calc_deriv=2);
  res := mqs.meq('Node.Execute',[name='fitpolc_solver',request=request],T);
  
  if( has_field(res.result.vellsets[1],'fail') )
  {
    fl := res.result.vellsets[1]['fail'];
    print 'Solve failed: ',fl;
    fail fl[1].message;
  }
  
  # return the new polc
  return mqs.getnodestate('fitpolc_p2').polcs;
}

# eval_polc()
# Evaluates a polc over either
#   (a) a set of points -- x and y must be vectors of coordinates (in time/freq)
#   (b) a meqcells -- x must be a meqcells object
# Returns (a) vector of values conforming to x/y length 
#         (b) matrix of values conforming to cells layout
const eval_polc := function (polc,x=F,y=F,cells=F)
{
  if( is_dmi_type(cells,'MeqCells') )
  {
    # cells implies a regular grid -- compute & place grid points into xp, yp
    xp := cells.domain[1] + 
        (cells.domain[2]-cells.domain[1])*((1:cells.num_freq)-0.5)/cells.num_freq;
    yp := cells.times;
    # create composite arrays where every combination is present
    x := rep(xp,len(yp));           # [x1,...,xn,x1,...,xn,x1,...,xn,x1 ...
    y := array(0.,len(xp)*len(yp)); # [y1 ... y1,y2 ... y2,y3 ... y3,y4 ...
    for( i in 1:len(yp) )
      y[(i-1)*len(xp)+(1:len(xp))] := yp[i];
    print 'x: ',x;
    print 'y: ',y;
  }
  else
  {
    if( is_boolean(x) || is_boolean(y) )
      fail 'numeric x and y vectors must be siupplied'
    if( len(x) != len(y) )
      fail 'lengths of x and y vectors must match';
  }
  # evaluate
  x := (x-polc.freq_0)/polc.freq_scale;
  y := (y-polc.time_0)/polc.time_scale;
  res := array(0.,len(x));
  powx := 1;
  nx := shape(polc.coeff)[1];
  ny := shape(polc.coeff)[2];
  for( i in 1:nx )
  {
    powy := powx;
    for( j in 1:ny )
    {
      res +:= polc.coeff[i,j]*powy;
      powy *:= y;
    }
    powx *:= x;
  }
  # if input was a cells, reform the result into a matrix
  if( is_dmi_type(cells,'MeqCells') )
    res := array(res,cells.num_freq,len(cells.times));
  return res;
}


const fitpolcs_test := function ()
{
  global polcs,polc;
  polcs := [=];
  polcs[1] := meqpolc(array([0,1],2,1),domain=meqdomain(0,1,0,1));
  polcs[2] := meqpolc(array([2,-1],2,1),domain=meqdomain(1,2,0,1));
  polcs[3] := meqpolc(array([-2,1],2,1),domain=meqdomain(2,3,0,1));
  polc := fitpolcs(polcs,nx=4,ny=1,scale=[0,1,0,1],verbose=1);
  print polc;
}
