include 'sdemo-common.g'

demo(verbose=0,suspend=F,rpt=T)

params := "{RA,DEC,StokesI}.CP%d";

solverec_peel := 
  [ iter_step=0,
    max_iter=3,
	  solvable_params=['*',params], 
    solvable_flag=[F,T],
    peel_index=2,
    pred_index=[],
    when_max_iter=[save_residuals=T,save_params=T,apply_peel=T] ];
    
solverec_simul := 
  [ iter_step=0,
    max_iter=3,
	  solvable_params="* {RA,DEC,StokesI}.*", 
    solvable_flag=[F,T],
    when_max_iter=[save_residuals=T,save_params=T] ];
    
#peel_order := [2,4,1,3,5];
peel_order := [2,4];

# add peel solutions 
for( src in peel_order )
{
  rec := solverec_peel;
  rec.peel_index := src;
  rec.solvable_params[2] := sprintf(params,src)
  solv.solve(rec);
}

# afterwards, we can reinit the solver (thus re-reading data) and do 
# a simultaneous solution
const final_solve := function ()
{
  solv.init(wait=T);
  solv.solve(solverec_simul);
}

