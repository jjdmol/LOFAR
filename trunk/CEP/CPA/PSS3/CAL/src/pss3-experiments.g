include 'pss3-demo.g';

# This file is meant to contain the actual experiments to be done with pss3




initgsm_2source_experiments := function(fname='demo', offset=2.0)
{
    tg := gsm(spaste(fname,'.GSM'), T);
    if (is_fail(tg)) fail;
    
    arcsec_per_radian := 180.0*3600.0/pi;
    alpha0 := (10.0+26.0/60 + 36.3/3600)*15.0*pi/180;
    delta0 := 26.0*pi/180.0;
    print 'alpha0 = ', alpha0;
    print 'delta0 = ', delta0;

#    l   := 0.1/arcsec_per_radian;
#    m   := -0.3/arcsec_per_radian;
#    pos := lm_to_alpha_delta(l,m, alpha0, delta0);    
#    tg.addpointsource ('CP1', [0,1e20], [0,1e20],
#                       pos[1], pos[2], 1, 0, 0, 0);
#    print pos;


    l   := (offset+1)/arcsec_per_radian;
    m   := (offset-1)/arcsec_per_radian;
    pos := lm_to_alpha_delta(l,m, alpha0, delta0);    
    tg.addpointsource ('CP1', [0,1e20], [0,1e20],
                       pos[1], pos[2], 1, 0, 0, 0);
    print pos;

    tg.done();
    pt := parmtable(spaste(fname,'_gsm.MEP'), T);
    pt.loadgsm (spaste(fname,'.GSM'));
    pt.done();
}




demo := function(solver=0,niter=10,fname='demo',sleep=F,sleeptime=2,wait=F)
{
    global annotator;

    setparms(fname);
    setgsm(fname);
    if (0 == solver)
    {
        annotator:=solve(fname=fname, niter=niter,
                         sleep=sleep, sleeptime=sleeptime, wait=wait);
    }
    else
    {
        annotator:=solvepos(fname=fname, niter=niter);
    }
}





demogain := function(niter=10,fname='demo')
{
    setparms(fname);
    initgsm(fname);

    solvegain(niter=niter,fname=fname);
}




demo_2source_experiments := function(niter=10, offset=2)
{
    global annotator;

# Generate filename ...
    fname:= '';
    i := 1
    while( i <= (2-as_integer(log(offset)))){
        fname := spaste(fname, '0');
        i +:= 1;
    }
    fname := spaste(fname, offset, '_vis');
    print fname;
# end of filename  generation

    sleep:=F;
    sleeptime:=2;
    wait:=F;

    initgsm_2source_experiments(fname, offset=offset);

    setparms(fname);


    mssel:='';
    mkimg(spaste(fname, '.MS'), spaste(fname, '.img'), msselect=mssel,
          cellx='0.25arcsec', celly='0.25arcsec', npix=1000, type='observed');

    if (0 == solver)
    {
        annotator:=solve(fname=fname, niter=niter, ant=[0:99],
                         sleep=sleep, sleeptime=sleeptime, wait=wait);
    }
    else
    {
        annotator:=solvepos(fname=fname, niter=niter);
    }


    
}




testsolve := function()
{
    initparms();
    initgsm();
    setgsm();
    return solve(niter=10);
}





testpeel := function()
{
    initparms();
    initgsm();
    setgsm();
    peel(src=1, niter=5);
    peel(src=2, niter=5, datacol='CORRECTED_DATA', mapnr='1');
    peel(src=3, niter=5, datacol='CORRECTED_DATA', mapnr='2');
    return solve(niter=5);
}





testnsolve := function()
{
    initparms('demos');
    initgsm('demos');
    setgsm('demos');
    return solve('demos',niter=10);
}





testnpeel := function()
{
    initparms('demos');
    initgsm('demos');
    setgsm('demos');
    peel('demos', src=1, niter=5);
    peel('demos', src=2, niter=5, datacol='CORRECTED_DATA', mapnr='1');
    peel('demos', src=3, niter=5, datacol='CORRECTED_DATA', mapnr='2');
    return solve('demos', niter=5);
}





testpeel2 := function()
{
    initparms();
    initgsm();
    setgsm();
    peel(src=1, predsrc=2, niter=5);
    peel(src=2, predsrc=3, niter=5, datacol='CORRECTED_DATA', mapnr='1');
    peel(src=3, niter=5, datacol='CORRECTED_DATA', mapnr='2');
    return solve(niter=5);
}





testsolveej := function()
{
    initparms();
    initgsm();
    setej();
    return solveej(niter=20);
}





testpeelej := function()
{
    initparms();
    initgsm();
    setej();
    peelej(src=1, niter=5);
    peelej(src=2, niter=5, datacol='CORRECTED_DATA', mapnr='1');
    peelej(src=3, niter=5, datacol='CORRECTED_DATA', mapnr='2');
    return T;
#    return solveej(niter=20);
}





testpeeleja := function()
{
    initparms();
    initgsm();
    setej();
    peelej(ant=[], src=1, niter=5);
    peelej(ant=[], src=2, niter=5, datacol='CORRECTED_DATA', mapnr='1');
    peelej(ant=[], src=3, niter=5, datacol='CORRECTED_DATA', mapnr='2');
#    return T;
    return solveej(ant=[], niter=20);
}





testpeelej2 := function()
{
    initparms();
    initgsm();
    setej();
    peelej(src=1, predsrc=2, niter=5);
    peelej(src=2, predsrc=3, niter=5, datacol='CORRECTED_DATA', mapnr='1');
    peelej(src=3, niter=5, datacol='CORRECTED_DATA', mapnr='2');
    return solveej(niter=20);
}





testpeelej2a := function()
{
    initparms();
    initgsm();
    setej();
    peelej(ant=[], src=1, predsrc=2, niter=5);
    peelej(ant=[], src=2, predsrc=3, niter=5, datacol='CORRECTED_DATA', mapnr='1');
    peelej(ant=[], src=3, niter=5, datacol='CORRECTED_DATA', mapnr='2');
    return solveej(ant=[], niter=20);
}
