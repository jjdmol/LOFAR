# use_suspend := T;
# use_nostart  := T;
# use_valgrind := T;
# "--skin=helgrind --logfile=hg.meqserver";
use_valgrind_opts := [ "",
#  "--gdb-attach=yes",          # use either this...
  "--logfile=vg.meqserver",       # ...or this, not both
#  "--gdb-path=/home/oms/bin/valddd", 
  ""];
  
if( any(argv == '-runtest' ) ) {
  meq_path := '.'
} else {
  meq_path := 'meq'
}
  
include spaste(meq_path,'/meqserver.g')
include spaste(meq_path,'/meptable.g')

default_debuglevels := [  MeqNode       =2,
                          MeqForest     =2,
                          MeqSink       =2,
                          MeqSpigot     =2,
                          MeqVisHandler =2,
                          MeqServer     =2,
                          meqserver     =1 ];
                      
# inits a meqserver
const mqsinit := function (verbose=3,debug=[=],gui=F)
{
  global mqs;
  if( !is_record(mqs) )
  {
    mqs := meq.server(verbose=verbose,options="-d0 -nogw -meq:M:O:MeqServer",gui=gui);
    if( is_fail(mqs) )
      fail;
    mqs.init([output_col="PREDICT"],wait=T);
    if( !( is_boolean(debug) && !debug ) )
    {
      for( lev in field_names(default_debuglevels) )
        mqs.setdebug(lev,default_debuglevels[lev]);
      if( is_record(debug) )
        for( lev in field_names(debug) )
          mqs.setdebug(lev,debug[lev]);
    }
  }
}

const solver_test := function (stage=0,gui=F,debug=[=],verbose=1)
{
  global mqs;
  mqsinit(debug=debug,verbose=verbose,gui=gui)
  mqs.meq('Clear.Forest');

  # define true parameter values. Solutions will start from zero
  x0 := 2.; y0 := 1.;
  
  if( stage == 0 )
  {
    # use default record for parms
    print mqs.meq('Create.Node',meq.parm('x',meq.polc(0),groups='Parm'));
    print mqs.meq('Create.Node',meq.parm('y',meq.polc(0),groups='Parm'));
  }
  else if( stage == 1 )
  {
    # use table for parms
    tablename := 'test.mep';
    pt := meq.meptable(tablename,create=T);
    pt.putdef('x',0);
    pt.putdef('y',0);
    pt.done();
    x := meq.parm('x',groups='Parm');
    x.table_name := tablename;
    y := meq.parm('y',groups='Parm');
    y.table_name := tablename;
    print mqs.meq('Create.Node',x);
    print mqs.meq('Create.Node',y);
  }
  
  # stages 0 and 1: create forest
  if( stage != 2 )
  {
    cc := [ a1=1,b1=1,a2=1,b2=-1 ];
    cc.c1 := cc.a1*x0 + cc.b1*y0;
    cc.c2 := cc.a2*x0 + cc.b2*y0;
    for( f in field_names(cc) )
      print mqs.meq('Create.Node',meq.parm(f,array(as_double(cc[f]),1,1)));
    print mqs.meq('Create.Node',meq.node('MeqMultiply','a1x',children="a1 x"));
    print mqs.meq('Create.Node',meq.node('MeqMultiply','a2x',children="a2 x"));
    print mqs.meq('Create.Node',meq.node('MeqMultiply','b1y',children="b1 y"));
    print mqs.meq('Create.Node',meq.node('MeqMultiply','b2y',children="b2 y"));
    print mqs.meq('Create.Node',meq.node('MeqAdd','lhs1',children="a1x b1y"));
    print mqs.meq('Create.Node',meq.node('MeqAdd','lhs2',children="a2x b2y"));
    print mqs.meq('Create.Node',meq.node('MeqCondeq','eq1',children="lhs1 c1"));
    print mqs.meq('Create.Node',meq.node('MeqCondeq','eq2',children="lhs2 c2"));
    # create solver
    global rec;
    rec := meq.node('MeqSolver','solver',children="eq1 eq2");
    rec.num_steps := 10;
    rec.parm_group := hiid('Parm');
    rec.solvable := meq.solvable_list("x y");
    print mqs.meq('Create.Node',rec);

    # resolve children
    print mqs.meq('Resolve.Children',[name='solver']);

#    for( n in "eq1 lhs1 c1 a1x x" )
    for( n in "x eq1 eq2" )
      print mqs.meq('Node.Publish.Results',[name=n]);

    # execute request on x and y parms to load polcs and get original values
    global cells,request,res;
    cells := meq.cells(meq.domain(0,1,0,1),num_freq=4,num_time=4);
    request := meq.request(cells,calc_deriv=0);
    res := mqs.meq('Node.Execute',[name='x',request=request],T);
    res := mqs.meq('Node.Execute',[name='y',request=request],T);
   
    res := mqs.meq('Save.Forest',[file_name='solver_test.forest.save']);
  }
  else # stage 2: simply load the forest
  {
    res := mqs.meq('Load.Forest',[file_name='solver_test.forest.save']);
    print mqs.meq('Node.Clear.Cache',[name='solver',recursive=T]);
    print mqs.meq('Node.Publish.Results',[name='x']);
    print mqs.meq('Node.Publish.Results',[name='y']);
  }
  global stx0,stx1,sty0,sty1,xs,ys;
  
  stx0 := mqs.getnodestate('x');
  sty0 := mqs.getnodestate('y');
  if( stx0.polcs[1].coeff != 0 || sty0.polcs[1].coeff != 0 )
  {
    print '======================= stage ',stage,': init failed';
    return F;
  }
  
  # execute request on solver
  global cells,request,res;
  cells := meq.cells(meq.domain(0,1,0,1),num_freq=4,num_time=4);
  request := meq.request(cells,calc_deriv=2);
  res := mqs.meq('Node.Execute',[name='solver',request=request],T);
  print res;
  
  # get new values of x and y
  stx1 := mqs.getnodestate('x');
  sty1 := mqs.getnodestate('y');

  xs := stx1.solve_polcs[1].coeff;
  ys := sty1.solve_polcs[1].coeff;

  print sprintf("Expected values: %10.10f %10.10f",x0,y0);
  print sprintf("Original values: %10.10f %10.10f",stx0.polcs[1].coeff,sty0.polcs[1].coeff);
  print sprintf("Solution:        %10.10f %10.10f",xs,ys);

  if( abs(x0-xs) < 1e-5*abs(x0) && abs(y0-ys) < 1e-5*abs(y0) )
  {
    print '======================= stage ',stage,': solve succeeded';
    return T;
  }

  print '======================= stage ',stage,': solve failed';
  return F;
}

if( any(argv == '-runtest' ) )
{
  global use_suspend,use_nostart;
  use_suspend := use_nostart := F;
  for( stage in 0:2 )
    if( !solver_test(stage=stage,gui=F,debug=F,verbose=0) )
      exit 1;
  exit 0;
}
