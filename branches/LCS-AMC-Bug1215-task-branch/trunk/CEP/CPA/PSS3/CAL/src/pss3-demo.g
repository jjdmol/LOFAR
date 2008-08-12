include 'table.g';
include 'meqcalibrater.g';
include 'parmtable.g';
include 'gsm.g';
include 'imgannotator.g';
include 'mkimg.g';

# This file is meant to contain utility functions used in the
# experiments. The experiments are to be placed in pss3-experiments.g


#
# Demo function showing the predict functionality and creating an image of it.
#
predict := function(fname='demo', ant=4*[0:20],
                    modeltype='LOFAR', calcuvw=F, trace=T)
{

    local mc := meqcalibrater(spaste(fname,'.MS'), 
                              fname, spaste(fname,'_gsm'),
                              ant=ant, modeltype=modeltype, calcuvw=calcuvw);
    if (is_fail(mc)) {
        print "meqcalibratertest(): could not instantiate meqcalibrater";
        fail;
    }
    
    mc.settimeinterval(3600); # calibrate per 1 hour
    mc.clearsolvableparms();
    
    mc.resetiterator();
    while (mc.nextinterval())
    {
        d := mc.getsolvedomain();
        print 'solvedomain = ', d;
        
        mc.predict('MODEL_DATA');
#	mc.saveresidualdata();
    }

    mc.done();

    mssel := '';
    if (len(ant) > 0) {
        ant +:=1;               # msselect adds 1 to ANTENNA1,2
        mssel := spaste('all([ANTENNA1,ANTENNA2] in ',substitutevar(ant), ')');
    }
    print mssel;
    mkimg(spaste(fname, '.MS'), spaste(fname, '.img'), msselect=mssel,
          cellx='1arcsec', celly='1arcsec', npix=1000, type='model');

    return T;
}






