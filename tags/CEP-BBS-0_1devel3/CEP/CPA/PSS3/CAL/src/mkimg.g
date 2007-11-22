include 'imager.g'
include 'image.g'
include 'table.g'

tohms := function (rad)
{
    hh := rad*180/pi/15;
    mm := 60 * (hh - as_integer(hh));
    ss := 60 * (mm - as_integer(mm));
    print spaste(rad,' rad = ',as_integer(hh),'h',as_integer(mm),'m',ss);
}



todms := function (rad)
{
    dd := rad*180/pi;
    mm := 60 * (dd - as_integer(dd));
    ss := 60 * (mm - as_integer(mm));
    print spaste(rad,' rad = ',as_integer(dd),':',as_integer(mm),':',ss);
}

fromhms := function(hh,mm,ss)
{
    return (hh + mm/60. + ss/3600.)*pi*15/180;
}
fromdms := function(dd,mm,ss)
{
    return (dd + mm/60. + ss/3600.)*pi/180;
}


mkimg := function (msname, imgname, type='model', npix=500, nchan=0,
                   start=1, step=1, msselect='', mode='mfs',
                   cellx='0.1arcsec', celly='0.1arcsec')
{
  if (nchan == 0) {
      t:=table(msname, readonly=F);
      a:=t.getcell('DATA',1);
      nchan := shape(a)[2];
      t.close();
  }
  print nchan;

  imgr := imager(msname);

  if (!is_fail(imgr))
  {
    imgr.setdata(mode='channel', nchan=nchan, start=start, step=1, fieldid=1,
                 msselect=msselect);
    imgr.setimage(nx=npix, ny=npix, cellx=cellx, celly=celly,
                  mode=mode,  facets=1);
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
