include 'pgplot.g'
include 'meptable.g'
include 'debug_methods.g'
include 'text_frame.g'
include 'widgetserver.g'

const eval_poly := function (poly,xval,yval)
{
  f := xval*0;
  for( i in 1:poly::shape[1] )
    for( j in 1:poly::shape[2] )
      f +:= poly[i,j]*xval^(i-1)*yval^(j-1);
  return f;
}

const make_range := function(rng,np)
{
  return rng[1] + (0:(np-1))/(np-1)*(rng[2]-rng[1]);
}

const make_x_slice := function(poly,xrng=[-1,1],y0=0,npix=200)
{
  xval := (0:(npix-1))/as_float(npix-1)*2 - 1;
  yval := array(y0,npix);
  return eval_poly(poly,xval,yval);
}

const make_y_slice := function(poly,x0=0,yrng=[-1,1],npix=200)
{
  xval := array(x0,npix);
  yval := (0:(npix-1))/as_float(npix-1)*2 - 1;
  return eval_poly(poly,xval,yval);
}

const make_poly_image := function(poly,npix=200)
{
  nx := npix;
  ny := npix;
  # all polynomials are normalized to [-1,1]
  xval0 := (0:(nx-1))/as_float(nx-1)*2 - 1;
  yval0 := (0:(ny-1))/as_float(ny-1)*2 - 1;
  xval := array(0.0,nx,ny);
  yval := xval;
  for( i in 1:ny )
    xval[,i] := xval0;
  for( i in 1:nx )
    yval[i,] := yval0;
  return eval_poly(poly,xval,yval);
}

const plot_poly_image := function(ref pgp,ref img,
      xrng=[-1,1],yrng=[-1,1],
      labels=F,bci=1,lci=1)
{
  nx := len(img[,1]);
  ny := len(img[1,]);
  # plot image
  pgp->sci(bci);
  pgp->env(xrng[1],xrng[2],yrng[1],yrng[2],0,-1);
  pgp->box('bcni',0,0,'bcni',0,0);
  dx := xrng[2]-xrng[1];
  dy := yrng[2]-yrng[1];
  pgp->gray(img,max(img),min(img),
    [xrng[1]-dx/(2*nx),dx/nx,0,yrng[1]-dy/(2*ny),0,dy/ny]);
#  print labels;
  pgp->sci(lci);
  if( labels )
    pgp->lab(labels[1],labels[2],labels[3]);
}

const plot_slice := function (ref pgp,ref slice,env=F,labels=F,ci=1)
{
  if( !is_boolean(env) )
  {
    x1 := as_float(env[1]);
    x2 := as_float(env[2]);
    y1 := as_float(env[3]);
    y2 := as_float(env[4]);
  #  print 'min/max',x1,x2,y1,y2;
    if( x1 == x2 )
    { x1 := 0.75*x1; x2 := 1.25*x2; }
    if( y1 == y2 )
    { y1 := 0.75*y1; y2 := 1.25*y2; }
  #  print 'x: ',x1,x2,slice.x;
  #  print 'y: ',y1,y2,slice.y;
    pgp->sci(1);
    pgp->env(x1,x2,y1,y2,0,-1);
    pgp->box('bcni',0,0,'bcni',0,0);
    if( labels )
      pgp->lab(labels[1],labels[2],labels[3]);
  }
  pgp->sci(ci);
  pgp->line(slice.x,slice.y);
}

const plot_all_slices := function (ref pgp,ref slices,colors,labels=F)
{
  env := slices._range;
  for( f in field_names(slices) )
  {
    if( f == '_range' )
      next;
#    print f,len(slices[f].x),len(slices[f].y),env,labels,colors[f];
    plot_slice(pgp,slices[f],env=env,labels=labels,ci=colors[f]);
    env := F;
  }
  w := pgp->qwin();
  cs := pgp->qcs(4);
  x0 := w[1];
  y0 := w[4] + cs[2]*.3;
  for( f in field_names(slices) )
  {
    pgp->sci(colors[f]);
    pgp->ptxt(x0,y0,0,0,f);
    bb := pgp->qtxt(x0,y0,0,0,f);
    x0 := bb[4] + cs[1]*1;
  }
}

