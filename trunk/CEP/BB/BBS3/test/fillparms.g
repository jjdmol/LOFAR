include 'table.g';
include 'parmtable.g';
include 'gsm.g';


tohms := function (rad)
{
    hh := rad*180/pi/15;
    mm := 60 * (hh - as_integer(hh));
    ss := 60 * (mm - as_integer(mm));
    print spaste(rad,' rad = ',as_integer(hh),'h',as_integer(mm),'m',ss);
}

todms := function (rad)
{
    dd := rad*180/pi;
    mm := 60 * (dd - as_integer(dd));
    ss := 60 * (mm - as_integer(mm));
    print spaste(rad,' rad = ',as_integer(dd),':',as_integer(mm),':',ss);
}

fromhms := function(hh,mm,ss)
{
    return (hh + mm/60. + ss/3600.)*pi*15/180;
}
fromdms := function(dd,mm,ss)
{
    return (dd + mm/60. + ss/3600.)*pi/180;
}



initparms := function(fname='demo', pert=1e-6)
{
    pt := parmtable(spaste(fname,'.MEP'), T);
    if (is_fail(pt)) fail;
    pt.putinit ('frot', values=0, diff=pert);
    pt.putinit ('drot', values=0, diff=pert);
    pt.putinit ('dell', values=0, diff=pert);
    pt.putinit ('gain.11', values=1, diff=pert);
    pt.putinit ('gain.22', values=1, diff=pert);
    pt.putinit ('EJ11.real', values=1, diff=pert);
    pt.putinit ('EJ12.real', values=0, diff=pert);
    pt.putinit ('EJ21.real', values=0, diff=pert);
    pt.putinit ('EJ22.real', values=1, diff=pert);
    pt.putinit ('EJ11.imag', values=1, diff=pert);
    pt.putinit ('EJ12.imag', values=0, diff=pert);
    pt.putinit ('EJ21.imag', values=0, diff=pert);
    pt.putinit ('EJ22.imag', values=1, diff=pert);
    pt.done();
}


initparmsp := function(fname='demo3p', pert=1e-6)
{
    pt := parmtable(spaste(fname,'.MEP'), T);
    if (is_fail(pt)) fail;
    pt.putinit ('frot', values=0, diff=pert);
    pt.putinit ('drot', values=0, diff=pert);
    pt.putinit ('dell', values=0, diff=pert);
    pt.putinit ('gain.11', values=1, diff=pert);
    pt.putinit ('gain.22', values=1, diff=pert);
    pt.putinit ('EJ11.real', values=array([1,0.01],2,1),
		normalize=F, time0=2.35209e+09-1168, diff=pert);
    pt.putinit ('EJ12.real', values=0, diff=pert);
    pt.putinit ('EJ21.real', values=0, diff=pert);
    pt.putinit ('EJ22.real', values=1, diff=pert);
    pt.putinit ('EJ11.imag', values=1, diff=pert);
    pt.putinit ('EJ12.imag', values=0, diff=pert);
    pt.putinit ('EJ21.imag', values=0, diff=pert);
    pt.putinit ('EJ22.imag', values=1, diff=pert);
    pt.done();
}


initgsm := function(fname='demo', pert=1e-6)
{
    tg := gsm(spaste(fname,'.GSM'), T);
    if (is_fail(tg)) fail;
    tg.addpointsource ('CP1', [0,1e20], [0,1e20],
                       2.734, 0.45379, 1, 0, 0, 0);
    tg.addpointsource ('CP2', [0,1e20], [0,1e20],
                       2.73402, 0.45369, 0.5, 0, 0, 0);
    tg.addpointsource ('CP3', [0,1e20], [0,1e20],
                       2.73398, 0.45375, 0.3, 0, 0, 0);
    tg.done();
    pt := parmtable(spaste(fname,'_gsm.MEP'), T);
    pt.loadgsm (spaste(fname,'.GSM'), diff=pert);
    pt.done();
}


