pragma include once
include 'appagent/app_proxy.g'

# find path to server binary
if( has_field(lofar_software,'meq') && has_field(lofar_software.meq,'servers') )
  for( f in lofar_software.meq.servers )
    if( len(stat(f)) )
    {
      print 'Found path to meqserver binary:',f;
      const _meqserver_binary := f;
      break;
    }
# not found? try default
if( !is_defined('_meqserver_binary') )
{
  const _meqserver_binary := 'meqserver';
  print 'Will use default meqserver binary, hope this works';
}

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
  if( !is_boolean(children) )
    defrec.children := children;
  if( !is_record(default) || len(default) )
    defrec.default := default;
  return defrec;
}

const meqparm := function (name,default=[=] )
{
  return meqnode('MeqParmPolcStored',name,default=default);
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
  # create meqserver object
  mqs := meqserver(verbose=4,suspend=F);
  # set verbose debugging messages
  mqs.setdebug('MeqNode',5);
  # initialize meqserver
  mqs.init([=],[=],[=],wait=T);
  
  # create some test nodes
  print mqs.meq('Create.Node',[class='MeqNode',name='x'],T);
  print mqs.meq('Create.Node',[class='MeqNode',name='y'],T);
  print mqs.meq('Create.Node',[class='MeqNode',name='z'],T);
  
  # test various ways to specify children
  #   children specified as an array of names
  #   "w" is a forward reference, child w will be created later on
  print mqs.meq('Create.Node',[class='MeqNode',name='test1',children="x y z w"],T);
  #   children specified as an array of node indices
  print mqs.meq('Create.Node',[class='MeqNode',name='test2',children=[2,3,4]],T);
  #   children specified as a record. Field name is child name (argument name)
  children := [ a='x',          # child 'a' specified by name
                b=2,            # child 'b' specified by node index
                c='y',          # child 'c' will be created later on
                d=[ class='MeqNode',name='aa' ] ]; # created on-the-fly
  print mqs.meq('Create.Node',[class='MeqNode',name='w',children=children],T);
        
  # this resolves remaining children of "test1" (specifically, "w")
  print mqs.meq('Resolve.Children',[name='test1'],T);
  
  # get node state, node specified via name
  print mqs.meq('Get.Node.State',[name='test1'],T);
  # get node state, node specified via index
  print mqs.meq('Get.Node.State',[nodeindex=1],T);

  # test creating a sub-tree
  defval1 := array(as_double(1),2,2);
  defval2 := array(as_double(2),1,1);
  cosrec := [ class='MeqCos',name='cosp1',children=[ 
      x=[ class='MeqParmPolcStored',name='p1',default=defval1 ] ] ];
  addrec := [ class='MeqAdd',name='add1_2',children=[
      x=cosrec,
      y=[ class='MeqParmPolcStored',name='p2',default=defval2 ] ] ];
      
  print mqs.meq('Create.Node',addrec,T);
  print mqs.meq('Resolve.Children',[name='add1_2'],T);
  
  cells := meqcells(meqdomain(0,10,0,10),nfreq=20,times=[1.,2.,3.],timesteps=[1.,2.,3.]);
  request := meqrequest(cells);
  print mqs.meq('Get.Result',[name='add1_2',request=request],T);
  print mqs.meq('Get.Node.State',[name='add1_2'],T);
}

const meqsink_test := function ()
{
  global mqs;
  # create meqserver object
  mqs := meqserver(verbose=4,options="-d0 -meq:M:M:MeqServer",suspend=F);
  # set verbose debugging messages
  mqs.setdebug("MeqNode MeqForest MeqSink MeqSpigot MeqNode MeqVisHandler MeqServ",5);
  mqs.setdebug("MeqServ MeqVisHandler",3);
  mqs.setdebug("MeqServer",1);
  # initialize meqserver
  mqs.init([output_col="PREDICT"],wait=T);
  
  # create a small subtree
  defval1 := array(as_double(1),2,2);
  defval2 := array(as_double(2),1,1);
  addrec := meqnode('Meqcompare','compare',
              children=[ x=meqparm('p1',default=defval1), 
                         y='spigot1' ]);
  print mqs.meq('Create.Node',addrec);
  # create spigot (note! for now, a spigot MUST be created first)
  spigrec := meqnode('MeqSpigot','spigot1');
  spigrec.input_col := 'DATA';
  spigrec.corr_index := 1;
  spigrec.station_1_index := 1;
  spigrec.station_2_index := 2;
  print mqs.meq('Create.Node',spigrec);
  
  # create sink
  sinkrec := meqnode('MeqSink','sink1',children="compare");
  sinkrec.output_col := 'PREDICT'; 
  sinkrec.corr_index := 1;
  sinkrec.station_1_index := 1;
  sinkrec.station_2_index := 2;
  print mqs.meq('Create.Node',sinkrec);
  
  # resolve its children
  print mqs.meq('Resolve.Children',[name='sink1'],T);
  
  # activate input agent and watch the fireworks
  global inputrec;
  inputrec := [ ms_name = 'test.ms',data_column_name = 'DATA',tile_size=10,
                selection = [=]  ];
  mqs.init(input=inputrec); 
}
