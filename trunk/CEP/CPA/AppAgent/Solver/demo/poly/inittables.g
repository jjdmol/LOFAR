pragma include once
include 'parmtable.g'
include 'gsm.g'

initparms := function(fname='demo')
{
  pt := parmtable(spaste(fname,'.MEP'), T);
  if (is_fail(pt)) fail;
  pt.putinit ('frot', values=0);
  pt.putinit ('drot', values=0);
  pt.putinit ('dell', values=0);
  pt.putinit ('gain.11', values=1);
  pt.putinit ('gain.22', values=1);
  for (src in [1:3]) {
      for (stat in 1+4*[0:20]) {
	  pt.put (spaste('EJ11.ampl.SR',stat,'.CP',src),
		  srcnr=src, statnr=stat,
		  values=array([2,0.5,0.6,-1,-0.5,-0.8], 3,2),
		  time0=2352088832,
		  timerange=[2352088831,2352088831+10],
		  freqrange=[137500000,162500000],
		  normalize=T);
      }
  }	  
#  pt.putinit ('EJ11.ampl', values=array([1, 0.01, -0.000015,
#					    6.e-9, 7.e-11, 1.e-13],
#					    3,2),
#	      time0=2352088832);
#  pt.putinit ('EJ11.ampl', values=1);
  pt.putinit ('EJ12.ampl', values=1);
  pt.putinit ('EJ21.ampl', values=1);
  pt.putinit ('EJ22.ampl', values=1);
  pt.putinit ('EJ11.phase', values=0);
  pt.putinit ('EJ12.phase', values=0);
  pt.putinit ('EJ21.phase', values=0);
  pt.putinit ('EJ22.phase', values=0);
  pt.done();
}

initgsm := function(fname='demo')
{
  tg := gsm(spaste(fname,'.GSM'), T);
  if (is_fail(tg)) fail;
  tg.addpointsource ('CP1', [0,1e20], [0,1e20],
		     2.734, 0.45379, 1, 0, 0, 0);
  tg.addpointsource ('CP2', [0,1e20], [0,1e20],
		     2.73402, 0.45369, 0.5, 0, 0, 0);
  tg.addpointsource ('CP3', [0,1e20], [0,1e20],
		     2.73398, 0.45375, 0.3, 0, 0, 0);
  tg.done()
  pt := parmtable(spaste(fname,'_gsm.MEP'), T);
  pt.loadgsm (spaste(fname,'.GSM'));
  pt.done();
}

setej := function(fname='demo', mode=1, fact=1)
{
  pt := parmtable(spaste(fname,'.MEP'));
#  pt.perturb ('NAME==pattern("EJ11.ampl.SR5.*")', 0.1, F);
  pt.perturb ('NAME==pattern("EJ11.ampl.*")', 1, F);
#  pt.perturb ('NAME==pattern("EJ11.phase.*")', -0.1, F);
#  pt.perturb ('NAME==pattern("EJ11.ampl.SR81.*")', 0, F);
#  pt.perturb ('NAME==pattern("EJ11.phase.SR81.*")', 0, F);
#  pt.perturb ('NAME==pattern("EJ11.ampl.SR{1,5,9,13}.*")', 0.1, F);
#  pt.perturb ('NAME==pattern("EJ11.phase.SR{1,5,9,13}.*")', -0.1, F);
#  pt.perturb ('NAME==pattern("EJ11.SR{1,5,9}.CP{1}")', 0.5*fact, F);
  if (mode==2) {
#      pt.perturb ('NAME==pattern("EJ11.SR2.CP1")', 0.5000015, F);
#      pt.perturb ('NAME==pattern("EJ11.SR2.CP2")', 0.5000015, F);
      pt.perturb ('NAME==pattern("EJ11.SR2.CP3")', 0.5000015, F);
  }
  pt.done();
}

initsolution := function(fname='demo')
{
  initparms(fname);
  initgsm(fname);
  setej(fname);
}
