# use_suspend  := T;
# use_gui := T;
# use_nostart  := T;
# use_valgrind := T;
use_valgrind_opts := [ "",
#  "--gdb-attach=yes",          # use either this...
  "--logfile=vg.meqserver",       # ...or this, not both
# "--skin=helgrind --logfile=hg.meqserver";
#  "--gdb-path=/home/oms/bin/valddd", 
  ""];
  
include 'meq/meqserver.g'

default_debuglevels := [  MeqNode       =3,
                          MeqForest     =3,
                          MeqSink       =3,
                          MeqSpigot     =3,
                          MeqVisHandler =3,
                          MeqServer     =3,
                          meqserver     =1      ];

# inits a meq.server
const mqsinit := function (verbose=3,debug=[=],gui=use_gui)
{
  global mqs;
  if( !is_record(mqs) )
  {
    mqs := meq.server(verbose=verbose,options="-d0 -nogw -meq:M:O:MeqServer",gui=gui);
    if( is_fail(mqs) )
      fail;
#    mqs.setdebug('Glish',5);
    r:=[=];
    r.a := function () {};
    r.b := T;
    r.a::dmi_ignore := T;
    r.b::dmi_ignore := T;
    mqs.meq('a',r);
    mqs.init([output_col="PREDICT"],wait=T);
    if( is_record(debug) )
    {
      for( lev in field_names(default_debuglevels) )
        mqs.setdebug(lev,default_debuglevels[lev]);
      for( lev in field_names(debug) )
        mqs.setdebug(lev,debug[lev]);
    }
  }
}


