# use_suspend  := T;
# use_nostart  := T;
# use_valgrind := T;
use_valgrind_opts := [ "",
#  "--gdb-attach=yes",          # use either this...
  "--logfile=meqserver",       # ...or this, not both
#  "--gdb-path=/usr/bin/ddd", 
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
  mqs := meqserver(verbose=4,options="-d0 -meq:M:M:MeqServer");
  # set verbose debugging messages
  mqs.setdebug("MeqNode MeqForest MeqSink MeqSpigot",1);
  mqs.setdebug("MeqServ MeqVisHandler",1);
  mqs.setdebug("MeqServer",1);
  # initialize meqserver
  mqs.init([output_col="PREDICT"],wait=T);
  
  # create a small subtree
  defval1 := array(as_double(1),2,2);
  defval2 := array(as_double(2),1,1);
  addrec := meqnode('meqCompare','compare',
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
