include 'randomnumbers.g';
include 'table.g';
include 'quanta.g';


# Apply additive gaussian noise
# sigma is a quantity, e.g '5mJy'
#
applynoise := function(ms='', sigma='0Jy', column='MODEL_DATA')
{
    t := table(ms, readonly=F);
    if(is_fail(t)) {
        print('applynoise(): failed to open MS');
        return F;
    }
    

    chunksize    := 100.0e+6; # Bytes
    elem_per_row := prod(shape(t.getcell(column,1)));
    nrows        := floor(chunksize/elem_per_row/8/8);

    print(paste('elem_per_row: ', elem_per_row));
    print(paste('nrow: ', nrows));
    
    totalrows := t.nrows();    
    print(paste('t.nrows(): ',totalrows));
    start     := 1;
 
    rn := randomnumbers();
        
    while( start <= totalrows) {
        print(paste('applynoise(): start = ', start));
        dcol := t.getcol(column, start, nrows);
        if(is_fail(dcol)) {
            print('applynoise(): failed to open column');
            t.close();
            return F;
        }

        print 'applynoise(): shape(dcol) = ', shape(dcol);
        
        noisere := rn.normal(0, dq.convert(dq.quantity(sigma), 'Jy').value, shape(dcol));
        noiseim := rn.normal(0, dq.convert(dq.quantity(sigma), 'Jy').value, shape(dcol));
        
        noise := noisere + noiseim*(0+1i);
        dcol  := dcol + noise;

        t.putcol(column, dcol, start, nrows);
        start := start + nrows;
    }
    t.close();
}


advisesigma := function(ms='', sigmamap='0Jy', column='MODEL_DATA', numant=0, numchan=1, numtimes=1, numpol=1)
{
    nvis  := 0.0;
    nflag := 0.0;
    if(numant == 0) {
        t := table(ms);
        if(is_fail(t)) {
            print('advisesigma(): failed to open MS');
            return F;
        }
        dcol := t.getcol(column);
        nvis := prod(shape(dcol));
        nflag := sum(t.getcol('FLAG'));
        t.close();
    } else {
        nvis := numant*(numant-1)/2*numchan*numtimes*numpol;
    }
    return dq.convert(dq.quantity(sigmamap), 'Jy').value*sqrt((nvis-nflag)/4);
}
