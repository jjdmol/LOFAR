include 'table.g'

doit := function (msname, ant1, ant2, ra, dec, stchan=1, endchan=-1, maxrow=-1)
{
    t := tablecommand (paste('select from', msname, 'where ANTENNA1==',
			      ant1 ,'&& ANTENNA2==', ant2,
			     '&& FIELD_ID==0 && DATA_DESC_ID==0'));
    if (t.nrows() == 0) fail;
    t1 := table(t.getkeyword('FIELD'));
    phasecenter := t1.getcell('PHASE_DIR', 1);
    ra0 := phasecenter[1,1];
    dec0 := phasecenter[2,1];
    print "phase-center: ", ra0, dec0;
    t1.close();
    t1 := table(t.getkeyword('SPECTRAL_WINDOW'));
    chanfreq := t1.getcell('CHAN_FREQ',1);
    t1.close();
    if (stchan <= 0) stchan := 1;
    if (stchan > len(chanfreq)) stchan := len(chanfreq);
    if (endchan <= 0  ||  endchan > len(chanfreq)) endchan := len(chanfreq);
    if (stchan > endchan) fail "stchan>endchan";
    print "chan-range: ",endchan,stchan,chanfreq[stchan:endchan];
    nrchan := endchan-stchan+1;
    data := array(0+0i, nrchan);
    pred := array(0+0i, nrchan);
    sl := cos(dec)*sin(ra-ra0);
    sm := sin(dec)*cos(dec0) - cos(dec)*sin(dec0)*cos(ra-ra0);
    sn := sqrt(1 - sl*sl - sm*sm);
    print 'lmn=',sl,sm,sn-1,ra,dec,ra0,dec0;

    c := 2.99792458e+08;
    # dft = exp(2*pi*(ul+vm+wn))
    t0 := t.getcell('TIME',1);
    if (maxrow <= 0  ||  maxrow > t.nrows()) {
	maxrow := t.nrows();
    }
    for (row in [1:maxrow]) {
	uvw := t.getcell('UVW', row);
	dat := t.getcell('DATA', row);
	print 'uvw=',uvw;
	for (chan in [1:nrchan]) {
	    wvl := c/chanfreq[chan+stchan-1];
	    x := -2*pi*(uvw[1]*sl + uvw[2]*sm + uvw[3]*(sn-1))/wvl;
	    pred[nrchan-chan+1] := (cos(x) + 1i*sin(x)) / sn;
	    data[nrchan-chan+1] := dat[1,chan+stchan-1];
	}
	pred *:= 0.5;
	data::print.precision := 10;
	print t.getcell('TIME',row)-t0, t.getcell('ANTENNA1',row),
	    t.getcell('ANTENNA2',row), uvw, "pred:", pred;
	print "data:",data;
    }
    t.close();
}

doit('10008336.MS',4,8,4.3396003966265599,1.0953677174056471,25,40,10000);
exit