const meqsink_test := function (gui=use_gui)
{
  # remove output column from table
  tbl := table('test.ms',readonly=F);
  tbl.removecols('PREDICTED_DATA');
  tbl.done();
  
  if( is_fail(mqsinit(verbose=1,gui=gui)) )
  {
    print mqs;
    fail;
  }

  # initialize meq.server
  mqs.init([output_col="PREDICT"],
      output=[write_flags=F,predict_column='PREDICTED_DATA'],
      wait=T);
  
  # create a small subtree
  defval1 := array(as_double(1),2,2);
  defval2 := array(as_double(2),1,1);
  addrec := meq.node('MeqSubtract','compare',children="spigot1 spigot2");
  print mqs.meq('Create.Node',addrec);
  # create spigot (note! for now, a spigot MUST be created first)
  spigrec1 := meq.node('MeqSpigot','spigot1');
  spigrec1.input_col := 'DATA';
  spigrec1.station_1_index := 1;
  spigrec1.station_2_index := 2;
  print mqs.meq('Create.Node',spigrec1);
  spigrec2 := meq.node('MeqSpigot','spigot2');
  spigrec2.input_col := 'DATA';
  spigrec2.station_1_index := 1;
  spigrec2.station_2_index := 2;
  print mqs.meq('Create.Node',spigrec2);
  # create sink
  sinkrec := meq.node('MeqSink','sink1',children="compare");
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

const meqsel_test := function (gui=use_gui)
{
  if( is_fail(mqsinit(verbose=4,gui=gui)) )
  {
    print mqs;
    fail;
  }
  # create a small subtree
  defval1 := array(as_double(1),2,2);
  defval2 := array(as_double(2),1,1);
  defval3 := array(as_double(3),1,1);
  print mqs.meq('Create.Node',meq.parm('parm1',defval1));
  print mqs.meq('Create.Node',meq.parm('parm2',defval2));
  print mqs.meq('Create.Node',meq.parm('parm3',defval3));
  print mqs.meq('Create.Node',meq.parm('parm4',defval1));
  print mqs.meq('Create.Node',meq.parm('parm5',defval2));
  print mqs.meq('Create.Node',meq.parm('parm6',defval3));
  print mqs.meq('Create.Node',meq.node('MeqComposer','compose1',children="parm1 parm2 parm3"));
  print mqs.meq('Create.Node',meq.node('MeqComposer','compose2',children="parm3 parm5 parm6"));
  rec := meq.node('MeqSelector','select1',children="compose1");
  rec.index := [1,5];
  print mqs.meq('Create.Node',rec);
  rec := meq.node('MeqSelector','select2',children="compose2");
  rec.index := [1,5];
  print mqs.meq('Create.Node',rec);
  print mqs.meq('Create.Node',meq.node('MeqComposer','compose3',children="select1 select2"));
  rec := meq.node('MeqSelector','select3',children="compose3");
  rec.index := [2,3,4];
  print mqs.meq('Create.Node',rec);
  
  # resolve children
  print mqs.meq('Resolve.Children',[name='select3']);
  
  global cells,request,res;
  cells := meq.cells(meq.domain(0,10,0,10),20,3);
  request := meq.request(cells);
  res := mqs.meq('Node.Execute',[name='select3',request=request],T);
  print res;
}

const state_test_init := function (gui=use_gui)
{
  if( is_fail(mqsinit(verbose=4,gui=F)) )
  {
    print mqs;
    fail;
  }
  mqs.meq('Clear.Forest');
  # create a small subtree
  defval1 := array(as_double(1),1,1);
  defval2 := array(as_double(2),1,1);
  defval3 := array(as_double(3),1,1);
  print mqs.meq('Create.Node',meq.parm('parm1',defval1));
  print mqs.meq('Create.Node',meq.parm('parm2',defval2));
  print mqs.meq('Create.Node',meq.parm('parm3',defval3));
  print mqs.meq('Create.Node',meq.parm('parm4',defval1));
  print mqs.meq('Create.Node',meq.parm('parm5',defval2));
  print mqs.meq('Create.Node',meq.parm('parm6',defval3));
  print mqs.meq('Create.Node',meq.node('MeqComposer','compose1',children="parm1 parm2 parm3"));
  print mqs.meq('Create.Node',meq.node('MeqComposer','compose2',children="parm4 parm5 parm6"));
  rec := meq.node('MeqSelector','select1',children="compose1");
  rec.index := 1;
  rec.node_groups := hiid("a b");
  print mqs.meq('Create.Node',rec);
  rec := meq.node('MeqSelector','select2',children="compose2");
  rec.index := 1;
  rec.node_groups := hiid("a b");
  print mqs.meq('Create.Node',rec);
  print mqs.meq('Create.Node',meq.node('MeqComposer','compose3',children="select1 select2"));
  
  # resolve children
  print mqs.meq('Resolve.Children',[name='compose3']);
}

const state_test := function (gui=use_gui)
{
  if( !is_record(mqs) )
    state_test_init(gui=gui);
  mqs.setdebug('Glish',5);
  
  # get indices
  ni_sel1 := mqs.getnodestate('select1').nodeindex;
  ni_sel2 := mqs.getnodestate('select2').nodeindex;
  
  global cells,request,res;
  cells := meq.cells(meq.domain(0,10,0,10),20,3);
  request := meq.request(cells);
  
#  mqs.setdebug("DMI Glish MeqServer glishclientwp meq.server Dsp",5);
  mqs.setdebug("Glish",5);

  res1 := mqs.meq('Node.Execute',[name='compose3',request=request],T);
  req1 := request;
  print res1;
  
  request := meq.request(cells);
  request.add_state('a','select1',[index=2]);
  request.add_state('b',ni_sel2,[index=3]);
  res2 := mqs.meq('Node.Execute',[name='compose3',request=request],T);
  req2 := request;
  print res2;
  
  request := meq.request(cells);
  request.add_state('a',"select1 select2",[index=3]);
  res3 := mqs.meq('Node.Execute',[name='compose3',request=request],T);
  req3 := request;
  print res3;
  
  request := meq.request(cells);
  request.add_state('b','select1',[index=2]);
  request.add_state('b',"",[index=1]);
  res4 := mqs.meq('Node.Execute',[name='compose3',request=request],T);
  req4 := request;
  print res4;
  
  print 'Expecting 1,1: ',res1,req1;
  print 'Expecting 2,3: ',res2,req2;
  print 'Expecting 3,3: ',res3,req3;
  print 'Expecting 2,1: ',res4,req4;
}

const freq_test := function (gui=use_gui)
{
  meqsel_test();
  mqs.setverbose(5);
  mqs.setdebug("MeqNode",4);
  dom:=meq.domain(10,20,10,20);
  cells:=meq.cells(dom,10,3);
  mqs.meq('Create.Node',[class='MeqFreq',name='f']);
  print a:=mqs.meq('Node.Execute',[name='f',request=meq.request(cells)],T);
}

const solver_test := function (gui=use_gui,verbose=4,publish=T)
{
  if( is_fail(mqsinit(verbose=verbose,gui=gui,debug=F)) )
  {
    print mqs;
    fail;
  }
  mqs.setdebug('MeqSolver',5);
  # create parms and condeq
#  defval1 := array([3.,0.5,0.5,0.1],2,2);
#  defval2 := array([2.,10.,2.,10. ],2,2);
  defval1 := 1;
  defval2 := 0;
  print mqs.meq('Create.Node',meq.parm('a',defval1,groups='Parm'));
  print mqs.meq('Create.Node',meq.parm('b',defval2,groups='Parm'));
  print mqs.meq('Create.Node',meq.node('MeqCondeq','condeq1',children="a b"));
  # create solver
  global rec;
  rec := meq.node('MeqSolver','solver1',children="condeq1",groups='Solver');
  rec.default := [ num_iter = 3 ];
  rec.solvable := meq.solvable_list("a");
  rec.parm_group := hiid('Parm');
  print mqs.meq('Create.Node',rec);
  
  for( n in "a condeq1" )
    print mqs.meq('Node.Publish.Results',[name=n,enable=T]);
  
  global cells,cells2,request,request2,res,res2,st1,st2,st3;
  
  # resolve children
  print mqs.meq('Resolve.Children',[name='solver1']);
  
  st1 := mqs.getnodestate('a');
  cells  := meq.cells(meq.domain(0,.5,0,.5),num_freq=4,num_time=4);
  cells2 := meq.cells(meq.domain(.5,1,.5,1),num_freq=4,num_time=4);
  request := meq.request(cells,calc_deriv=2);
  
  cmdrec := [ clear_matrix=F,invert_matrix=F,num_iter=5,save_polcs=F ];
  request.add_command('Solver','solver1',cmdrec);
  
  request2 := meq.request(cells2,calc_deriv=0);
  res := mqs.meq('Node.Publish.Results',[name='condeq1'],T);
  res := mqs.meq('Node.Execute',[name='solver1',request=request],T);
  print res;
  st2 := mqs.getnodestate('a');
  print st2.solve_polcs;
  res2 := mqs.meq('Node.Execute',[name='a',request=request2],T);
  print res2;
  st3 := mqs.getnodestate('a');
  print st3.solve_polcs;
}

const save_test := function (clear=F)
{
  print 'saving forest';
  print mqs.meq('Save.Forest',[file_name='forest.sav'],T);
}

const load_test := function (gui=use_gui)
{
  if( is_fail(mqsinit(verbose=4,gui=gui)) )
  {
    print mqs;
    fail;
  }
  print 'loading forest';
  print mqs.meq('Load.Forest',[file_name='forest.sav'],T);
  print mqs.meq('Get.Node.List',[=],T);
}

const mep_test := function (gui=use_gui)
{
  tablename := 'test.mep';
  print 'Initializing MEP table ',tablename;
  
  include 'meq/meptable.g';
  pt := meq.meptable(tablename,create=T);
  pt.putdef('a');
  pt.putdef('b');
  pt.done();
  
  if( is_fail(mqsinit(verbose=4,gui=gui)) )
  {
    print mqs;
    fail;
  }
  mqs.setdebug('MeqParm',5);
  
  defrec := meq.parm('a');
  defrec.table_name := tablename;
  print mqs.meq('Create.Node',defrec);
  
  global cells,request,res;
  cells := meq.cells(meq.domain(0,1,0,1),4,4);
  request := meq.request(cells,calc_deriv=0);
  
  res := mqs.meq('Node.Publish.Results',[name='a'],T);
  res := mqs.meq('Node.Execute',[name='a',request=request],T);
}

const mep_grow_test := function (gui=use_gui)
{
  if( is_fail(mqsinit(verbose=4,gui=gui)) )
  {
    print mqs;
    fail;
  }
  mqs.setdebug('MeqParm',5);
  
  polc := meq.polc(array([1,1,1,0],2,2),domain=meq.domain(0,1,0,1));
  polc.grow_domain := T;
  print 'Polc is ',polc;
  defrec := meq.parm('a',polc,groups='Parm');
  mqs.meq('Create.Node',defrec,T);
  
  global cells,request,res1,res2;
  cells := meq.cells(meq.domain(0,1,0,1),4,4);
  request := meq.request(cells,calc_deriv=1);
  
  mqs.meq('Node.Publish.Results',[name='a'],T);
  res1 := mqs.meq('Node.Execute',[name='a',request=request],T);

  cells := meq.cells(meq.domain(0,2,0,2),4,4);
  request := meq.request(cells,calc_deriv=1);
  
  res2 := mqs.meq('Node.Execute',[name='a',request=request],T);
}


