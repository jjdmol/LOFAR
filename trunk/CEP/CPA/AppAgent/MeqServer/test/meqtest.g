# use_suspend  := T;
# use_nostart  := T;
# use_valgrind := T;
use_valgrind_opts := [ "",
#  "--gdb-attach=yes",          # use either this...
 "--logfile=vg.meqserver",       # ...or this, not both
#  "--gdb-path=/home/oms/bin/valddd", 
  ""];
  
include 'meq/meqserver.g'

const meqserver_test := function ()
{
  global mqs;
  # create meqserver object
  mqs := meqserver(verbose=4);
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
  print mqs.meq('Node.Get.State',[name='test1'],T);
  # get node state, node specified via index
  print mqs.meq('Node.Get.State',[nodeindex=1],T);

  # test creating a sub-tree
  defval1 := array(as_double(1),2,2);
  defval2 := array(as_double(2),1,1);
  cosrec := [ class='MeqCos',name='cosp1',children=[ 
      x=[ class='MeqParmPolcStored',name='p1',default=defval1 ] ] ];
  addrec := [ class='MeqAdd',name='add1_2',children=[
      x=cosrec,
      y=[ class='MeqParmPolcStored',name='p2',default=defval2 ] ] ];
      
  print mqs.meq('Create.Node',addrec,F);
  print mqs.meq('Resolve.Children',[name='add1_2'],F);
  
  cells := meqcells(meqdomain(0,10,0,10),nfreq=20,times=[1.,2.,3.],timesteps=[1.,2.,3.]);
  request := meqrequest(cells);
  print mqs.meq('Node.Execute',[name='add1_2',request=request],T);
  print mqs.meq('Node.Get.State',[name='add1_2'],T);
}

const meqsink_test := function ()
{
  global mqs;
  mqs := meqserver(verbose=4,options="-d0 -meq:M:O:MeqServer",gui=T);
  # set verbose debugging messages
  mqs.setdebug("MeqNode MeqForest MeqSink MeqSpigot",1);
  mqs.setdebug("MeqNode MeqForest MeqSink MeqSpigot MeqNode",2);
  mqs.setdebug("MeqServer MeqVisHandler",2);
  mqs.setdebug("meqserver",1);
  mqs.setdebug("MeqNode",5);
  # initialize meqserver
  mqs.init([output_col="PREDICT"],wait=T);
  
  # create a small subtree
  defval1 := array(as_double(1),2,2);
  defval2 := array(as_double(2),1,1);
  addrec := meqnode('MeqSubtract','compare',children="spigot1 spigot2");
  print mqs.meq('Create.Node',addrec);
  # create spigot (note! for now, a spigot MUST be created first)
  spigrec1 := meqnode('MeqSpigot','spigot1');
  spigrec1.input_col := 'DATA';
  spigrec1.station_1_index := 1;
  spigrec1.station_2_index := 2;
  print mqs.meq('Create.Node',spigrec1);
  spigrec2 := meqnode('MeqSpigot','spigot2');
  spigrec2.input_col := 'DATA';
  spigrec2.station_1_index := 1;
  spigrec2.station_2_index := 2;
  print mqs.meq('Create.Node',spigrec2);
  # create sink
  sinkrec := meqnode('MeqSink','sink1',children="compare");
  sinkrec.output_col := 'PREDICT'; 
  sinkrec.station_1_index := 1;
  sinkrec.station_2_index := 2;
  print mqs.meq('Create.Node',sinkrec);
  
  # resolve its children
  print mqs.meq('Resolve.Children',[name='sink1'],F);
  
  # activate input agent and watch the fireworks
  global inputrec;
  inputrec := [ ms_name = 'test.ms',data_column_name = 'DATA',tile_size=5,
                selection = [=]  ];
  mqs.init(input=inputrec); 
}

const meqsel_test := function ()
{
  global mqs;
  mqs := meqserver(verbose=4,options="-d0 -meq:M:O:MeqServer",gui=T);
  # set verbose debugging messages
  mqs.setdebug("MeqNode MeqForest MeqSink MeqSpigot",5);
  mqs.setdebug("MeqNode MeqForest MeqSink MeqSpigot",5);
  mqs.setdebug("MeqServ MeqVisHandler",5);
  mqs.setdebug("meqserver",1);
  # initialize meqserver
  mqs.init([output_col="PREDICT"],wait=T);
  
  # create a small subtree
  defval1 := array(as_double(1),2,2);
  defval2 := array(as_double(2),1,1);
  defval3 := array(as_double(3),1,1);
  print mqs.meq('Create.Node',meqparm('parm1',defval1));
  print mqs.meq('Create.Node',meqparm('parm2',defval2));
  print mqs.meq('Create.Node',meqparm('parm3',defval3));
  print mqs.meq('Create.Node',meqparm('parm4',defval1));
  print mqs.meq('Create.Node',meqparm('parm5',defval2));
  print mqs.meq('Create.Node',meqparm('parm6',defval3));
  print mqs.meq('Create.Node',meqnode('MeqComposer','compose1',children="parm1 parm2 parm3"));
  print mqs.meq('Create.Node',meqnode('MeqComposer','compose2',children="parm4 parm5 parm6"));
  rec := meqnode('MeqSelector','select1',children="compose1");
  rec.index := [1,5];
  print mqs.meq('Create.Node',rec);
  rec := meqnode('MeqSelector','select2',children="compose2");
  rec.index := [1,5];
  print mqs.meq('Create.Node',rec);
  print mqs.meq('Create.Node',meqnode('MeqComposer','compose3',children="select1 select2"));
  rec := meqnode('MeqSelector','select3',children="compose3");
  rec.index := [2,3,4];
  print mqs.meq('Create.Node',rec);
  
  # resolve children
  print mqs.meq('Resolve.Children',[name='select3']);
  
  global cells,request,res;
  cells := meqcells(meqdomain(0,10,0,10),nfreq=20,times=[1.,2.,3.],timesteps=[1.,2.,3.]);
  request := meqrequest(cells);
  res := mqs.meq('Node.Execute',[name='select3',request=request],T);
  print res;
}

cells := meqcells(meqdomain(0,10,0,10),nfreq=20,times=[1.,2.,3.],timesteps=[1.,2.,3.]);
request := meqrequest(cells,1);

const freq_test := function ()
{
  meqsel_test();
  mqs.setverbose(5);
  mqs.setdebug("MeqNode",4);
  dom:=meqdomain(10,20,10,20);
  cells:=meqcells(dom,10,[11.0,12,13],[1.,1,1]);
  mqs.meq('Create.Node',[class='MeqFreq',name='f']);
  print a:=mqs.meq('Node.Execute',[name='f',request=meqrequest(cells)],T);
}
