include 'meqcalibrater.g'

include 'imager.g'
include 'table.g'
include 'image.g'

mkimg := function (msname, imgname, type='model', npix=2000)
{
  t:=table(msname, readonly=F);
  a:=t.getcell('DATA',1);
  nchannels := shape(a)[2];
  print nchannels;
  t.close();

  imgr := imager(msname);
  imgr.setdata(mode='channel', nchan=nchannels, step=1, fieldid=1);
  imgr.setimage(nx=npix, ny=npix, cellx='1arcsec', celly='1arcsec', mode='mfs',  facets=1);
  imgr.weight('uniform');
  imgr.makeimage(type=type, image=imgname);

#  include "image.g";

  img := image(imgname);
  img.view();
}

#
# Demo function showing the predict functionality and creating an image of it.
#
predict := function(msname)
{
    local mc := meqcalibrater(msname,
			      '/aips++/wierenga/mep',
			      '/aips++/wierenga/gsm');

    if (is_fail(mc)) {
	print "meqcalibratertest(): could not instantiate meqcalibrater"
	fail
    }

    mc.settimeinterval(3600); # calibrate per 1 hour
    mc.clearsolvableparms();
    mc.setsolvableparms("a.b.* f.g.*[34]", T);
#    mc.setsolvableparms("Leakage.*", T);
    
    mc.resetiterator()
    i := 0
    while (mc.nextinterval())
    {
	d := mc.getsolvedomain();
	print 'solvedomain = ', d;

	mc.predict('MODEL_DATA');

	# mc.saveresidualdata('DATA', 'MODEL_DATA', 'RESIDUAL_DATA');
	# mc.saveparms();
	
	parms := mc.getparms("f.g.h.i.j.[1-5]");
	print 'getparms = ', parms

	i+:=1;
    }

    mc.done();

    return T;
}

predictdemo := function()
{
    predict('/aips++/wierenga/michiel.MS');

    mkimg('/aips++/wierenga/michiel.MS', '/aips++/wierenga/GER.img');
}
    