#
#   Solve for positions and fluxes of sources
#
solve := function(fname='demo', ant=4*[0:20],
                  modeltype='LOFAR', calcuvw=F, 
                  niter=1, sleep=F, sleeptime=2, wait=F,
                  solve_fluxes=T)
{
    annotator := imgannotator(spaste(fname, '.img'), 'raster');
    
    # Plot correct position, initial flux magnitude and final flux magnitude
    # First get the values used in the simulation.
    t  := table(spaste(fname,'_gsm.MEP/DEFAULTVALUES'));
    t1 := t.query('NAME==pattern("{RA,DEC,StokesI}.*")',
                  sortlist='SRCNR,NAME');
    t.close();
    if (t1.nrows() == 0) fail "No sources in MEP table";
    if (t1.nrows()%3 != 0) fail "_gsm.MEP inconsistent for RA,DEC,StokesI";
    # MEP is sorted, thus order is DEC,RA,StokesI
    annotator.hold();
    for (i in [1:(t1.nrows()/3)]) {
        dec := t1.getcell("SIM_VALUES", 3*i-2)[1,1];
        ra  := t1.getcell("SIM_VALUES", 3*i-1)[1,1];
        stk := t1.getcell("SIM_VALUES", 3*i)[1,1];
        print stk;
        src_mrk[i] := annotator.add_marker(1, ra, dec, F, F, 1,
                                           as_integer(100*stk), 3);
        annotator.add_marker(2, ra, dec, F, F, 1, as_integer(100*stk), 12);
    }
    annotator.release();
    t1.close();

    #
    # Create calibrater object
    #
    mc := meqcalibrater(spaste(fname,'.MS'), fname, spaste(fname,'_gsm'),
                        ant=ant,datacolname='DATA',
                        modeltype=modeltype, calcuvw=calcuvw);

    if (wait)
    {
        print "Press RETURN to continue.";
        shell("read");
    }

    if (is_fail(mc)) {
        print "meqcalibratertest(): could not instantiate meqcalibrater";
        fail;
    }

#    mc.select('', 5, 5);

    # Plot initial position
    ss := mc.getparmnames();
    print len(ss),'parmnames:',ss[1:10],'...';           # rather long!

    note('mc.settimeinterval()');
    mc.settimeinterval(3600); # calibrate per 3600 seconds
    note('mc.clearsolvableparms()');
    mc.clearsolvableparms();

    if(solve_fluxes){
        note('mc.setsolvableparms("StokesI.*")');    
        mc.setsolvableparms("StokesI.*");
    }
    note('mc.setsolvableparms("DEC.* RA.*")');
    mc.setsolvableparms("DEC.* RA.*");
    #mc.setsolvableparms("Leakage.{11,22}.*");

    solverec := [=];

    sleep_cmd := spaste('sleep ', sleeptime);
    if (sleep) shell(sleep_cmd);

    note('mc.resetiterator()');
    mc.resetiterator();
    
    note('while(mc.nextinterval())');
    while (mc.nextinterval())
    {
        note('mc.getsolvedomain()');
        d := mc.getsolvedomain();
        print 'solvedomain = ', d;
        
        parms := mc.getparms("RA.* DEC.* StokesI.*");
        print parms;
        print len(parms);
        nrpos := len(parms) / 3;
        if (nrpos > 0) {
#            annotator.hold();
            for (i in [1:nrpos]) {
                ra      := parms[spaste('RA.CP',i)].value[1];
                dec     := parms[spaste('DEC.CP',i)].value[1];
                stokesI := parms[spaste('StokesI.CP',i)].value[1];
                print 'src = ', i, ' ra = ', ra, ' dec = ', dec,
                    ' I = ', stokesI;

#                annotator.change_marker_size(src_mrk[i], stokesI*100);
#                if (is_fail(annotator)) fail;
#                annotator.add_marker(i, real(ra), real(dec), i==nrpos);
            }
#            annotator.release();
        }
        
        for (i in [1:niter]) {
            print "iteration", i;

            srec := mc.solve();
            solverec[spaste("iter",i)] := srec;
            
            parms := mc.getparms("RA.* DEC.* StokesI.*");
            nrpos := len(parms) / 3;
            if (nrpos > 0) {
#                annotator.hold();
                for (j in [1:nrpos]) {
                    ra  := parms[spaste('RA.CP',j)].value[1];
                    dec := parms[spaste('DEC.CP',j)].value[1];
                    stokesI := parms[spaste('StokesI.CP',j)].value[1];
                    print 'src = ', j, ' ra = ', ra, ' dec = ', dec,
                        ' I = ', stokesI;
                    
#                    annotator.change_marker_size(src_mrk[j], stokesI*100);
#                    if (is_fail(annotator)) fail;
#                    annotator.add_marker(j, real(ra), real(dec), j==nrpos);
                }
#                annotator.release();
            }
            sleep_cmd := spaste('sleep ', sleeptime);
            if (sleep) shell(sleep_cmd);
        }
        print mc.getstatistics();
        mc.saveresidualdata();
        mc.saveparms();
    }
    
    mc.done();

    mssel := '';
    if (len(ant) > 0) {
        ant +:=1;               # msselect adds 1 to ANTENNA1,2
        mssel := spaste('all([ANTENNA1,ANTENNA2] in ',substitutevar(ant), ')');
    }
    mkimg(spaste(fname, '.MS'), npix=1000, cellx='1arcsec', celly='1arcsec', 
          spaste(fname, '.imgs', src[1]),
          msselect=mssel, type='corrected');

    return solverec;    
#    return ref annotator;
}