initgsm10 := function(fname='demo10', pert=1e-6)
{
    tg := gsm(spaste(fname,'.GSM'), T);
    if (is_fail(tg)) fail;
    tg.addpointsource ('CP1', [0,1e20], [0,1e20],
                       fromhms(10,26,35.2), fromdms(26,0,0.87), 1.5, 0, 0, 0);
    tg.addpointsource ('CP2', [0,1e20], [0,1e20],
                       fromhms(10,26,35.2), fromdms(26,0,22.0), 1.0, 0, 0, 0);
    tg.addpointsource ('CP3', [0,1e20], [0,1e20],
                       fromhms(10,26,37.2), fromdms(25,59,50.), 0.7, 0, 0, 0);
    tg.addpointsource ('CP4', [0,1e20], [0,1e20],
                       fromhms(10,26,35.0), fromdms(25,59,40.), 0.5, 0, 0, 0);
    tg.addpointsource ('CP5', [0,1e20], [0,1e20],
                       fromhms(10,26,36.1), fromdms(26,0,10.0), 0.3, 0, 0, 0);
    tg.addpointsource ('CP6', [0,1e20], [0,1e20],
                       fromhms(10,26,36.0), fromdms(26,0,0.00), 0.3, 0, 0, 0);
    tg.addpointsource ('CP7', [0,1e20], [0,1e20],
                       fromhms(10,26,37.5), fromdms(25,59,55.), 0.2, 0, 0, 0);
    tg.addpointsource ('CP8', [0,1e20], [0,1e20],
                       fromhms(10,26,34.7), fromdms(25,59,50.), 0.1, 0, 0, 0);
    tg.addpointsource ('CP9', [0,1e20], [0,1e20],
                       fromhms(10,26,37.0), fromdms(26,0,15.0), 0.1, 0, 0, 0);
    tg.addpointsource ('CP10', [0,1e20], [0,1e20],
                       fromhms(10,26,35.7), fromdms(26,0,0.80), 0.08, 0, 0, 0);
    tg.done();
    pt := parmtable(spaste(fname,'_gsm.MEP'), T);
    pt.loadgsm (spaste(fname,'.GSM'), diff=pert);
    pt.done();
}






setej := function(fname='demo', mode=1, fact=1)
{
    pt := parmtable(spaste(fname,'.MEP'));
    pt.perturb ('NAME==pattern("EJ11.real*")', 0.5, F);
    pt.done();
}






setgsm := function(fname='demo')
{
#  tg := gsm(spaste(fname,'.GSM'), T);
#  if (is_fail(tg)) fail;
#  tg.addpointsource ('CP1', [0,1e20], [0,1e20],
#		     2.734030, 0.453785, 1, 0, 0, 0);
#  tg.addpointsource ('CP2', [0,1e20], [0,1e20],
#		     2.734025, 0.4536875, 1, 0, 0, 0);
#  tg.addpointsource ('CP3', [0,1e20], [0,1e20],
#		     2.73399, 0.4537525, 1, 0, 0, 0);
#  tg.done();
    pt := parmtable(spaste(fname,'_gsm.MEP'));
    pt.perturb ('NAME=="RA.CP1"', 0.000003, F);
    pt.perturb ('NAME=="RA.CP2"', 0.0000005, F);
    pt.perturb ('NAME=="RA.CP3"', 0.000001, F);
    pt.perturb ('NAME=="DEC.CP1"', -0.0000005, F);
    pt.perturb ('NAME=="DEC.CP2"', -0.00000025, F);
    pt.perturb ('NAME=="DEC.CP3"', 0.00000025, F);
    pt.perturb ('NAME=="StokesI.CP1"', 0, F);
    pt.perturb ('NAME=="StokesI.CP2"', 0.5, F);
    pt.perturb ('NAME=="StokesI.CP3"', 0.7, F);
    pt.done();
}

setgsmall := function(fname='demo')
{
    pt := parmtable(spaste(fname,'_gsm.MEP'));
    pt.perturb ('NAME~m/RA.*/', 0.0000005, F);
    pt.perturb ('NAME~m/DEC.*/', -0.0000005, F);
    pt.perturb ('NAME~m/StokesI.*/', 0.1, F);
    pt.done();
}
