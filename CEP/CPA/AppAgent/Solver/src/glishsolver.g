include 'app_proxy.g'
pragma include once

solver := function (appid='Solver',
    server='./applauncher',options="-d0 -meq:M:M:Solver",
    suspend=F,
    verbose=1)
{
  self := [=];
  public := [=];
  # init app proxy
  ret := define_app_proxy(self,public,appid,server,options,suspend,verbose);
  if( is_fail(ret) )
    fail;
  # define solver-specific methods
  const public.solve := function (solverec)
  {
    wider public;
    public.command("Solver.Add.Solution",solverec);
  }
  const public.endsolve := function (endrec=[=])
  {
    wider public;
    public.command("Solver.End.Solution",endrec);
  }
  const public.nextdomain := function ()
  {
    wider public;
    public.command("Next.Domain");
  }
  return ref public;
}

mtest := function (dosolve=F,suspend=F,verbose=1)
{
  print "Creating solver and output repeater";
  start_octopussy('./applauncher',"-d0 -meq:M:O:Solver -rpt:O:M:Repeater",
                  suspend=suspend);
  global solv;
  global rpt;
  solv := solver('Solver',verbose=verbose);
  rpt := app_proxy('Repeater',verbose=verbose);
  
  print "Starting solver";
  inputrec := [ ms_name = 'demo.MS',data_column_name = 'MODEL_DATA',
	        tile_size = 1];
  inputrec.selection :=  [ ddid_index = 1, field_index = 1, 
      channel_start_index = 1, channel_end_index = 10,
      selection_string = 'ANTENNA1 in [0:2] && ANTENNA2 in [0:2]' ];
  outputrec := [ event_map_out = [default_prefix = hiid('vis.out') ] ];
  rec := [domain_size = 3600,               # 1-hr domains
	  mep_name = 'demo.MEP',
	  gsm_name = 'demo_gsm.MEP',
	  calcuvw = F,
	  modeltype = 'LOFAR'];
  solv.init(rec,inputrec,outputrec,wait=T);
  
  print "Starting repeater"
  inputrec := [ event_map_in = [default_prefix = hiid('vis.out') ] ];
  outputrec := [ write_flags=F ];
  rpt.init([=],inputrec,outputrec,wait=T);
  
  if( dosolve )
  {
    print "Running solve";
    solv.solve([iter_step=1, niter=2, max_iter=2,
	        solvableparm="{RA,DEC,StokesI}.*", solvableflag=T,
	        peelnrs=1, prednrs=[2], when_max_iter=[=]]);
  }
  
  print "Created global app proxy objects: solv, rpt";
}

test := function (dosolve=T,suspend=F,verbose=1)
{
  print "Creating solver";
  global solv;
  solv := solver(suspend=suspend,verbose=verbose);

  print "Starting solver";
  inputrec := [ ms_name = 'demo.MS',data_column_name = 'MODEL_DATA',
	        tile_size = 1];
  inputrec.selection :=  [ ddid_index = 1, field_index = 1, 
      channel_start_index = 1, channel_end_index = 10,
      selection_string = 'ANTENNA1 in [0:2] && ANTENNA2 in [0:2]' ];
  outputrec := [ event_map_out = [default_prefix = hiid('vis.out') ] ];
  rec := [domain_size = 3600,               # 1-hr domains
	  mep_name = 'demo.MEP',
	  gsm_name = 'demo_gsm.MEP',
	  calcuvw = F,
	  modeltype = 'LOFAR'];
  solv.init(initrec=rec,input=inputrec,output=outputrec,wait=T);
  
  if( dosolve )
  {
    print "Running solve";
    solv.solve([iter_step=1, niter=2, max_iter=2,
	        solvableparm="{RA,DEC,StokesI}.*", solvableflag=T,
	        peelnrs=1, prednrs=[2], when_max_iter=[=]]);
  }
  
  print "Created global app proxy object: solv";
}
