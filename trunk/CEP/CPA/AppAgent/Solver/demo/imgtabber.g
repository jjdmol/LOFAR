include 'widgetserver.g'
include 'debug_methods.g'
include 'viewer.g'
include 'image.g'

dws := ref dv.widgetset();

const imgtabber := function (parent=F,title='Image tabber',
                   viewer=dv,verbose=1)
{
  self := [ appid='imgtabber' ];
  public := [=];
  define_debug_methods(self,public,verbose);

  self.agent := create_agent();  
  self.dv := ref viewer;
  self.ws := ref self.dv.widgetset();
  self.topfr := self.ws.frame(parent,side='top',title=title);
  self.btnfr_base := self.ws.frame(self.topfr,side='top',expand='x',padx=0,pady=0);
  self.btnfr := [=];
  self.btnfr[1] := self.ws.frame(self.btnfr_base,side='left',expand='x');
  self.vdp   := ref self.dv.newdisplaypanel(self.topfr,
        width=500,height=500,
        hasgui=T,hasdismiss=F,hasdone=F);
  self.current_id := F;
  self.current_frame := 0;
  
#  self.btmfr := self.ws.frame(self.topfr,side='right',expand='x');
  
  # set the parent
  if( is_agent(parent) )
    self.parent := ref parent;
  else
    self.parent := ref self.topfr;
    
  self.imgs := [=];
  
  whenever self.agent->showimage do
  {
    if( is_string(self.current_id) && self.current_id == $value )
      self.dprint(3,'showimage: already showing ',$value);
    else
    {
      self.dprint(3,'showimage: ',$value,', holding');
      self.dv.hold();
      if( is_string(self.current_id) && self.vdp.isregistered(self.imgs[self.current_id].vdd) )
      {
        self.dprint(4,'showimage: unregistering ',self.current_id);
        self.vdp.unregister(self.imgs[self.current_id].vdd);
      }
      self.dprint(4,'showimage: registering ',$value);
      if( !self.vdp.isregistered(self.imgs[$value].vdd))
        self.vdp.register(self.imgs[$value].vdd);
      else
        self.dprint(4,'showimage warning: ',$value,' is already registered');
      self.current_id := $value;
      self.dprint(4,'showimage: release');
      self.dv.release();
      self.vdp.unzoom();
      self.imgs[$value].button->state(T);
    }
  }

  whenever self.agent->addimage do
  {
    id := $value.id;
    img := ref $value.img;
    newrow := $value.newrow;
    if( has_field(self.imgs,id) )
      self.dprint(0,'addimage: image ',id,' already exists');
    else
    {
      self.dprint(1,'addimage: ',id);
      # load image data
      vdd := self.dv.loaddata(img,'raster');
      if( is_fail(vdd) )
        self.dprint(0,'addimage: loaddata failed: ',vdd);
      else
      {
        vdd.setoptions([axislabelswitch=T,titletext=id]);
        self.imgs[id] := [ vdd = ref vdd ];
        # add another button frame?
        if( newrow || !self.current_frame )
        {
          self.current_frame +:= 1;
          if( !has_field(self.btnfr,self.current_frame) || 
              !is_agent(self.btnfr[self.current_frame]) )
          {
            self.btnfr[self.current_frame] := 
                self.ws.frame(self.btnfr_base,side='left',expand='x');
          }
        }
        # add button
        self.imgs[id].button := self.ws.button(self.btnfr[self.current_frame],
                                               id,type='radio',
                                               group=self.btnfr_base,
                                               value=id,relief='flat');
     #   print self.imgs[id];
        self.agent->showimage(id);
        whenever self.imgs[id].button->press do
          self.agent->showimage($value);
      }
      # link self.imgs[id].button->press to self.agent->showimage;
    }
  }
  
  whenever self.agent->deleteimage do
  {
    id := $value.id;
    self.dprint(1,'deleteimage: ',id);
    if( !has_field(self.imgs,id) )
      self.dprint(0,'deleteimage: no such image ',$value);
    else
    {
      self.imgs[id].button := F;
      self.imgs[id].vdd.done();
      self.imgs[id] := F;
    }
  }
  whenever self.agent->deleteall do
  {
    self.dprint(1,'deleteall');
    self.vdp.unregisterall();
    for( f in field_names(self.imgs) )
      self.imgs[id].vdd.done();
    self.imgs := [=];
    self.current_frame := 1;
  }
  
  const public.addimage := function (id,ref img,unmap=T,newrow=F)
  {
    wider self;
    self.agent->addimage([id=id,img=ref img,newrow=newrow]);
  }
  const public.showimage := function (id)
  {
    wider self;
    self.agent->showimage(id);
  }
  const public.deleteimage := function (id,unmap=T)
  {
    wider self;
    self.agent->deleteimage([id=id]);
  }
  const public.deleteall := function (unmap=T)
  {
    wider self;
    self.agent->deleteall();
  }

  public.self := ref self;
  return ref public;
}

## f := dv.widgetset().frame();
## dv.hold();
## tabber := imgtabber(f,verbose=4);
## tabber.addimage('observed',image('demo.img.obs0'),unmap=F);
## tabber.addimage('predict',image('demo.img.pred0'),unmap=F);
## tabber.addimage('residuals',image('demo.img.res0'),unmap=F);
## dv.release();
