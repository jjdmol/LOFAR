pragma include once
include 'appagent/app_proxy.g'

# find path to server binary
if( has_field(lofar_software,'meq') && has_field(lofar_software.meq,'server') )
  const _meqserver_binary := lofar_software.meq.server;
else
  const _meqserver_binary := './meqserver';
  print _meqserver_binary;

const meqserver := function (appid='MeqServer',
    server=_meqserver_binary,options="-d0 -meq:M:M:MeqServer",
    suspend=F,verbose=1,
    gui=F,ref parent_frame=F,ref widgetset=dws,
    ref self=[=],ref public=[=])
{
  # construct base app_proxy object
  if( is_fail( app_proxy(appid,server=server,options=options,
                         suspend=suspend,verbose=verbose,
                         gui=gui,parent_frame=parent_frame,widgetset=widgetset,
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
  if( is_record(children) )
    defrec.children := children;
  if( !is_record(default) || len(default) )
    defrec.default := default;
  return defrec;
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

const meqserver_test := function ()
{
  global mqs;
  mqs := meqserver(verbose=4,server='./meqserver'); #,suspend=T);
  mqs.setdebug('MeqNode',5);
  mqs.init([=],[=],[=],wait=T);
  print mqs.meq('Create.Node',[class='MEQNode',name='test'],T);
  print mqs.meq('Create.Node',[class='MEQNode',name='child1'],T);
  print mqs.meq('Create.Node',[class='MEQNode',name='child2'],T);
  print mqs.meq('Create.Node',[class='MEQNode',name='child4'],T);
  
  defval1 := array(as_double(1),2,2);
  defval2 := array(as_double(2),1,1);
  
  cosrec := [ class='MEQCos',name='cosp1',children=[ 
      x=[ class='MEQParmPolcStored',name='p1',default=defval1 ] ] ];
      
  addrec := [ class='MEQAdd',name='add1_2',children=[
      x=cosrec,
      y=[ class='MEQParmPolcStored',name='p2',default=defval2 ] ] ];
      
  print mqs.meq('Create.Node',addrec,T);
  print mqs.meq('Resolve.Children',[name='add1_2'],T);
  print mqs.meq('Get.Node.State',[name='test'],T);
  print mqs.meq('Get.Node.State',[name='add1_2'],T);
  print mqs.meq('Get.Node.State',[nodeindex=1],T);
  
  cells := meqcells(meqdomain(0,10,0,10),nfreq=20,times=[1.,2.,3.],timesteps=[1.,2.,3.]);
  request := meqrequest(cells);
  
  print mqs.meq('Get.Result',[name='add1_2',request=request],T);
  print mqs.meq('Get.Node.State',[name='add1_2'],T);
}