solveej := function(fname='demo', ant=4*[0:20],
                    modeltype='LOFAR', calcuvw=F, 
                    niter=1, sleep=F, sleeptime=2, wait=F)
{
    #
    # Create calibrater object
    #
    mc := meqcalibrater(spaste(fname,'.MS'), fname, spaste(fname,'_gsm'),
                        ant=ant,
                        modeltype=modeltype, calcuvw=calcuvw);

    if (wait)
    {
        print "Press RETURN to continue.";
        shell("read");
    }

    if (is_fail(mc)) {
        print "meqcalibratertest(): could not instantiate meqcalibrater";
        fail;
    }

    mc.settimeinterval(3600); # calibrate per 3600 seconds
    mc.clearsolvableparms();
#    mc.setsolvableparms(parmpatterns="EJ11.phase.*", excludepatterns="EJ11.phase.SR1.CP1");
    mc.setsolvableparms(parmpatterns="EJ11.phase.SR23.CP1");

#    mc.setsolvableparms("EJ11.SR{1,5,9,13,17,21,25,29,33,37}.*");
#    print mc.getparmnames();

    solverec := [=];

    sleep_cmd := spaste('sleep ', sleeptime);
    if (sleep) shell(sleep_cmd);

    mc.resetiterator();
    while (mc.nextinterval())
    {
        d := mc.getsolvedomain();
        print 'solvedomain = ', d;
        
        parms := mc.getparms("EJ11.{ampl,phase}.*.*");
        nrp := len(parms);
        if (nrp > 0) {
            for (nm in field_names(parms)) {
                print nm, '=', parms[nm].value;
            }
        }
        
        for (i in [1:niter]) {
            print "iteration", i;
            
            srec := mc.solve();
            solverec[spaste("iter",i)] := srec;
            
            parms := mc.getparms("EJ11.{ampl,phase}.*.*");
            nrp := len(parms);
            if (nrp > 0) {
                for (nm in field_names(parms)) {
                    print nm, '=', parms[nm].value;
                }
            }
            
            sleep_cmd := spaste('sleep ', sleeptime);
            if (sleep) shell(sleep_cmd);
        }
        print mc.getstatistics();
        mc.saveresidualdata();
        mc.saveparms();
    }
    
    mc.done();

    mssel := '';
    if (len(ant) > 0) {
        ant +:=1;               # msselect adds 1 to ANTENNA1,2
        mssel := spaste('all([ANTENNA1,ANTENNA2] in ',substitutevar(ant), ')');
    }
#    mkimg(spaste(fname, '.MS'), spaste(fname, '.imgs', src[1]),
#          msselect=mssel, type='corrected');

    return solverec;    
#    return ref annotator;
}









