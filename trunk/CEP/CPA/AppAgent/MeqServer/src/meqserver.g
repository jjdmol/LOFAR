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

const meqserver_test := function ()
{
  global mqs;
  mqs := meqserver(verbose=4); # ,server='./meqserver');
  mqs.init([=],[=],[=],wait=T);
  print mqs.meq('Create.Node',[class='MEQNode',name='test'],T);
  print mqs.meq('Get.Node.State',[name='test'],T);
  print mqs.meq('Get.Node.State',[nodeindex=1],T);
}
