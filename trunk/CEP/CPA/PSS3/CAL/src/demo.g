include 'table.g'
include 'meqcalibrater.g'
include 'parmtable.g'
include 'gsm.g'
include 'imgannotator.g'
include 'mkimg.g'

#
# Demo function showing the predict functionality and creating an image of it.
#
predict := function(fname='michiel.demo')
{
    local mc := meqcalibrater(spaste(fname,'.MS'), 
			      fname, fname);
    if (is_fail(mc)) {
	print "meqcalibratertest(): could not instantiate meqcalibrater"
	fail
    }

    mc.settimeinterval(3600); # calibrate per 1 hour
    mc.clearsolvableparms();
    
    mc.resetiterator()
    while (mc.nextinterval())
    {
	d := mc.getsolvedomain();
	print 'solvedomain = ', d;

	mc.predict('MODEL_DATA');
    }

    mc.done();

    mkimg(spaste(fname, '.MS'), spaste(fname, '.img'));

    return T;
}

solve := function(fname='michiel.demo', niter=1)
{
    local mc := meqcalibrater(spaste(fname,'.MS'), 
			      fname, fname);
    if (is_fail(mc)) {
	print "meqcalibratertest(): could not instantiate meqcalibrater"
	fail
    }

    mc.settimeinterval(3600); # calibrate per 1 hour
    mc.clearsolvableparms();
    #mc.setsolvableparms("GSM.*.I");
    #mc.setsolvableparms("GSM.*.RA GSM.*.DEC");
    mc.setsolvableparms("GSM.*.DEC GSM.*.RA");
    #mc.setsolvableparms("Leakage.{11,22}.*");
    
    mc.resetiterator()
    while (mc.nextinterval())
    {
	d := mc.getsolvedomain();
	print 'solvedomain = ', d;

	for (i in [1:niter]) {
	  mc.solve('MODEL_DATA');
        }
	print mc.getparms ("GSM.*");
    }

    mc.done();

    return T;
}

solvepos := function(fname='michiel.demo', niter=1)
{
    annotator := imgannotator(spaste(fname, '.img'), 'raster');
	
    local mc := meqcalibrater(spaste(fname,'.MS'), 
			      fname, fname);

    mc.select('all([ANTENNA1,ANTENNA2] in 4*[0:20])');

    if (is_fail(mc)) {
	print "meqcalibratertest(): could not instantiate meqcalibrater"
	fail
    }

    # Plot initial position
    parms := mc.getparms("GSM.0.RA GSM.0.DEC");
    ra    := parms['GSM.0.RA'].value[1];
    dec   := parms['GSM.0.DEC'].value[1];
    print 'ra = ', ra, ' dec = ', dec;
    annotator.add_marker(real(ra), real(dec));

    mc.settimeinterval(3600); # calibrate per 1 hour
    for (i in [1:niter]) {
        mc.clearsolvableparms();
	print 'Solving for RA ...'
        mc.setsolvableparms("GSM.*.RA");
    
        mc.resetiterator()
        while (mc.nextinterval())
        {
	    mc.solve('MODEL_DATA');

	    parms := mc.getparms("GSM.0.RA GSM.0.DEC");
	    ra    := parms['GSM.0.RA'].value[1];
	    dec   := parms['GSM.0.DEC'].value[1];
	    print 'ra = ', ra, ' dec = ', dec;

	    annotator.add_marker(real(ra), real(dec));
        }

        mc.clearsolvableparms();
	print 'Solving for DEC ...'
        mc.setsolvableparms("GSM.*.DEC");
    
        mc.resetiterator()
        while (mc.nextinterval())
        {
	    mc.solve('MODEL_DATA');

	    parms := mc.getparms("GSM.0.RA GSM.0.DEC");
	    ra    := parms['GSM.0.RA'].value[1];
	    dec   := parms['GSM.0.DEC'].value[1];
	    print 'ra = ', ra, ' dec = ', dec;

	    annotator.add_marker(real(ra), real(dec));
        }
    }

    mc.done();

    return ref annotator;
}

initleakage := function(fname='michiel.demo')
{
  pt := parmtable(spaste(fname,'.MEP'), T);
  if (is_fail(pt)) fail;
  pt.putinit ('Leakage', 0);
  pt.putinit ('Leakage.11', 1);
  pt.done();
}

setleakage := function(fname='michiel.demo')
{
  pt := parmtable(spaste(fname,'.MEP'), T);
  if (is_fail(pt)) fail;
  pt.putinit ('Leakage', 0);
  pt.putinit ('Leakage.11', 1.1);
  pt.done()
}

initgsm := function()
{
  tg := gsm('michiel.demo.GSM', T);
  if (is_fail(tg)) fail;
  tg.addpointsource ('src0', [0,1e20], [0,1e20],
		     2.734, 0.45379, 1, 0, 0, 0);
#  tg.addpointsource ('src1', [0,1e20], [0,1e20],
#		     2.733, 0.45369, 1, 0, 0, 0);
#  tg.addpointsource ('src2', [0,1e20], [0,1e20],
#		     2.7335, 0.45375, 1, 0, 0, 0);
  tg.done()
}

setgsm := function()
{
  tg := gsm('michiel.demo.GSM', T);
  if (is_fail(tg)) fail;
  tg.addpointsource ('src0', [0,1e20], [0,1e20],
		     2.734030, 0.453785, 1, 0, 0, 0);
#  tg.addpointsource ('src1', [0,1e20], [0,1e20],
#		     2.7330025, 0.4536875, 1, 0, 0, 0);
#  tg.addpointsource ('src2', [0,1e20], [0,1e20],
#		     2.7335075, 0.4537525, 1, 0, 0, 0);
  tg.done();
#  t.putcell ('RAPARMS', 1, array(as_double(-2.744002744),1,1))
#  t.putcell ('DECPARMS', 1, array(as_double(0.5325005325),1,1))
#  t.putcell ('IPARMS', 1, array(as_double(1.2),1,1))
}

initchan := function()
{
  t := table('michiel.demo.MS/SPECTRAL_WINDOW',readonly=F)
  if (is_fail(t)) fail;
  t.putcell ('CHAN_WIDTH', 1, array(1e7,1));
  t.close()
}
setchan := function()
{
  t := table('michiel.demo.MS/SPECTRAL_WINDOW',readonly=F)
  if (is_fail(t)) fail;
  t.putcell ('CHAN_WIDTH', 1, array(1e6,1));
  t.close()
}

init := function()
{
	initleakage();
	initgsm();
	predict();
}

demo := function()
{
	global annotator;
	setgsm();
	annotator:=solvepos(niter=10);
}

