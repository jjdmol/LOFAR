include 'pss3-demo.g';

# This file is meant to contain the actual experiments to be done with pss3





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
