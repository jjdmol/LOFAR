include 'sdemo-common.g'

demo(verbose=0,suspend=F,rpt=T)

solverec_peel := 
  [ iter_step=0,
    max_iter=3,
	  solvable_params="* {RA,DEC,StokesI}.CP%d", 
    solvable_flag=[F,T],
    peel_index=2,
    pred_index=[],
    when_max_iter=[save_residuals=T,save_params=T,apply_peel=T] ];

for( src in [2,4,1,3,5] )
{
  rec := solverec_peel;
  rec.peel_index := src;
  rec.solvable_params[2] := sprintf(rec.solvable_params[2],src)
  solv.solve(rec);
}

