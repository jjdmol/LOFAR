include 'octopussy.g'
pragma include once

default_octopussy := F;

# Starts a global octopussy proxy with the given arguments, 
# unless already started.
# Returns a ref to the proxy object.
const start_octopussy := function(server,options="",suspend=F)
{
  global default_octopussy;
  if( is_boolean(default_octopussy) )
  {
    default_octopussy := octopussy(server=server,options=options,suspend=suspend);
    if( is_fail(default_octopussy) )
    {
      print 'fatal error: octopussy startup failed';
      fail 'octopussy startup failed';
    }
    default_octopussy.start();
  }
  return ref default_octopussy;
}

# defines a set of standard debug methods inside an object
const define_debug_methods := function (ref self,ref public,initverbose=1)
{
  self.verbose := initverbose;
  # prints debug message if level is <= current verbosity level
  const public.dprint := function (level,...)
  {
    wider self;
    if( level <= self.verbose )
     print spaste('[== ',self.appid,' ==] ',...);
  }
  # private version for convenience
  const self.dprint := function (level,...)
  {
    wider public;
    public.dprint(level,...);
  }
  # sets the verbosity level for the dprint methods
  const public.setverbose := function (level)
  {
    wider self;
    self.verbose := level;
    return level;
  }
  return T;
}

const define_app_proxy := function (ref self,ref public,
        appid,server,options,suspend,verbose)
{
  self.octo := start_octopussy(server,options,suspend);
  if( is_fail(self.octo) )
    fail;
  self.appid := appid;
  self.lappid := paste(split(to_lower(appid),'.'),sep='_');
  self.agentref := ref self.octo.agentref();
  self.relay := create_agent();
  self.octo.subscribe(spaste(appid,".Out.*"));
  self.state := -1;
  self.paused := F;
  self.statestr := "unknown";
  self.verbose_events := T;
  self.rqid := 1;
  self.waiting_init := F;
  
  # define standard debug methods
  define_debug_methods(self,public,verbose);
  
  # define standard app proxy methods
  # allocates a new request-id
  const self.new_requestid := function ()
  {
    ret := self.rqid;
    self.rqid +:= 1;
    return ret;
  }
  # Returns the application name
  const public.appid := function ()
  {
    wider self;
    return self.appid;
  }
  # Enables or disables verbose event reports. With no arguments,
  # returns current setting
  const public.verbose_events := function (verb=-1)
  {
    wider self;
    if( is_boolean(verb) )
      self.verbose_events := verb;
    return self.verbose_events;
  }
  # Creates an event name by prefixing it with "appname_"
  const public.eventname := function (...)
  {
    wider self;
    return paste(self.lappid,...,sep="_");
  }
  # Returns a ref to the octopussy proxy
  const public.octo := function ()
  {
    wider self;
    return ref self.octo;
  }
  # Returns a ref to octopussy's agent
  const public.agentref := function ()
  {
    wider self;
    return ref self.agentref;
  }
  # Returns a ref to the local relay agent
  const public.relay := function ()
  {
    wider self;
    return ref self.relay;
  }
  # Sends an app control command to the app
  const public.command := function (message,payload=F)
  {
    wider self;
    return self.octo.publish(spaste(self.appid,".In.App.Control.",message),
                priority=10,
                rec=payload);
  }
  # Reinitializes the app with a (possibly) partial init record
  const public.reinit := function ( initrec,wait=F )
  {
    wider public;
    wider self;
    self.waiting_init := T;
    public.command("Init",initrec);
    if( wait )
    {
      while( self.waiting_init )
      {
        self.dprint(2,'  (awaiting app_notify_init event)');
        await self.relay->app_notify_init;
      }
      self.dprint(2,'  (event received, initialization complete)');
    }
  }
  # Initializes the app.
  # If wait=T, waits for the app to complete init
  const public.init := function ( initrec=[=],input=[=],output=[=],wait=F )
  {
    wider public;
    wider self;
    initrec.input := input;
    initrec.output := output;
    initrec.control := [=];
    initrec.control.event_map_in  := [ default_prefix = hiid(self.appid,"In") ];
    initrec.control.event_map_out := [ default_prefix = hiid(self.appid,"Out") ];
    initrec.control.stop_when_end := F; 
    public.reinit(initrec,wait);
  }
  # Shortcuts for sending various commands to the app
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
  # Returns current state, pause state and state string
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
  # Requests a full status record from the app
  const public.reqstatus := function ()
  {
    wider public;
    public.command("Request.Status");
  }
  # Requests a specific field of the status record from the app
  const public.status := function (subfield=F)
  {
    wider public;
    wider self;
    rqid := self.new_requestid();
    rec := [ request_id = hiid(rqid) ];
    evname := public.eventname('out_app_status',rqid);
    if( subfield )
      rec.field := hiid(subfield);
    public.command("Request.Status",rec);
    await self.agentref->[evname];
    return $value.value;
  }
  
  # register global event handler for this app -- all "our" events are
  # relayed to the relay agent
  whenever self.agentref->* do
  {
    self.dprint(5,"original event: ",$name);
    # check that event is intended for us
    ev := split($name,'_');
    if( len(ev) > 2 && ev[1] == self.lappid && ev[2] == 'out' )
    {
      shortname := paste(ev[3:len(ev)],sep='_');
      # process state events
      if( shortname == 'app_notify_state' )
      {
        self.dprint(1,$value.state_string,' (',$value.state,')');
        self.state := $value.state;
        self.paused := $value.paused;
        self.statestr := $value.state_string;
      }
      else if( shortname == 'app_notify_init' )
      {
        self.waiting_init := F;
      }
      # if event has a text component, print it
      if( has_field($value,'text') )
        public.dprint(1,$value.text); 
      # print events if so requested
      if( self.verbose_events )
      {
        public.dprint(2,'   event: ', shortname);
        public.dprint(3,'   value: ', $value);
      }
      # forward event to local relay agent
      self.relay->[shortname]($value);
    }
  }
  
  return T;
}

const app_proxy := function(appid,server=F,options=F,suspend=F,verbose=1)
{
  self := [=];
  public := [=];
  ret := define_app_proxy(self,public,appid,server,options,suspend,verbose);
  if( is_fail(ret) )
    fail;
  return ref public;
}
