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
                      nostart=use_nostart,suspend=use_suspend);
  

const meqserver := function (appid='MeqServer',
    server=_meqserver_binary,options="-nogw -d0 -meq:M:M:MeqServer",
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
      self.dprint(5,'arguments are ',args);
      res := public.command(reqname,[request_id=rqid,args=args]);
      if( is_fail(res) )
      {
        self.dprint(3,'command sending failed');
        fail;
      }
      self.dprint(3,'awaiting reply ',replyname);
      await self.octoagent->[replyname];
      return $value;
    }
    else
    {
      self.dprint(3,'sending command ',cmd_name,' with no wait');
      self.dprint(5,'arguments are ',args);
      return public.command(spaste('Command.',cmd_name),[args=args]);
    }
  }
  # define shortcuts for common methods
  const public.getnodestate := function (node)
  {
    wider self,public;
    rec := [=];
    if( is_string(node) )
      rec.name := node;
    else if( is_integer(node) )
      rec.nodeindex := node;
    else
      fail 'node must be specified by name or index';
    return public.meq('Node.Get.State',rec,wait_reply=T);
  }
  const public.getnodelist := function ()
  {
    wider self,public;
    return public.meq('Get.Node.List',[=],wait_reply=T);
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
  domain::dmi_datafield_content_type := 'double';
  domain::dmi_actual_type  := 'MeqDomain';
  return domain;
}

const meqcells := function (domain,nfreq,times,timesteps )
{
  rec := [ domain=domain,num_freq=as_integer(nfreq),
           times=as_double(times),
           time_steps=as_double(timesteps) ];
  rec::dmi_actual_type := 'MeqCells';
  return rec;
}

const meqrequest := function (cells,reqid=F,calc_deriv=F)
{
  global _meqrequest_id;
  if( is_boolean(reqid) )
    reqid := _meqrequest_id +:= 1;
  else
    _meqrequest_id := reqid;
  rec := [ cells=cells,request_id=hiid(reqid),calc_deriv=calc_deriv ];
  rec::dmi_actual_type := 'MeqRequest';
  
  const rec.addstate := function (group,node,state)
  {
    wider rec;
    # add node_state and group subrecord
    if( !has_field(rec,'node_state') )
      rec.node_state := [=];
    if( !has_field(rec.node_state,group) )
      rec.node_state[group] := [=];
    ns := ref rec.node_state[group];
    # node specified by index
    if( is_integer(node) )
    {
      if( len(node) == 1 )  # single index: add to by_nodeindex map
      {
        if( !has_field(ns,'by_nodeindex') )
          ns.by_nodeindex := [=];
        ns.by_nodeindex[spaste('#',as_string(node))] := state;
      }
      else # multiple indices: add to by_list map
      {
        if( !has_field(ns,'by_list') )
        {
          ns.by_list := [=];
          ns.by_list::dmi_datafield_content_type := 'DataRecord';
        }
        ns.by_list[spaste('#',len(ns.by_list)+1)] := [ nodeindex=node,state=state ];
      }
    }
    else if( is_string(node) ) # string nodes: add to by_list map
    {
      if( !has_field(ns,'by_list') )
      {
        ns.by_list := [=];
        ns.by_list::dmi_datafield_content_type := 'DataRecord';
      }
      ns.by_list[spaste('#',len(ns.by_list)+1)] := [ name=node,state=state ];
    }
    else
      fail 'meqrequest.addstate(): node must be specified by index or name(s)';
    return T;
  }
  
  return ref rec;
}

