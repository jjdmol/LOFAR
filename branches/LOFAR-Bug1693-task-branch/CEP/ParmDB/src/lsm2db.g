include 'table.g'

lsm2db := function(fin, out, maxnrsrc)
{
    t := tablefromascii(out, fin);
    if (is_fail(t)) fail;
    nrsrc := min(maxnrsrc, t.nrows());
    if (nrsrc == 0) {
        fail 'lsm list is empty';
    }
    t1 := t.query(sortlist='Flux desc');
    of := open("> lsm.fil-scr");
    write (of, 'beginParmdbInputFile () {');
    write (of, '  rm -f fill.parmdb');
    write (of, '  add2file "create $*"');
    write (of, '}');
    write (of, 'add2file () {');
    write (of, '  echo $* >> fill.parmdb');
    write (of, '}');
    write (of, 'applyfile () {');
    write (of, '  add2file  "quit" ');
    write (of, '  parmdb < fill.parmdb');
    write (of, '}');
    write (of, 'makeLsm () {');
    write (of, '  diff="1e-6" ');
    write (of, '  if [ "$#" -eq "1" ]; then');
    write (of, '    diff=$1 ');
    write (of, '  fi');
    rah := t1.getcol('RAh');
    ram := t1.getcol('RAm');
    ras := t1.getcol('RAs');
    decd := t1.getcol('Decd');
    decm := t1.getcol('Decm');
    decs := t1.getcol('Decs');
    flux := t1.getcol('Flux');
    t.close();
    t1.close();
    for (i in [1:nrsrc]) {
	# Get ra and dec in radians.
	ra := (abs(rah[i]) + ram[i]/60. + ras[i]/3600.)*15*pi/180;
	if (rah[i] < 0) ra := -ra;
	ra::print.precision:=15
	dec := (abs(decd[i]) + decm[i]/60. + decs[i]/3600.)*pi/180;
	if (decd[i] < 0) dec := -dec;
	dec::print.precision:=15;
	fl := flux[i];
	fl::print.precision:=5;
	write (of, spaste('  add2file  "adddef RA.CP',i,'  values=',ra,', diff=${diff}, diff_rel=F"'));
	write (of, spaste('  add2file  "adddef DEC.CP',i,' values=',dec,', diff=${diff}, diff_rel=F"'));
	write (of, spaste('  add2file  "adddef StokesI.CP',i,' values=',fl,',     diff=${diff} "'));
	write (of, spaste('  add2file  "adddef StokesQ.CP',i,' values=0,       diff=${diff} "'));
	write (of, spaste('  add2file  "adddef StokesU.CP',i,' values=0,       diff=${diff} "'));
	write (of, spaste('  add2file  "adddef StokesV.CP',i,' values=0,       diff=${diff} "'));
    }
    write (of, '}');
    print 'wrote', out;
    return T;
}
