include 'glishsolver.g'
include 'mkimg.g'

msname := 'demo.MS';
mssel := 'ANTENNA1 in [0:2] && ANTENNA2 in [0:2]';
rescol := 'CORRECTED_DATA';
solverec := [ iter_step=1, niter=2, max_iter=5,
	            solvableparm="{RA,DEC,StokesI}.*", solvableflag=T,
	            peelnrs=1, prednrs=[2], when_max_iter=[save_residuals=T] ];


# create solv and rpt objects
mtest := function (dosolve=F,verbose=1,suspend=F)
{
  print "Creating solver and output repeater";
  start_octopussy('./applauncher',"-d0 -meq:M:O:Solver -rpt:O:M:Repeater",
                  suspend=suspend);
  global solv;
  global rpt;
  
  solv := solver('Solver',verbose=verbose);
  rpt := app_proxy('Repeater',verbose=verbose);
  
  print "Starting solver";
  inputrec := [ ms_name = msname,data_column_name = 'MODEL_DATA',
	        tile_size = 1 ];
          
  inputrec.selection :=  [ ddid_index = 1, field_index = 1, 
      channel_start_index = 1, channel_end_index = 10,
      selection_string = mssel ];
  outputrec := [ event_map_out = [default_prefix = hiid('vis.out') ] ];
  rec := [domain_size = 3600,               # 1-hr domains
	  mep_name = 'demo.MEP',
	  gsm_name = 'demo_gsm.MEP',
	  calcuvw = F,
	  modeltype = 'LOFAR'];
  solv.init(rec,inputrec,outputrec,wait=T);
  
  const rpt.enable := function ()
  {
    wider rpt;
    inputrec := [ event_map_in = [default_prefix = hiid('vis.out') ] ];
    outputrec := [ write_flags=F,residuals_column='CORRECTED_DATA' ];
    return rpt.init([=],inputrec,outputrec,wait=T);
  }
  const rpt.disable := function ()
  {
    wider rpt;
    inputrec := [ event_map_in = [default_prefix = hiid('a.b') ] ];
    outputrec := [ write_flags=F ];
    return rpt.init([=],inputrec,outputrec,wait=T);
  }
  
  print "Starting repeater"
  rpt.enable();
  
  if( dosolve )
  {
    print "Running solve";
    solv.solve(solverec);
  }
}

getres := function ()
{
  tbl := table(msname);
  tbl1 := tbl.query(mssel);
  return tbl1.getcol(rescol);
}

img := function ()
{
  mkimg('demo.MS','demo.imgs',msselect=mssel, type='corrected');
}
