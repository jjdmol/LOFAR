include 'widgetserver.g'
include 'glishsolver.g'
include 'mkimg.g'
include 'inittables.g'

print shell('killall -9 applauncher');

# init the GSM and MEP tables
filename:='noordam';
initsolution(filename);
msname := spaste(filename,'.MS');

mssel := 'ANTENNA1 in 4*[0:20] && ANTENNA2 in 4*[0:20]';

datacol := 'DATA';
rescol := 'CORRECTED_DATA';
predcol := 'MODEL_DATA';

selection := [ ddid_index=1,field_index=1,
               selection_string=mssel ];

inputrec := [ ms_name = msname,
              data_column_name = datacol,
              tile_size = 1, selection = selection ];
  
outputrec := 
  [ write_flags=F,
    residuals_column=rescol,predict_column=predcol ];

solver_initrec := 
  [ domain_size = 3600,               # 1-hr domains
	  mep_name = 'noordam.MEP',
	  gsm_name = 'noordam_gsm.MEP',
	  calcuvw = F,
	  modeltype = 'LOFAR.RI' ];
  
solverec_simul := 
  [ iter_step=0,
    max_iter=5,
	  solvable_params="{RA,DEC,StokesI}.*", 
    solvable_flag=T,
    when_max_iter=[save_residuals=T,save_params=F,apply_peel=F] ];

solverec_peel := 
  [ iter_step=0,
    max_iter=3,
	  solvable_params="{RA,DEC,StokesI}.CP2", 
    solvable_flag=T,
    peel_index=2,
    pred_index=[],
    when_max_iter=[save_residuals=T,save_params=F,apply_peel=T] ];

# create solv and rpt objects
demo := function (dosolve=F,verbose=1,suspend=F,rpt=T)
{
  print "Creating solver and output repeater";
  opts := "-d0 -meq:M:O:Solver"
  if( rpt )
    opts := [opts,"-rpt:O:M:Repeater"];
  start_octopussy('./applauncher',opts,suspend=suspend);
  global solv;
  global rpt;
  
  parent := dws.frame(title='Solver demo',side='left');
  parent->unmap();
  
  solv := solver('Solver',verbose=verbose,parent_frame=parent);
  if( is_fail(solv) )
    fail;
  rpt := app_proxy('Repeater',verbose=verbose,parent_frame=parent);
  if( is_fail(rpt) )
    fail;
  
  parent->map();
  
  print "Starting solver";
  solver_outputrec := [ event_map_out = [default_prefix = hiid('vis.out') ] ];
  
  solv.init(solver_initrec,inputrec,solver_outputrec,wait=T);
  
  const rpt.enable := function ()
  {
    wider rpt;
    rpt_inputrec := [ event_map_in = [default_prefix = hiid('vis.out') ] ];
    return rpt.init([=],rpt_inputrec,outputrec,wait=F);
  }
  const rpt.disable := function ()
  {
    wider rpt;
    rpt_inputrec := [ event_map_in = [default_prefix = hiid('a.b') ] ];
    return rpt.init([=],rpt_inputrec,outputrec,wait=F);
  }
  
  print "Starting repeater"
  rpt.enable();
  
  solv.solve(solverec_simul,set_default=T);
  solv.endsolve([save_params=F,save_residuals=T,apply_peel=T],set_default=T);
  
  global img_pred,img_res;
#  whenever solv->end_solution do
#  {
#    print "end of solution, making predicted image";
#    img_pred := make_image(predict=T,redo=T);
#    print "making residual image";
#    img_res := make_image(residual=T,redo=T);
#  }
}

getcols := function ()
{
  tbl := table(msname);
  tbl1 := tbl.query(mssel);
  print tbl1.getcol(rescol);
  print tbl1.getcol(predcol);
}

make_image := function (number=0,data=F,predict=F,residual=F,redo=F)
{
  if( data )
  { type:='observed'; suf:='obs'; }
  else if( predict )
  { type := 'model'; suf:='pred'; }
  else if( residual )
  { type := 'corrected'; suf:='res'; }
  else
    fail 'must specify one of: data, predict, residual'
  
  imgname := spaste(filename,'.img.',suf,number);
  
  if( !redo )
  {
    img := image(imgname);
    if( !is_fail(img) )
    {
      print 'Image file already exists and redo=F, reusing';
      img.view();
      return ref img;
    }
  }
  
  img := mkimg(msname,imgname,msselect=mssel,type=type,
               cellx='0.25arcsec',
               celly='0.25arcsec',
               npix=1000)
  return ref img;
}

# img0 := make_image(0,data=T,redo=F);