peel := function(fname='demo', src=1, predsrc=[], ant=4*[0:20],
                 datacol='MODEL_DATA', mapnr='',
                 modeltype='LOFAR', calcuvw=F, 
                 niter=1, sleep=F, sleeptime=2, wait=F)
{
    annotator := imgannotator(spaste(fname, '.img', mapnr), 'raster');
    
    # Plot correct position, initial flux magnitude and final flux magnitude
    # First get the values used in the simulation.
    t := table(spaste(fname,'_gsm.MEP/DEFAULTVALUES'));
    t1 := t.query('NAME==pattern("{RA,DEC,StokesI}.*")',
                  sortlist='SRCNR,NAME');
    t.close();
    if (t1.nrows() == 0) fail "No sources in MEP table";
    if (t1.nrows()%3 != 0) fail "_gsm.MEP inconsistent for RA,DEC,StokesI";
    # MEP is sorted, thus order is DEC,RA,StokesI
    annotator.hold();
    for (i in [1:(t1.nrows()/3)]) {
        dec := t1.getcell("SIM_VALUES", 3*i-2)[1,1];
        ra  := t1.getcell("SIM_VALUES", 3*i-1)[1,1];
        stk := t1.getcell("SIM_VALUES", 3*i)[1,1];
        print stk;
        src_mrk[i] := annotator.add_marker(1, ra, dec, F, F, 1,
                                           as_integer(100*stk), 3);
        annotator.add_marker(2, ra, dec, F, F, 1, as_integer(100*stk), 12);
    }
    annotator.release();
    t1.close();

    #
    # Create calibrater object
    #
    mc := meqcalibrater(spaste(fname,'.MS'), fname, spaste(fname,'_gsm'),
                        ant=ant, datacolname=datacol,
                        modeltype=modeltype, calcuvw=calcuvw);

    if (wait)
    {
        print "Press RETURN to continue.";
        shell("read");
    }

    if (is_fail(mc)) {
        print "meqcalibratertest(): could not instantiate meqcalibrater";
        fail;
    }

#    mc.select('', 5, 5);

    mc.settimeinterval(3600); # calibrate per 3600 seconds
    mc.clearsolvableparms();
    for (i in src) {
        mc.setsolvableparms(spaste('StokesI.CP', i));
        mc.setsolvableparms(spaste('RA.CP', i));
        mc.setsolvableparms(spaste('DEC.CP', i));
    }

    solverec := [=];

    sleep_cmd := spaste('sleep ', sleeptime);
    if (sleep) shell(sleep_cmd);

    mc.resetiterator()
        while (mc.nextinterval())
        {
            mc.peel (src-1, predsrc-1);

            d := mc.getsolvedomain();
            print 'solvedomain = ', d;

            parms := mc.getparms("RA.* DEC.* StokesI.*");
            print parms
                print len(parms)
                    nrpos := len(parms) / 3;
            if (nrpos > 0) {
                annotator.hold();
                for (i in [1:nrpos]) {
                    ra      := parms[spaste('RA.CP',i)].value[1];
                    dec     := parms[spaste('DEC.CP',i)].value[1];
                    stokesI := parms[spaste('StokesI.CP',i)].value[1];
                    print 'src = ', i, ' ra = ', ra, ' dec = ', dec,
                        ' I = ', stokesI;

                    annotator.change_marker_size(src_mrk[i], stokesI*100);
                    if (is_fail(annotator)) fail;
                    annotator.add_marker(i, real(ra), real(dec), i==nrpos);
                }
                annotator.release();
            }
            
            for (i in [1:niter]) {
                print "iteration", i;

                srec := mc.solve();
                solverec[spaste("iter",i)] := srec;
                
                parms := mc.getparms("RA.* DEC.* StokesI.*");
                nrpos := len(parms) / 3;
                if (nrpos > 0) {
                    annotator.hold();
                    for (j in [1:nrpos]) {
                        ra  := parms[spaste('RA.CP',j)].value[1];
                        dec := parms[spaste('DEC.CP',j)].value[1];
                        stokesI := parms[spaste('StokesI.CP',j)].value[1];
                        print 'src = ', j, ' ra = ', ra, ' dec = ', dec,
                            ' I = ', stokesI;
                        
                        annotator.change_marker_size(src_mrk[j], stokesI*100);
                        if (is_fail(annotator)) fail;
                        annotator.add_marker(j, real(ra), real(dec), j==nrpos);
                    }
                    annotator.release();
                }
                sleep_cmd := spaste('sleep ', sleeptime);
                if (sleep) shell(sleep_cmd);
            }
            print mc.getstatistics();
            mc.saveresidualdata();
            mc.saveparms();
        }
    
    mc.done();
    mssel := '';
    if (len(ant) > 0) {
        ant +:=1;               # msselect adds 1 to ANTENNA1,2
        mssel := spaste('all([ANTENNA1,ANTENNA2] in ',substitutevar(ant), ')');
    }
    mkimg(spaste(fname, '.MS'), spaste(fname, '.img', src[1]),
          msselect=mssel, type='corrected');
    return T;
#    return ref annotator;
}