const poly_mep_viewer := function (rec,
      xlab='TIME',ylab='FREQ',
      ids=F,plotcolors=F,
      parent=F,title='PolyMEP Viewer',
      textwidth=38,pgpsize=350,unmap=F,
      ref widgetset=dws,
      verbose=2)
{
  self := [ appid = 'poly_mep_viewer' ];
  public := [=];
  define_debug_methods(self,public,verbose);

  self.ws := ref widgetset;  
  self.agent := create_agent();
  
  #--------------------------------------------------------------------------
  const public.plot_param := function(which,field,newslice=F)
  {
    wider self;
    self.agent->plot_param(which,field,newslice);
  }
  whenever self.agent->plot_param do
  {
    if( len($value) != 3 )
      self.dprint(0,'plot_param event error: incorrect # of arguments');
    self.plot_param($value[1],$value[2],$value[3]);
  }
  const self.plot_param := function(which,field,newslice)
  {
    if( is_string(which) && !has_field(self.rec,which) )
      fail paste('unknown parameter',which)
    else
      which := field_names(self.rec)[which];
    parm := ref self.rec[which];
    self.dprint(3,paste('check that we have the right field',which,field));
    if( !has_field(parm,'poly') )
      fail 'missing poly field in parameter';
    if( !has_field(parm.poly,field) )
      fail paste('missing field',field,'in parameter',which);
    self.dprint(3,'setup ranges, if none');
    if( !has_field(parm,'xrng') )
      parm.xrng := [-1,1];
    if( !has_field(parm,'yrng') )
      parm.yrng := [-1,1];
    self.dprint(3,'ranges are ',parm.xrng,parm.yrng);
    # print values in status window
    txt := spaste('Parameter: ',which,'\n');
    maxlen := max(strlen(field_names(parm.poly)))+2;
    format := spaste('%-',maxlen,'s');
    pad := spaste('\n',spaste(array(' ',maxlen)));
    for( f in field_names(parm.poly) )
    {
      txt := spaste(txt,sprintf(format,spaste(f,': ')));
      str := "";
      for( i in 1:len(parm.poly[f][,1]) )
        str[i] := paste(sprintf("%10f",parm.poly[f][i,]));
      txt := spaste(txt,paste(str,sep=pad),'\n');
    }
    self.status.settext(txt);
    
    
    # store new slice and mark cross-sections for recomputation
    if( !is_boolean(newslice) ) 
    {
      parm.xcross := max(min(newslice[1],parm.xrng[2]),parm.xrng[1]);
      parm.ycross := max(min(newslice[2],parm.yrng[2]),parm.yrng[1]);
      parm.xslice := [=];
      parm.yslice := [=];
    }
    # if nothing marked, specify new cross-section marks
    if( !has_field(parm,'xcross') || !has_field(parm,'ycross') )
    {
      parm.xcross := sum(parm.xrng)/2;
      parm.ycross := sum(parm.yrng)/2;
      parm.xslice := [=];
      parm.yslice := [=];
    }
    self.dprint(3,'slices are ',parm.xcross,' ',parm.ycross);
    self.dprint(3,'compute image, if not already there');
    if( !has_field(parm,'image') )
      parm.image := [=];
    if( !has_field(parm.image,field) )
      parm.image[field] := make_poly_image(parm.poly[field],npix=pgpsize);
    self.dprint(3,'plot image');
    self.pgpmain->sci(1);
    plot_poly_image(self.pgpmain,parm.image[field],parm.xrng,parm.yrng,
        labels=[self.xlab,self.ylab,spaste(which,' - ',field,': polynomial surface') ],
        bci=self.plotcolors[field],lci=1);
    self.dprint(3,'plot slice marks');
    self.pgpmain->sci(self.plotcolors[field]);
    self.pgpmain->line(parm.xrng,[parm.ycross,parm.ycross]);
    self.pgpmain->line([parm.xcross,parm.xcross],parm.yrng);
    self.dprint(3,'compute slices, if not already done');
    if( !has_field(parm,'xslice') || !len(parm.xslice) ||
        !has_field(parm,'yslice') || !len(parm.yslice)  )
    {
      parm.xslice := [=];
      parm.yslice := [=];
      parm.xslice._range := [];
      parm.yslice._range := [];
      for( f in field_names(parm.poly) )
      {
        x0 := (parm.xcross-parm.xrng[1])/(parm.xrng[2]-parm.xrng[1])*2-1;
        y0 := (parm.ycross-parm.yrng[1])/(parm.yrng[2]-parm.yrng[1])*2-1;
        x := make_range(parm.xrng,pgpsize);
        y := make_x_slice(parm.poly[f],y0=y0,npix=pgpsize);
        parm.xslice[f] := [x=x,y=y];
        parm.xslice._range := range(parm.xslice._range,y);
        x := make_y_slice(parm.poly[f],x0=x0,npix=pgpsize);
        y := make_range(parm.yrng,pgpsize);
        parm.yslice[f] := [ x=x,y=y ];
        parm.yslice._range := range(parm.yslice._range,x);
      }
      parm.xslice._range := [parm.xrng,parm.xslice._range];
      parm.yslice._range := [parm.yslice._range,parm.yrng];
      self.dprint(3,'slice ranges: ',parm.xslice._range,' ',parm.yslice._range);
    }
    self.dprint(3,'plotting x slices');
    labels := [self.xlab,which,sprintf('Cross-section for %s=%f',self.ylab,parm.ycross)];
    self.dprint(3,labels);
    plot_all_slices(self.pgptop,parm.xslice,self.plotcolors,labels=labels);
    self.dprint(3,'plotting y slices');
    labels := [which,self.ylab,sprintf('Cross-section for %s=%f',self.xlab,parm.xcross)];
    self.dprint(3,labels);
    plot_all_slices(self.pgpleft,parm.yslice,self.plotcolors,labels=labels);
  }
  #--------------------------------------------------------------------------
  const public.set_data := function (ref rec,xlab='TIME',ylab='FREQ',ids=F,plotcolors=F)
  {
    wider self;
    self.agent->set_data([rec=ref rec,xlab=xlab,ylab=ylab,ids=ids,plotcolors=plotcolors]);
  }
  whenever self.agent->set_data do
  {
    self.xlab := $value.xlab;
    self.ylab := $value.ylab;
    # record of polynomial parameters
    self.rec := ref $value.rec;
    # available ids (i.e., 'original', 'perturbed', etc.). If not specified,
    # take from first record
    ids := $value.ids;
    if( !is_string(ids) )
      ids := field_names(self.rec[1].poly);
    self.data_ids := ids;
    self.selected_id := ids[1];
    # setup plot colors; allocate if not specified
    if( is_boolean(plotcolors) || len(plotcolors) != len(ids) )
      plotcolors := 1+(1:len(self.data_ids));
    self.plotcolors := [=];
    for( i in 1:len(ids) )
      self.plotcolors[self.data_ids[i]] := plotcolors[i];

    # work out a good width for the selector widgets
    width := max(self.textwidth,strlen(field_names(self.rec)));
    if( len(self.data_ids) > 1 )
      width := max(width,strlen(self.data_ids));
#    width := min(width,textwidth);

    # build parameter selector
    self.paramsel := [=];
    self.paramsel.fr := self.ws.frame(self.cmdframe,side='left',expand='x',padx=0,pady=0);
  #  self.param.label := self.ws.label(self.paramframe,'Parameter: ',fill='none');
    self.paramsel.sel := self.ws.listbox(self.paramsel.fr,
                          width=width,height=min(6,len(self.rec)),fill='none');
    self.paramsel.sb := self.ws.scrollbar(self.paramsel.fr);
    whenever self.paramsel.sb->scroll do self.paramsel.sel->view($value);
    whenever self.paramsel.sel->yscroll do self.paramsel.sb->view($value);
    self.paramsel.pad := self.ws.frame(self.paramsel.fr,expand='x',width=10,height=5);
    self.paramsel.sel->insert(field_names(self.rec));
    
    # build data id selector
    self.idselect := [=];
    self.idselect.fr0 := self.ws.frame(self.cmdframe,side='left',padx=0,pady=0,expand='x');
    self.idselect.sel := self.ws.listbox(self.idselect.fr0,
                          width=width,height=min(6,len(self.data_ids)),fill='none');
    self.idselect.sb := self.ws.scrollbar(self.idselect.fr0);
    whenever self.idselect.sb->scroll do self.idselect.sel->view($value);
    whenever self.idselect.sel->yscroll do self.idselect.sb->view($value);
    self.idselect.pad := self.ws.frame(self.idselect.fr0,expand='x',width=10,height=5);
    self.idselect.sel->insert(self.data_ids);
    self.idselect.sel->select(0);
    
    # set status
    self.status.settext(
        sprintf("Loaded polynomials for %d parameters, datasets: %s",
                len(self.rec),paste(self.data_ids,sep=', ')));
    
#     self.idselect.fr := self.ws.frame(self.idselect.fr0,side='top',padx=0,pady=0,expand='none',
#                                   relief='groove');
#     self.idselect.fr1 := self.ws.frame(self.idselect.fr0,side='left',padx=0,pady=0,height=5,width=5,expand='x');
#     self.idselect.fr->disable();
#     self.idselect.btn := [=];
#     for( f in self.data_ids )
#       self.add_data_id_button(f);
#     self.idselect.btn[self.data_ids[1]]->state(T);

    # handle id-selector events      
    whenever self.idselect.sel->select do 
    {
      self.selected_id := self.data_ids[$value+1];
      self.refresh();
    }
    # handle param-selector events
    whenever self.paramsel.sel->select do 
    {
      self.selected_param := $value+1;
      self.selected_id := self.data_ids[1];
      self.refresh();
    }
      
#    self.idselect := self.ws.listbox(self.idselect_fr,
#          width=width,height=min(3,len(self.data_ids)),fill='none');
#    self.idselect_pad := self.ws.frame(self.idselect_fr,expand='x',width=10,height=5);
#    self.idselect->insert(self.data_ids);
#    self.idselect->select(0);
    # handle id selection events
#    whenever self.idselect->select do
#    {
#      self.selected_id := self.data_ids[$value+1];
#      if( has_field(self,'selected_param') && len(self.data_ids)>1 ) 
#        public.plot_param(self.selected_param,self.selected_id);
#    }
  }
  #--------------------------------------------------------------------------
  const public.clear := function ()
  {
    wider self;
    self.agent->clear();
  }
  whenever self.agent->clear do
  {
    self.rec := F;
    self.paramsel := F;
    self.idselect := F;
    self.pgpmain->eras();
    self.pgptop->eras();
    self.pgpleft->eras();
  }
  #--------------------------------------------------------------------------
  const self.refresh := function ()
  {
    wider self;
    if( is_record(self.rec) && has_field(self,'selected_param') )
    {
      if( !has_field(self,'selected_id') )
        self.selected_id := self.data_ids[1];
      public.plot_param(self.selected_param,self.selected_id);
    }
  }
#   const self.add_data_id_button := function (id) 
#   {
#     wider self;
#     self.idselect.btn[id] := self.ws.button(self.idselect.fr,id,type='radio',
#                                        relief='flat',justify='left',value=id);
#     whenever self.idselect.btn[id]->press do
#     {
#       self.selected_id := $value;
#       if( has_field(self,'selected_param') && len(self.data_ids)>1 ) 
#         public.plot_param(self.selected_param,self.selected_id);
#     }
#   }
  const public.add_data_id := function (id,plotcolor=F,refresh=F)
  {
    wider self;
    self.agent->add_data_id([id=id,plotcolor=F,refresh=refresh]);
  }
  whenever self.agent->add_data_id do
  {
    self.data_ids := [ self.data_ids,$value.id ];
    self.idselect.sel->insert($value.id);
    if( is_boolean($value.plotcolor) )
      self.plotcolors[$value.id] := self.plotcolors[len(self.plotcolors)]+1; 
    else
      self.plotcolors[$value.id] := $value.plotcolor;
#    if( is_record(self.idselect.btn) )
#      self.add_data_id_button($value.id);
    # slices will need to be recomputed
    for( f in field_names(self.rec) )
    {
      self.rec[f].xslice := [=];
      self.rec[f].yslice := [=];
    }
    if( $value.refresh )
      self.refresh();
  }
  const public.topframe := function ()
  {
    wider self;
    return ref self.parent;
  }

  self.parent := self.ws.frame(parent,side='top',title=title);
  self.parent->unmap();
  self.frames := [=];
  self.frames.top := self.ws.frame(self.parent,side='left',padx=0,pady=0,borderwidth=0);
  self.frames.btm := self.ws.frame(self.parent,side='left',padx=0,pady=0,borderwidth=0);
  self.frames.f1  := self.ws.frame(self.frames.top,side='top',padx=0,pady=0,borderwidth=0);
  self.frames.f2  := self.ws.frame(self.frames.top,side='left',padx=0,pady=0,borderwidth=0);
  self.frames.f3  := self.ws.frame(self.frames.btm,side='left',padx=0,pady=0,borderwidth=0);
  self.frames.f4  := self.ws.frame(self.frames.btm,side='left',padx=0,pady=0,borderwidth=0);
  self.textwidth := textwidth;
  self.status   := text_frame(self.frames.f1,size=[textwidth-4,10],wrap='word',
                              disabled=T,ws=self.ws);
  self.cmdframe := self.ws.frame(self.frames.f1,side='top',width=pgpsize);
  self.cmdpad   := self.ws.frame(self.frames.f1,side='top',padx=0,pady=0,width=pgpsize,height=0);
  self.pgptop   := pgplot(self.frames.f2,width=pgpsize,height=pgpsize,padx=0,pady=0);
  self.pgpleft  := pgplot(self.frames.f3,width=pgpsize,height=pgpsize,padx=0,pady=0);
  self.pgpmain  := pgplot(self.frames.f4,width=pgpsize,height=pgpsize,padx=0,pady=0);
  self.pgpmain->bind('<Button>','click');
  self.rec := F;
  
  if( is_record(rec) )
    public.set_data(rec,xlab,ylab,ids,plotcolors);
   
  if( !unmap )
    self.parent->map();
  
  # handle button-press events in the polt window
  whenever self.pgpmain->click do
  {
    if( is_record(self.rec) && has_field(self,'selected_param') )
      public.plot_param(self.selected_param,self.selected_id,newslice=$value.world);
  }
  # handle top-level resize events
  whenever self.parent->resize do
  {
    if( is_record(self.rec) && has_field(self,'selected_param') )
      public.plot_param(self.selected_param,self.selected_id);
  }

  public.self := ref self;
  return ref public;
}

