# use_suspend  := T;
# use_nostart  := T;
# use_valgrind := T;
# "--skin=helgrind --logfile=hg.meqserver";
use_valgrind_opts := [ "",
#  "--gdb-attach=yes",          # use either this...
  "--logfile=vg.meqserver",       # ...or this, not both
#  "--gdb-path=/home/oms/bin/valddd", 
  ""];
  
include 'meq/meqserver.g'

const meqsink_test := function ()
{
  global mqs;
  mqs := meqserver(verbose=1,options="-d0 -nogw -meq:M:M:MeqServer",gui=T);
  if( is_fail(mqs) )
  {
    print mqs;
    fail;
  }
  # set verbose debugging messages
  mqs.setdebug("MeqNode MeqForest MeqSink MeqSpigot",1);
  mqs.setdebug("MeqNode MeqForest MeqSink MeqSpigot MeqNode",2);
  mqs.setdebug("MeqServer MeqVisHandler",3);
  mqs.setdebug("meqserver",1);
  mqs.setdebug("MeqNode",5);
  mqs.setdebug("MSVisAgent",10);
  # remove output column from table
  tbl:=table('test.ms',readonly=F);
  tbl.removecols('CORRECTED_DATA');
  tbl.done()

  # initialize meqserver
  mqs.init([output_col="PREDICT"],
      output=[write_flags=F,predict_column='MODEL_DATA'],
      wait=T);
  
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
  mqs := meqserver(verbose=4,options="-d0 -nogw -meq:M:O:MeqServer",gui=F);
  if( is_fail(mqs) )
  {
    print mqs;
    fail;
  }
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
  print mqs.meq('Create.Node',meqnode('MeqComposer','compose2',children="parm3 parm5 parm6"));
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

const state_test_init := function ()
{
  global mqs;
  if( !is_record(mqs) )
  {
    mqs := meqserver(verbose=4,options="-d0 -nogw -meq:M:O:MeqServer",gui=F);
    if( is_fail(mqs) )
    {
      print mqs;
      fail;
    }
  }
  # set verbose debugging messages
  mqs.setdebug("MeqNode MeqForest MeqSink MeqSpigot",5);
  mqs.setdebug("MeqNode MeqForest MeqSink MeqSpigot",5);
  mqs.setdebug("MeqServ MeqVisHandler",5);
  mqs.setdebug("Glish",5);
  mqs.setdebug("meqserver",1);
  # initialize meqserver
  mqs.init([output_col="PREDICT"],wait=T);
  
  # create a small subtree
  defval1 := array(as_double(1),1,1);
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
  rec.index := 1;
  rec.config_groups := hiid("a b");
  print mqs.meq('Create.Node',rec);
  rec := meqnode('MeqSelector','select2',children="compose2");
  rec.index := 1;
  rec.config_groups := hiid("a b");
  print mqs.meq('Create.Node',rec);
  print mqs.meq('Create.Node',meqnode('MeqComposer','compose3',children="select1 select2"));
  
  # resolve children
  print mqs.meq('Resolve.Children',[name='compose3']);
}

const state_test := function ()
{
  global mqs;
  if( !is_record(mqs) )
    state_test_init();
  
  # get indices
  ni_sel1 := mqs.getnodestate('select1').nodeindex;
  ni_sel2 := mqs.getnodestate('select2').nodeindex;
  
  global cells,request,res;
  cells := meqcells(meqdomain(0,10,0,10),nfreq=20,times=[1.,2.,3.],timesteps=[1.,2.,3.]);
  request := meqrequest(cells);
  
  res1 := mqs.meq('Node.Execute',[name='compose3',request=request],T);
  req1 := request;
  print res1;
  
  request := meqrequest(cells);
  request.addstate('a','select1',[index=2]);
  request.addstate('b',ni_sel2,[index=3]);
  res2 := mqs.meq('Node.Execute',[name='compose3',request=request],T);
  req2 := request;
  print res2;
  
  request := meqrequest(cells);
  request.addstate('a',"select1 select2",[index=3]);
  res3 := mqs.meq('Node.Execute',[name='compose3',request=request],T);
  req3 := request;
  print res3;
  
  request := meqrequest(cells);
  request.addstate('b','select1',[index=2]);
  request.addstate('b','*',[index=1]);
  res4 := mqs.meq('Node.Execute',[name='compose3',request=request],T);
  req4 := request;
  print res4;
  
  print 'Expecting 1,1: ',res1,req1;
  print 'Expecting 2,3: ',res2,req2;
  print 'Expecting 3,3: ',res3,req3;
  print 'Expecting 2,1: ',res4,req4;
}

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

const solver_test := function ()
{
  global mqs;
  mqs := meqserver(verbose=4,options="-d0 -nogw -meq:M:O:MeqServer",gui=F);
  if( is_fail(mqs) )
  {
    print mqs;
    fail;
  }
  # set verbose debugging messages
  mqs.setdebug("MeqNode MeqForest MeqSink MeqSpigot",3);
  mqs.setdebug("MeqNode MeqForest MeqSink MeqSpigot",3);
  mqs.setdebug("MeqServ MeqVisHandler",3);
  mqs.setdebug("MeqNode",5);
  mqs.setdebug("Glish",10);
  mqs.setdebug("meqserver",1);
  # initialize meqserver
  mqs.init([output_col="PREDICT"],wait=T);
  
  # create parms and condeq
  defval1 := array([1.,2.,1.5,0.2,1.3,0.5],2,3);
  defval2 := array([2.,10.,2.,10.,2.,10],2,3);
  print mqs.meq('Create.Node',meqparm('parm1',defval1,config_groups='Solvable.Parm'));
  print mqs.meq('Create.Node',meqparm('parm2',defval1,config_groups='Solvable.Parm'));
  print mqs.meq('Create.Node',meqnode('MeqCondeq','condeq1',children=[a='parm1',b='parm2']));
  # create solver
  global rec;
  rec := meqnode('MeqSolver','solver1',children="condeq1");
  rec.num_steps := 3;
  rec.solvable_parm := [ by_list=meqinitstatelist() ];
  meqaddstatelist(rec.solvable_parm.by_list,"parm2",[solvable=T]); 
  meqaddstatelist(rec.solvable_parm.by_list,"*",[solvable=F]); 
  print mqs.meq('Create.Node',rec);
  
  # resolve children
  print mqs.meq('Resolve.Children',[name='solver1']);
  
  global cells,request,res;
  cells := meqcells(meqdomain(1,4,-2,3),nfreq=4,times=[0.,1.,2.,3.],timesteps=[1.,1.,1.,1.]);
  request := meqrequest(cells,calc_deriv=T);
  res := mqs.meq('Node.Execute',[name='solver1',request=request],T);
  print res;
}


# cells := meqcells(meqdomain(0,10,0,10),nfreq=20,times=[1.,2.,3.],timesteps=[1.,2.,3.]);
# request := meqrequest(cells,1);

