include 'octopussy.g'

solver := function (suspend=F,verbose=1)
{
  self := [=];
  public := [=];
  # start up octopussy
  self.octo := octopussy(server='./glishsolver',suspend=suspend,
                         options="-d0");
  self.octo.start();
  self.agentref := ref self.octo.agentref();
  self.octo.subscribe("Solver.Out.*");
  self.state := -1;
  self.paused := F;
  self.statestr := "unknown";
  self.verbose := verbose;
  self.rqid := 1;
  
  
  const public.dprint := function (level,...)
  {
    wider self;
    if( level <= self.verbose )
     print spaste(...);
  }
  const self.dprint := function (level,...)
  {
    wider public;
    public.dprint(level,...);
  }
  const public.setverbose := function (level)
  {
    wider self;
    self.verbose := level;
    return level;
  }
  
  # register standard event handlers
  whenever self.agentref->solver_out_app_notify_state do
  {
    self.dprint(1,"== state: ",$value.state,"(",$value.state_string,")");
    self.state := $value.state;
    self.paused := $value.paused;
    self.statestr := $value.state_string;
  }
  
  const self.new_requestid := function ()
  {
    ret := self.rqid;
    self.rqid +:= 1;
    return ret;
  }
  const public.octo := function ()
  {
    wider self;
    return ref self.octo;
  }
  const public.agentref := function ()
  {
    wider self;
    return ref self.agentref;
  }
  
  const public.command := function (message,payload=F)
  {
    wider self;
    return self.octo.publish(spaste("Solver.In.App.Control.",message),
                priority=10,
                rec=payload);
  }
  
  const public.init := function ( input=[=],output=[=] )
  {
    wider public;
    initrec := [=];
    initrec.input := input;
    initrec.output := output;
    initrec.control := [=];
    initrec.control.event_map_in  := [ default_prefix = hiid("Solver.In") ];
    initrec.control.event_map_out := [ default_prefix = hiid("Solver.Out") ];
    initrec.control.stop_when_end := F; 
    public.command("Init",initrec);
  }
  
  const public.stop := function ()
  {
    wider public;
    public.command("Stop");
  }
  const public.halt := function ()
  {
    wider public;
    public.command("Halt");
  }
  const public.pause := function ()
  {
    wider public;
    public.command("Pause");
  }
  const public.resume := function (rec=F)
  {
    wider public;
    public.command("Resume",rec);
  }
  
  const public.solve := function (solverec)
  {
    wider public;
    public.command("Solver.Add.Solution",solverec);
  }
  
  const public.endsolve := function (endrec=[=])
  {
    wider public;
    public.command("Solver.End.Solution",endrec);
  }
  
  const public.nextdomain := function ()
  {
    wider public;
    public.command("Next.Domain");
  }
  
  const public.state := function ()
  {
    wider self;
    return self.state;
  }
  const public.is_paused := function ()
  {
    wider self;
    return self.paused;
  }
  const public.strstate := function ()
  {
    wider self;
    return self.statestr;
  }
  
  const public.reqstatus := function ()
  {
    wider public;
    public.command("Request.Status");
  }
  
  const public.status := function (subfield=F)
  {
    wider public;
    wider self;
    rqid := self.new_requestid();
    rec := [ request_id = hiid(rqid) ];
    evname := spaste("solver_out_app_notify_status_",rqid);
    if( subfield )
      rec.field := hiid(subfield);
    public.command("Request.Status",rec);
    await self.agentref->[evname];
    return $value.value;
  }
  
  return ref public;
};

verbose := 3;
print "Creating solver";
solv := solver(suspend=F,verbose=verbose);

print "Creating event handlers"
whenever solv.agentref()->* do 
{
  solv.dprint(2,'       ==========> Event: ', $name);
  solv.dprint(3,'       value: ', $value);
}

test := function ()
{
  print "Starting solver";
  inputrec := [ ms_name = 'test.ms',data_column_name = 'DATA', tile_size = 10 ];
  inputrec.selection :=  [ ddid_index = 1, field_index = 1, 
      channel_start_index = 1, channel_end_index = 10,
      selection_string = 'ANTENNA1 == 1 && ANTENNA2 == 2' ];
  outputrec := [ write_flags = F ];
  solv.init(input=inputrec,output=outputrec);
}