##   pgp := pgplot(f,width=200,height=200);
##   if( is_fail(pgp) )
##     print pgp;
##   poly := array(0.,2,2);
##   poly[1,1] := 1;
##   poly[1,2] := .5;
##   poly[2,1] := .5;
##   poly[2,2] := -.5;
## 
##   img:=polyplot(pgp,poly);
## rec := [=];
## rec.poly0 := [ poly = [ original=array([1,.5,.5,-.5,.2,-.1],2,3),
##                         solution=array([1,.4,.4,-.4,.1,-.2],2,3) ] ];
## rec.poly1 := [ poly = [ original=array([0,1,-1,.5,-.2,.1],2,3),
##                         solution=array([1,.4,.4,-.4,.1,-.2],2,3) ] ];
## rec.poly2 := [ poly = [ original=array([0,-1,1,-.5,.3,.1],2,3),
##                         solution=array([1,.4,.4,-.4,.1,-.2],2,3) ] ];
##                         
## demomep := read_mep_table('demo_gsm.MEP',demomep);

const mep2rec := function (mep,col='VALUES',field='poly',range=T,rec=[=],
                           subset=F)
{
  if( is_boolean(subset) )
    subset := field_names(mep);
    
  for( f in field_names(mep) )
    if( has_field(mep[f],col) && len(mep[f][col]) > 1 )
    {
      if( !has_field(rec,f) )
        rec[f] := [=];
      if( !has_field(rec[f],'poly') )
        rec[f].poly := [=];
      rec[f].poly[field] := mep[f][col];
      if( range )
      {
        if( has_field(mep[f],'STARTTIME') && has_field(mep[f],'ENDTIME') )
        {
          rec[f].xrng := [mep[f].STARTTIME,mep[f].ENDTIME];
          if( rec[f].xrng[1] == rec[f].xrng[2] )
            rec[f].xrng := [-1,1];
        }
        if( has_field(mep[f],'STARTFREQ') && has_field(mep[f],'ENDFREQ') )
        {
          rec[f].yrng := [mep[f].STARTFREQ,mep[f].ENDFREQ];
          if( rec[f].yrng[1] == rec[f].yrng[2] )
            rec[f].yrng := [-1,1];
        }
#        print 'Ranges for ',f,': ',rec[f].xrng,rec[f].yrng;
      }
    }
  return rec;
}

const viewmep := function (fname='demo')
{
  tabname := spaste(fname,'.MEP');
  mep := read_mep_table(tabname);
  rec := mep2rec(mep,field='SIM_VALUES',col='SIM_VALUES',range=T);
  rec := mep2rec(mep,field='VALUES',col='VALUES',range=F,rec=rec);
  # adjust ranges for better readability
  for( f in field_names(rec) )
  {
    rec[f].xrng := [0,1];
    rec[f].yrng *:= 1e-8;
  #  rec[f].poly.solution := rec[f].poly.original*.9;
  }

  viewer := poly_mep_viewer(rec,
             ids="SIM_VALUES VALUES",
             xlab='NORMALIZED_TIME',ylab='FREQ (MHz)',
             title=paste('PolyMEP Viewer:',tabname),
             verbose=1);
  return ref viewer;
}


# viewmep('demo');
