include 'imager.g'
include 'image.g'
include 'table.g'

mkimg := function (msname, imgname, type='model', npix=500, nchan=0,
start=1, step=1, msselect)
{
  t:=table(msname, readonly=F);
  a:=t.getcell('DATA',1);
  if(nchan == 0) {
      nchan := shape(a)[2];
  }
  print nchan;
  t.close();

  imgr := imager(msname);

  if (!is_fail(imgr))
  {
    imgr.setdata(mode='channel', nchan=nchan, start=start, step=1, fieldid=1,
                 msselect=msselect);
    imgr.setimage(nx=npix, ny=npix, cellx='0.1arcsec', celly='0.1arcsec',
                  mode='mfs',  facets=1);
    imgr.weight('uniform');
    imgr.makeimage(type=type, image=imgname);

    img := image(imgname);
    
    if (!is_fail(img))
    {
      img.view();
    }

    return ref img;
  }
  else return F;
}