peelej := function(fname='demo', src=1, predsrc=[], ant=4*[0:20],
                   datacol='MODEL_DATA', mapnr='',
                   modeltype='LOFAR', calcuvw=F, 
                   niter=1, sleep=F, sleeptime=2, wait=F)
{
    #
    # Create calibrater object
    #
    mc := meqcalibrater(spaste(fname,'.MS'), fname, spaste(fname,'_gsm'),
                        ant=ant, datacolname=datacol,
                        modeltype=modeltype, calcuvw=calcuvw);

    if (wait)
    {
        print "Press RETURN to continue.";
        shell("read");
    }

    if (is_fail(mc)) {
        print "meqcalibratertest(): could not instantiate meqcalibrater";
        fail;
    }

#    mc.select('', 5, 5);

    mc.settimeinterval(3600); # calibrate per 3600 seconds
    mc.clearsolvableparms();
    for (i in src) {
        mc.setsolvableparms(spaste('EJ11.*.CP', src, '*'));
#	mc.setsolvableparms(spaste('EJ11.SR{5,9,13,17,21,25,29,33,37}.CP', src));
    }
    solverec := [=];

    sleep_cmd := spaste('sleep ', sleeptime);
    if (sleep) shell(sleep_cmd);

    mc.resetiterator()
        while (mc.nextinterval())
        {
            mc.peel (src-1, predsrc-1);

            d := mc.getsolvedomain();
            print 'solvedomain = ', d;

            parms := mc.getparms("EJ11.{real,imag}.SR{5,9}.*");
#        parms := mc.getparms("EJ*");
            print parms
                
                for (i in [1:niter]) {
                    print "iteration", i;

                    srec := mc.solve();
                    solverec[spaste("iter",i)] := srec;
                    
                    for (j in src) {
                        parms := mc.getparms("EJ11.{real,imag}.SR{5,9}.*");
#		parms := mc.getparms(spaste('EJ*.CP', src, '*'));
                        nrp := len(parms);
                        if (nrp > 0) {
                            for (nm in field_names(parms)) {
                                print nm, '=', parms[nm].value;
                            }
                        }
                    }
                    sleep_cmd := spaste('sleep ', sleeptime);
                    if (sleep) shell(sleep_cmd);
                }
            print mc.getstatistics();
            mc.saveresidualdata();
            mc.saveparms();
        }
    
    mc.done();
    mssel := '';
    if (len(ant) > 0) {
        ant +:=1;               # msselect adds 1 to ANTENNA1,2
        mssel := spaste('all([ANTENNA1,ANTENNA2] in ',substitutevar(ant), ')');
    }
    mkimg(spaste(fname, '.MS'), spaste(fname, '.img', src[1]),
          msselect=mssel, type='corrected');
    return T;
#    return ref annotator;
}








solvepos := function(fname='demo', ant=4*[0:20], niter=1)
{
    annotator := imgannotator(spaste(fname, '.img'), 'raster');
    
    mc := meqcalibrater(spaste(fname,'.MS'), fname, spaste(fname,'_gsm'),
                        ant=ant);
    if (is_fail(mc)) {
        print "meqcalibratertest(): could not instantiate meqcalibrater";
        fail;
    }
    
    # Plot initial position
    parms := mc.getparms("GSM.*.RA GSM.*.DEC");
    nrpos := len(parms) / 2;
    if (nrpos > 0) {
        for (i in [1:nrpos]) {
            ra  := parms[spaste('GSM.',i,'.RA')].value[1];
            dec := parms[spaste('GSM.',i,'.DEC')].value[1];
            print 'src = ', i, ' ra = ', ra, ' dec = ', dec;

            if (is_fail(annotator)) fail;
            annotator.add_marker(i, real(ra), real(dec), i==nrpos);
        }
    }

    mc.settimeinterval(3600); # calibrate per 3600 seconds (1 timeslot = 2 sec)
    for (i in [1:niter]) {
        print "iteration", i
            mc.clearsolvableparms();
        print 'Solving for RA ...';
        mc.setsolvableparms("GSM.*.RA");
        
        mc.resetiterator();
        while (mc.nextinterval())
        {
            mc.solve();
        }
        
        mc.clearsolvableparms();
        print 'Solving for DEC ...';
        mc.setsolvableparms("GSM.*.DEC");
        
        mc.resetiterator();
        while (mc.nextinterval())
        {
            mc.solve();
            
            parms := mc.getparms("GSM.*.RA GSM.*.DEC");
            nrpos := len(parms) / 2;
            if (nrpos > 0) {
                for (j in [1:nrpos]) {
                    ra  := parms[spaste('GSM.',j,'.RA')].value[1];
                    dec := parms[spaste('GSM.',j,'.DEC')].value[1];
                    print 'src =', j, ' ra =', ra, ' dec =', dec;
                    
                    if (is_fail(annotator)) fail;
                    annotator.add_marker(j, real(ra), real(dec), j==nrpos);
                }
            }
        }
    }
    
    mc.done();
    
    return ref annotator;
}








