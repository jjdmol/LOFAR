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
#    pt.putinit ('EJ11.real.SR1.CP1', values=1);
#    pt.putinit ('EJ11.real.SR1.CP2', values=1);
#    pt.putinit ('EJ11.real.SR1.CP3', values=1);
#    pt.putinit ('EJ11.real.SR2.CP1', values=1);
#    pt.putinit ('EJ11.real.SR2.CP2', values=1);
#    pt.putinit ('EJ11.real.SR2.CP3', values=1);
#    pt.putinit ('EJ11.real.SR5.CP1', values=1);
#    pt.putinit ('EJ11.real.SR5.CP2', values=1);
#    pt.putinit ('EJ11.real.SR5.CP3', values=1);
#    pt.putinit ('EJ11.real.SR9.CP1', values=1);
#    pt.putinit ('EJ11.real.SR9.CP2', values=1);
#    pt.putinit ('EJ11.real.SR9.CP3', values=1);
    pt.putinit ('EJ11.real', values=1);
    pt.putinit ('EJ12.real', values=0);
    pt.putinit ('EJ21.real', values=0);
    pt.putinit ('EJ22.real', values=1);
    pt.putinit ('EJ11.imag', values=1);
    pt.putinit ('EJ12.imag', values=0);
    pt.putinit ('EJ21.imag', values=0);
    pt.putinit ('EJ22.imag', values=1);
    pt.done();
}

setparms := function(fname='demo')
{
    pt := parmtable(spaste(fname,'.MEP'), T);
    if (is_fail(pt)) fail;
    pt.putinit ('frot', values=0);
    pt.putinit ('drot', values=0);
    pt.putinit ('dell', values=0);
    pt.putinit ('gain.11', values=1);
    pt.putinit ('gain.22', values=0);
    pt.putinit ('EJ11.real', values=1);
    pt.putinit ('EJ12.real', values=0);
    pt.putinit ('EJ21.real', values=0);
    pt.putinit ('EJ22.real', values=1);
    pt.putinit ('EJ11.imag', values=1);
    pt.putinit ('EJ12.imag', values=0);
    pt.putinit ('EJ21.imag', values=0);
    pt.putinit ('EJ22.imag', values=1);
    pt.done();
}


# l, m, alpha0, delta0 in radians...
lm_to_alpha_delta := function(l, m, alpha0, delta0)
{
    alphadelta := [0.0, 0.0];
    alphadelta[1] := alpha0 + atan(l/(cos(delta0)*sqrt(1.0 - l*l - m*m) - m*sin(delta0) ));
    alphadelta[2] := asin(m*cos(delta0)+sin(delta0)*sqrt(1.0-l*l - m*m));
    
    if(alphadelta[1] < 0.0) {
        alphadelta[1] +:= 2*pi;
    }
    if(alphadelta[1] >= 2*pi) {
        alphadelta[1] -:= 2*pi;
    }
    return alphadelta;
}





# The positions of the sources are (in arcsec):
#
# Delta X             Delta Y
# -------             -------
# -117.42             28.38
# 34.74               -30.33
# 65.56               66.54
# 93.44               65.07
# -8.81               -56.26
#
#   Field centre = 10h26m36.300 +26:00:00.000
#   J2000.0
#
initgsm := function(fname='demo')
{
    tg := gsm(spaste(fname,'.GSM'), T);
    if (is_fail(tg)) fail;
    
    arcsec_per_radian := 180.0*3600.0/pi;
    alpha0 := (10.0+26.0/60 + 36.3/3600)*15.0*pi/180;
    delta0 := 26.0*pi/180.0;
    print 'alpha0 = ', alpha0;

    l   := 117.42/arcsec_per_radian;
    m   := 28.38/arcsec_per_radian;
    pos := lm_to_alpha_delta(l,m, alpha0, delta0);    
    tg.addpointsource ('CP1', [0,1e20], [0,1e20],
                       pos[1], pos[2], 1, 0, 0, 0);
    print pos;


    l   := -34.74/arcsec_per_radian;
    m   := -30.33/arcsec_per_radian;
    pos := lm_to_alpha_delta(l,m, alpha0, delta0);    
    tg.addpointsource ('CP2', [0,1e20], [0,1e20],
                       pos[1], pos[2], 1, 0, 0, 0);
    print pos;

    l   := -65.56/arcsec_per_radian;
    m   := 66.54/arcsec_per_radian;
    pos := lm_to_alpha_delta(l,m, alpha0, delta0);    
    tg.addpointsource ('CP3', [0,1e20], [0,1e20],
                       pos[1], pos[2], 1, 0, 0, 0);
    print pos;

    l   := -93.44/arcsec_per_radian;
    m   := 65.07/arcsec_per_radian;
    pos := lm_to_alpha_delta(l,m, alpha0, delta0);    
    tg.addpointsource ('CP4', [0,1e20], [0,1e20],
                       pos[1], pos[2], 1, 0, 0, 0);
    print pos;

    l   := 8.81/arcsec_per_radian;
    m   := -56.26/arcsec_per_radian;
    pos := lm_to_alpha_delta(l,m, alpha0, delta0);    
    tg.addpointsource ('CP5', [0,1e20], [0,1e20],
                       pos[1], pos[2], 1, 0, 0, 0);
    print pos;



    tg.done();
    pt := parmtable(spaste(fname,'_gsm.MEP'), T);
    pt.loadgsm (spaste(fname,'.GSM'));
    pt.done();
}






setej := function(fname='demo', mode=1, fact=1)
{
    pt := parmtable(spaste(fname,'.MEP'));
#  pt.perturb ('NAME==pattern("EJ11.real*")', 0.5, F);
    pt.perturb ('NAME==pattern("EJ11.imag*")', -0.5, F);
#  pt.perturb ('NAME==pattern("EJ11.SR{1,5,9}.CP{1}")', 0.5*fact, F);
    if (mode==2) {
#      pt.perturb ('NAME==pattern("EJ11.SR2.CP1")', 0.5000015, F);
#      pt.perturb ('NAME==pattern("EJ11.SR2.CP2")', 0.5000015, F);
        pt.perturb ('NAME==pattern("EJ11.SR2.CP3")', 0.5000015, F);
    }
    pt.done();
}



setgsm := function(fname='demo')
{
    initgsm(fname);
    
#    pt := parmtable(spaste(fname,'_gsm.MEP'));
#    pt.perturb ('NAME=="RA.CP1"', 0.00003, F);
#    pt.perturb ('NAME=="RA.CP2"', 0.000005, F);
#    pt.perturb ('NAME=="RA.CP3"', 0.00001, F);
#    pt.perturb ('NAME=="DEC.CP1"', -0.000005, F);
#    pt.perturb ('NAME=="DEC.CP2"', -0.0000025, F);
#    pt.perturb ('NAME=="DEC.CP3"', 0.0000025, F);
#    pt.perturb ('NAME=="StokesI.CP1"', 0, F);
#    pt.perturb ('NAME=="StokesI.CP2"', 0.5, F);
#    pt.perturb ('NAME=="StokesI.CP3"', 0.7, F);
#    pt.done();
}

initsolution := function(fname='demo')
{
  initparms(fname);
  initgsm(fname);
}
