include 'table.g'


t := table('3C343.MS')
#t := table('10008336.MS')
t1 := t.query (sortlist='TIME,ANTENNA1,ANTENNA2')
t2 := table('10008336.MS_p1')
if (t1.nrows() != t2.nrows) fail '#rows mismatch'

if (!all (t1.getcol('ANTENNA1') == t2.getcol('ANTENNA1'))) fail 'ANTENNA1 mismatch'
if (!all (t1.getcol('ANTENNA2') == t2.getcol('ANTENNA2'))) fail 'ANTENNA2 mismatch'
if (!all (t1.getcol('TIME') == t2.getcol('TIME'))) fail 'TIME mismatch'

nr := as_integer(t.nrows()/10000)
if (nr > 0) {
    for (i in [1:nr]) {
	print 'checking',10000,'rows in part',i
	if (!all (t1.getcol('DATA',1+(i-1)*10000,10000) ==
		  t2.getcol('DATA',1+(i-1)*10000,10000))) {
	    print paste('mismatch in part',i)
	}
    }
}
nrl := t.nrows() - nr*10000
if (nrl > 0) {
    print 'checking',nrl,'rows in part',nr+1
    if (!all (t1.getcol('DATA',1+nr*10000,nrl) ==
	      t2.getcol('DATA',1+nr*10000,nrl))) {
	print paste('mismatch in part',nr+1)
    }
}

exit


t := table('10008336.MS')
t1 := t.query (sortlist='TIME,ANTENNA1,ANTENNA2')
t2 := table('10008336.MS_p1', readonly=F)
if (t1.nrows() != t2.nrows) fail '#rows mismatch'

nr := as_integer(t.nrows()/10000)
if (nr > 0) {
    for (i in [1:nr]) {
	print 'copying',10000,'rows in part',i
	t2.putcol ('DATA', t1.getcol('DATA',1+(i-1)*10000,10000),
		   1+(i-1)*10000,10000);
    }
}
nrl := t.nrows() - nr*10000
if (nrl > 0) {
    print 'copying',nrl,'rows in part',nr+1
    t2.putcol ('DATA', t1.getcol('DATA',1+nr*10000,10000),
               1+nr*10000,nrl);
}

exit