solvegain := function(fname='demo', ant=4*[0:20], niter=1)
{
    mc := meqcalibrater(spaste(fname,'.MS'), fname, spaste(fname,'_gsm'),
                        ant=ant);
    if (is_fail(mc)) {
        print "meqcalibratertest(): could not instantiate meqcalibrater"
            fail
            }

#    mc.select('', 0, 0);

    mc.settimeinterval(3600); # calibrate per 1 hour
    mc.clearsolvableparms();

    mc.setsolvableparms("gain.*");

#			"gain.22.ST_{0,1,2,3,4,5,6,7,8,9,10}");

#    parms := mc.getparmnames("*");
#    print 'ALL PARMS = ', parms;

    mc.resetiterator()
        while (mc.nextinterval())
        {
            d := mc.getsolvedomain();
            print 'solvedomain = ', d;

            for (i in [1:niter])
            {
                print "iteration", i
                    mc.solve();

                parms := mc.getparms("gain.11.*");
                print 'GAIN SOLUTION = ';
                for (k in [1:5])
                {
                    print parms[k].value[1];
                }

                parms := mc.getparms("gain.22.*");
                print 'GAIN SOLUTION = ';
                for (k in [1:5])
                {
                    print parms[k].value[1];
                }
            }
            print mc.getstatistics()
            }

    parms := mc.getparms("gain.11.*");
    print 'SOLUTION 11: ', parms;
    parms := mc.getparms("gain.22.*");
    print 'SOLUTION 22: ', parms;

    mc.done();
}






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
    
    pt.putinit ('EJ11.ampl', values=1);
    pt.putinit ('EJ11.phase', values=0, diff=1e-6, diffrelative=F);
    pt.putinit ('EJ12.ampl', values=0);
    pt.putinit ('EJ12.phase', values=0, diff=1e-6, diffrelative=F);
    pt.putinit ('EJ21.ampl', values=0);
    pt.putinit ('EJ21.phase', values=0, diff=1e-6, diffrelative=F);
    pt.putinit ('EJ22.ampl', values=0);
    pt.putinit ('EJ22.phase', values=0, diff=1e-6, diffrelative=F);

#    pt.putinit ('EJ11.real', values=1);
#    pt.putinit ('EJ12.real', values=0);
#    pt.putinit ('EJ21.real', values=0);
#    pt.putinit ('EJ22.real', values=1);
#    pt.putinit ('EJ11.imag', values=1);
#    pt.putinit ('EJ12.imag', values=0);
#    pt.putinit ('EJ21.imag', values=0);
#    pt.putinit ('EJ22.imag', values=1);
    pt.done();
}





# l, m, alpha0, delta0 in radians...
lm_to_alpha_delta := function(l, m, alpha0, delta0)
{
    alphadelta := [0.0, 0.0];
    alphadelta[1] := alpha0 + atan(l/(cos(delta0)*sqrt(1.0 - (l*l + m*m)) - m*sin(delta0) ));
    alphadelta[2] := asin(m*cos(delta0)+sin(delta0)*sqrt(1.0-(l*l + m*m)));
    
    if(alphadelta[1] < 0.0) {
        alphadelta[1] +:= 2*pi;
    }
    if(alphadelta[1] >= 2*pi) {
        alphadelta[1] -:= 2*pi;
    }
    return alphadelta;
}




alpha_delta_to_lm := function(alpha, delta, alpha0, delta0)
{
    lm := [0.0, 0.0];
    da := alpha-alpha0;
    lm[1] := cos(delta)*sin(da);
    lm[2] := sin(delta)*cos(delta0) - sin(delta0)*cos(delta)*cos(da);
    return lm;
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






initchan := function(fname='demo')
{
    t := table(spaste(fname, '.MS/SPECTRAL_WINDOW'),readonly=F)
        if (is_fail(t)) fail;
    t.putcell ('CHAN_WIDTH', 1, array(1e7,1));
    t.close();
}






setchan := function(fname='demo')
{
    t := table(spaste(fname, '.MS/SPECTRAL_WINDOW'),readonly=F)
        if (is_fail(t)) fail;
    t.putcell ('CHAN_WIDTH', 1, array(1e6,1));
    t.close();
}






init := function(fname='demo')
{
    initparms(fname=fname);
    initgsm(fname=fname);
    predict(fname=fname);
}
