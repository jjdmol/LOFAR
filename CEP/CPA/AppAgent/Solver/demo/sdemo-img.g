include 'widgetserver.g'
include 'glishsolver.g'
include 'imager.g'
include 'imgtabber.g'
include 'viewer.g'
include 'debug_methods.g'

print shell('killall -9 applauncher');

filename := 'demo';

rescol := 'CORRECTED_DATA';
predcol := 'MODEL_DATA';
outputrec := 
  [ write_flags=F,
    residuals_column=rescol,predict_column=predcol ];

# create solv and rpt objects
img_repeater := function (verbose=1,suspend=F)
{
  self := [ appid='imgrepeater' ];
  public := [=];
  define_debug_methods(self,public,verbose);
  
  print "Creating imaging repeater";
  start_octopussy('./applauncher',"-d0 -rpt:O:M:Repeater.A",
                  suspend=suspend);
                 
  self.ws := ref dv.widgetset();
  self.parent := self.ws.frame(title='Imaging repeater',side='left');
  self.parent->unmap();
  self.rpt := ref app_proxy('Repeater.A',
          verbose=verbose,parent_frame=self.parent,gui=T,widgetset=self.ws);
#  self.solv := ref app_proxy('Solver',verbose=verbose,widgetset=self.ws);
  
  if( is_fail(self.rpt) )
    fail;
  self.tabber := imgtabber(self.parent,verbose=verbose);
  self.parent->map();

  const public.rpt := function ()
  {
    wider self;
    return ref self.rpt;
  }    
  const public.enable := function (auto_pause=F)
  {
    wider self;
    rpt_inputrec := [ event_map_in = [default_prefix = hiid('vis.out') ] ];
    return self.rpt.init([auto_pause=auto_pause],rpt_inputrec,outputrec,wait=F);
  }
  const public.disable := function ()
  {
    wider self;
    rpt_inputrec := [ event_map_in = [default_prefix = hiid('a.b') ] ];
    return self.rpt.init([=],rpt_inputrec,outputrec,wait=F);
  }
  
  const public.imageloop := function (delete_on_reset=T)
  {
    wider self;
    wider public;
    public.enable(auto_pause=T);
    loop := T;
    reset := T;
    solvecount := 0;
    itercount := 0;
    imgcount := 0;
    self.rpt.relay()->notify_imaging([text='Entering imaging loop']);
    imgr := F;
    msname := "";
    while( loop )
    {
      await self.rpt.relay()->*;
#      print "Repeater event: ",$name;
      if( $name !~ m/data_set_footer_.*/ )
        next;
      newrow := F;
      id_pred := "";
      # figure out which images to generate based on footer
      if( $name ~ m/domain_intermediate_residuals/ )
      {
        itercount +:= 1;
        if( reset )
        {
          solvecount +:= 1;
          if( delete_on_reset )
          {
            self.tabber->deleteall();
            imgcount := 0;
          }
          else 
            newrow := T;
          reset := F;
          itercount := 1;
          id_pred := spaste(solvecount,':predict_',itercount);
        }
        id_res := spaste(solvecount,':res_',itercount);
      }
      else if( $name ~ m/domain_final_residuals/ )
      {
        reset := T;
        id_res := spaste(solvecount,':res_final');
        id_pred := spaste(solvecount,':predict_final');
      }
      else 
      {
        self.dprintf(0,'unknown footer ',$name,' skipping');
        next;
      }
      footer := $value;
#      # start imager if not already running
#      if( is_boolean(imgr) || msname != footer.header.ms_name )
#      {
        msname := footer.header.ms_name;
        self.rpt.notify('Starting imager on MS ',msname);
        imgr := imager(msname);
        if( is_fail(imgr) )
        {
          self.rpt.notify('imager startup failed',imgr);
          imgr := F;
          next;
        }
#      }
      # set imaging options
      mssel  := $value.header.selection.selection_string;
      chan0  := $value.header.channel_start_index;
      nchan  := $value.header.channel_end_index - chan0 +1;
      mssel =~ s/ANTENNA1/(ANTENNA1-1)/g;
      mssel =~ s/ANTENNA2/(ANTENNA2-1)/g;
      npix := 1000;
      mode := 'mfs';
      cellx := '0.1arcsec';
      celly := '0.1arcsec';
      self.dprint(1,'MS selection is ',mssel);
      self.dprint(1,'Channels are ',chan0,' ',nchan);
      res := imgr.setdata(mode='channel',nchan=nchan,start=chan0,step=1,
                   fieldid=1,msselect=mssel);
      if( is_fail(res) )
        { self.rpt.notify('imager.setdata failed: ',res); next; }
      res := imgr.setimage(nx=npix, ny=npix, cellx=cellx, celly=celly,
                    mode=mode,  facets=1);
      if( is_fail(res) )
        { self.rpt.notify('imager.setimage failed: ',res); next; }
      res := imgr.weight('uniform');
      if( is_fail(res) )
        { self.rpt.notify('imager.weight failed: ',res); next; }
      # generate the images
      if( len(id_pred) )
      {
        imgcount +:= 1;
        imgname := spaste(msname ~ s/.MS/.img./,'pred.',imgcount);
        self.rpt.notify('Making predicted image ',imgname);
        res := imgr.makeimage(type='model',image=imgname);
        print res;
        if( is_fail(res) )
          self.rpt.notify('imager.makeimage failed: ',res);
        else
        {
          res := self.tabber.addimage(id_pred,imgname,newrow=newrow);
          if( is_fail(res) )
            self.dprint(0,'tabber.addimage failed: ',res);
          newrow := F;
        }
      }
      if( len(id_res) )
      {
        imgname := spaste(msname ~ s/.MS/.img./,'res.',imgcount);
        self.rpt.notify('Making residual image ',imgname);
        res := imgr.makeimage(type='corrected',image=imgname);
        print res;
        if( is_fail(res) )
          self.rpt.notify('imager.makeimage failed: ',res);
        else
        {
          res := self.tabber.addimage(id_res,imgname,newrow=newrow);
          if( is_fail(res) )
            self.dprint(0,'tabber.addimage failed: ',res);
        }
      }
      imgr.done();
      imgr := F;
      imgcount +:= 1;
      self.rpt.resume();
    }
  }
  
  public.enable();
  public.self := ref self;
  
  return ref public;
}

make_image := function (number=0,data=F,predict=F,residual=F,redo=F)
{
  if( data )
  { type:='observed'; suf:='obs'; }
  else if( predict )
  { type := 'model'; suf:='pred'; }
  else if( residual )
  { type := 'corrected'; suf:='res'; }
  else
    fail 'must specify one of: data, predict, residual'
  
  imgname := spaste(filename,'.img.',suf,number);
  
  if( !redo )
  {
    img := image(imgname);
    if( !is_fail(img) )
    {
      print 'Image file already exists and redo=F, reusing';
      img.view();
      return ref img;
    }
  }
  sel := mssel;
  sel =~ s/ANTENNA1/(ANTENNA1-1)/g;
  sel =~ s/ANTENNA2/(ANTENNA2-1)/g;
  img := mkimg(msname,imgname,msselect=sel,type=type,
               cellx='0.25arcsec',
               celly='0.25arcsec',
               npix=1000)
  return ref img;
}

irep := img_repeater(verbose=1,suspend=F);
irep.self.tabber.setverbose(5);
irep.imageloop(delete_on_reset=F);
