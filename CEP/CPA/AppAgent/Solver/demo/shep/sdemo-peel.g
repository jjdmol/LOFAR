include 'sdemo-common.g'

demo(verbose=0,suspend=F,rpt=T)

solverec_peel := 
  [ iter_step=1,
    max_iter=3,
	  solvable_params="{RA,DEC,StokesI}.CP2", 
    solvable_flag=T,
    peel_index=2,
    pred_index=[],
    when_max_iter=[save_residuals=T,save_params=F,apply_peel=T] ];


solv.solve(solverec_peel,set_default=T);
