pragma include once
include 'app_proxy.g'

solver := function (appid='Solver',
    server='./applauncher',options="-d0 -meq:M:M:Solver",
    suspend=F,verbose=1,
    gui=F,parent_frame=F,ref widgetset=dws)
{
  self := [=];
  public := [=];
  
  # init app proxy
  ret := define_app_proxy(self,public,appid,server,options,
                          suspend,verbose,gui,parent_frame,widgetset=widgetset);
  if( is_fail(ret) )
    fail;
  # define solver-specific methods
  const public.solve := function (solverec,set_default=F)
  {
    wider public;
    public.command("Solver.Add.Solution",solverec,set_default=set_default);
  }
  const public.endsolve := function (endrec=[=],set_default=F)
  {
    wider public;
    public.command("Solver.End.Solution",endrec,set_default=set_default);
  }
  const public.nextdomain := function (from_gui=F)
  {
    wider public;
    public.command("Solver.Next.Domain");
  }
  # redefine the resume method
  const public.resume := function (rec=F,from_gui=F,set_default=F)
  {
    wider public;
    wider self;
    if( !from_gui )
      public.command("Resume",rec,set_default=set_default);
  }
  # add solver-specific gui elements
  if( has_field(self,'gui') )
  {
    self.gui.cmd2_pad := self.ws.frame(self.gui.cmd2_frame,width=30,height=5);
    self.gui.nextdomain := self.ws.button(self.gui.cmd2_frame,'Next domain');
    self.gui.endsolve := self.ws.button(self.gui.cmd2_frame,'End solve');
    self.gui.solve := self.ws.button(self.gui.cmd2_frame,'Solve');
    
    self.make_command_dialog('Add solution','Solver.Add.Solution',
                              self.gui.solve,size=[40,15]);
    self.make_command_dialog('End solution','Solver.End.Solution',
                              self.gui.endsolve,size=[40,5]);
    self.make_command_dialog('Resume solution','Resume',
                              self.gui.resume,size=[40,5]);
    
    whenever self.gui.nextdomain->press do public.nextdomain(from_gui=T);
    
  }
  # intercept solve commands so as to change default 'resume' arguments
  whenever self.relay->['sending_command_Solver.Add.Solution'] do
  {
    res := public.get_default_args('Resume');
    for( f in "iter_step max_iter convergence when_max_iter when_converged" )
      if( has_field($value,f) )
        res[f] := $value[f];
    public.set_default_args('Resume',res);
  }
  
  # set default arguments
  public.set_default_args('Solver.End.Solution',[save_parms=F,save_residuals=F,next_domain=F]);
  public.set_default_args('Resume',[iter_step=0]);
  
  return ref public;
}

