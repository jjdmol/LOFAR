# use_nostart  := T;
# use_valgrind := T;
# "--skin=helgrind --logfile=hg.meqserver";
use_valgrind_opts := [ "",
#  "--gdb-attach=yes",          # use either this...
  "--logfile=vg.meqserver",       # ...or this, not both
#  "--gdb-path=/home/oms/bin/valddd", 
  ""];
  
include 'meq/meqserver.g'

const solver_test := function (gui=T,debug_level=2,verbose=1)
{
  global mqs;
  mqs := meqserver(verbose=verbose,options="-d0 -nogw -meq:M:M:MeqServer",gui=gui);
  if( is_fail(mqs) )
  {
    print mqs;
    fail;
  }
  # set verbose debugging messages
  mqs.setdebug("MeqNode MeqForest MeqSink MeqSpigot",debug_level);
  mqs.setdebug("MeqNode MeqForest MeqSink MeqSpigot",debug_level);
  mqs.setdebug("MeqServ MeqVisHandler",debug_level);
  mqs.setdebug("MeqNode",debug_level);
  mqs.setdebug("Glish",debug_level);
  mqs.setdebug("meqserver",debug_level);
  # initialize meqserver
  mqs.init([output_col="PREDICT"],wait=T);

  # define true and initial parameter values
  x0 := 2.; xp := 0.;
  y0 := 1.; yp := 0.;

  print mqs.meq('Create.Node',meqparm('x',array(xp,1,1),config_groups='Solvable.Parm'));
  print mqs.meq('Create.Node',meqparm('y',array(yp,1,1),config_groups='Solvable.Parm'));
  cc := [ a1=1,b1=1,a2=1,b2=-1 ];
  cc.c1 := cc.a1*x0 + cc.b1*y0;
  cc.c2 := cc.a2*x0 + cc.b2*y0;
  for( f in field_names(cc) )
    print mqs.meq('Create.Node',meqparm(f,array(as_double(cc[f]),1,1)));
  print mqs.meq('Create.Node',meqnode('MeqMultiply','a1x',children="a1 x"));
  print mqs.meq('Create.Node',meqnode('MeqMultiply','a2x',children="a2 x"));
  print mqs.meq('Create.Node',meqnode('MeqMultiply','b1y',children="b1 y"));
  print mqs.meq('Create.Node',meqnode('MeqMultiply','b2y',children="b2 y"));
  print mqs.meq('Create.Node',meqnode('MeqAdd','lhs1',children="a1x b1y"));
  print mqs.meq('Create.Node',meqnode('MeqAdd','lhs2',children="a2x b2y"));
  print mqs.meq('Create.Node',meqnode('MeqCondeq','eq1',children="lhs1 c1"));
  print mqs.meq('Create.Node',meqnode('MeqCondeq','eq2',children="lhs2 c2"));
  # create solver
  global rec;
  rec := meqnode('MeqSolver','solver',children="eq1 eq2");
  rec.num_steps := 3;
  rec.solvable_parm := [ by_list=meqinitstatelist() ];
  meqaddstatelist(rec.solvable_parm.by_list,"x y",[solvable=T]); 
  meqaddstatelist(rec.solvable_parm.by_list,"*",[solvable=F]); 
  print mqs.meq('Create.Node',rec);

  # resolve children
  print mqs.meq('Resolve.Children',[name='solver']);

  global stx0,stx1,sty0,sty1,xs,ys;

  stx0 := mqs.getnodestate('x');
  sty0 := mqs.getnodestate('y');

  global cells,request,res;
  cells := meqcells(meqdomain(0,1,0,1),num_freq=4,times=[0.,0.1,0.2,0.3],time_steps=[.1,.1,.1,.1]);
  request := meqrequest(cells,calc_deriv=T);
  res := mqs.meq('Node.Execute',[name='solver',request=request],T);
  print res;

  stx1 := mqs.getnodestate('x');
  sty1 := mqs.getnodestate('y');

  xs := stx1.polcs[1].vellsets;
  ys := sty1.polcs[1].vellsets;

  print sprintf("Expected values: %10.10f %10.10f",x0,y0);
  print sprintf("Solution:        %10.10f %10.10f",xs,ys);

  if( abs(x0-xs) < 1e-5*abs(x0) && abs(y0-ys) < 1e-5*abs(y0) )
  {
    print '======================= Solve succeeded';
    return T;
  }

  print '======================= Solve failed';
  return F;
}

if( any(argv == '-runtest' ) )
{
  if( solver_test(gui=F,debug_level=0,verbose=0) )
    exit 0;
  else
    exit 1;
}
