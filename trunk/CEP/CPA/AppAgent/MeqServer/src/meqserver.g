###  meqserver.g: MeqTree server
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

include 'appagent/app_proxy.g'

# These can be uncommented (or set elsewhre prior to include) for debugging
#
#   use_suspend  := T;
#   use_nostart  := T;
#   use_valgrind := T;
#   use_valgrind_opts := [ "",
#     "--gdb-attach=yes",          # use either this...
#     "--logfile=meqserver",       # ...or this, not both
#     "--gdb-path=/usr/bin/ddd", 
#   ""];

# find path to server binary
if( has_field(lofar_software,'meq') && has_field(lofar_software.meq,'servers') )
  for( f in lofar_software.meq.servers )
    if( len(stat(f)) )
    {
      print 'Found path to meqserver binary:',f;
      if( use_valgrind )
        _meqserver_binary := f;
      else
        _meqserver_binary := f;
      break;
    }
# not found? try default
if( !is_defined('_meqserver_binary') )
{
  _meqserver_binary := 'meqserver';
  print 'Will use default meqserver binary, hope this works';
}

# define the server binary specification, using possible debug options set above
const _meqserver_binary := 
    define_octoserver(_meqserver_binary,
                      valgrind=use_valgrind,valgrind_opts=use_valgrind_opts,
                      nostart=use_nostart,suspend=F);
  

const meqserver := function (appid='MeqServer',
    server=_meqserver_binary,options="-d0 -meq:M:M:MeqServer",
    verbose=1,gui=F,ref parent_frame=F,ref widgetset=dws,
    ref self=[=],ref public=[=])
{
  # construct base app_proxy object
  if( is_fail( app_proxy(appid,server=server,options=options,
                         verbose=verbose,gui=gui,
                         parent_frame=parent_frame,widgetset=widgetset,
                         self=self,public=public) ))
    fail;
  # define meqserver-specific methods
  const public.meq := function (cmd_name,args=[=],wait_reply=F)
  {
    wider self,public;
    if( wait_reply )
    {
      rqid := self.new_requestid();
      reqname := spaste('Command.',cmd_name);
      replyname := to_lower(cmd_name ~ s/\./_/g);
      replyname := public.eventname('out_app_result',replyname,rqid);
      self.dprint(3,'sending command ',cmd_name);
      public.command(reqname,[request_id=rqid,args=args]);
      self.dprint(3,'awaiting reply ',replyname);
      await self.octoagent->[replyname];
      return $value;
    }
    else
    {
      self.dprint(3,'sending command ',cmd_name,' with no wait');
      public.command(spaste('Command.',cmd_name),[args=args]);
      return T;
    }
  }
  
  return ref public;
}

const meqnode := function (class,name,children=F,default=[=] )
{
  defrec := [ class=class,name=name ];
  if( !is_boolean(children) )
    defrec.children := children;
  if( !is_record(default) || len(default) )
    defrec.default := default;
  return defrec;
}

const meqparm := function (name,default=[=] )
{
  return meqnode('MeqParm',name,default=default);
}

const meqdomain := function (startfreq,endfreq,starttime,endtime)
{
  domain := as_double([startfreq,endfreq,starttime,endtime]);
  domain::is_datafield := T;
  return domain;
}

const meqcells := function(domain,nfreq,times,timesteps )
{
  return [ domain=domain,nfreq=as_integer(nfreq),
           times=as_double(times),
           timesteps=as_double(timesteps) ];
}

const meqrequest := function(cells,reqid=F,calcderiv=F)
{
  global _meqrequest_id;
  if( is_boolean(reqid) )
    reqid := _meqrequest_id +:= 1;
  else
    _meqrequest_id := reqid;
  return [ cells=cells,reqid=as_integer(reqid),calcderiv=calcderiv ];
}

