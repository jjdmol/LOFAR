pragma include once
include 'octopussy.g'
include 'recutil.g'

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

const define_app_proxy := function (ref self,ref public,
        appid,server,options,suspend,verbose,gui,parent_frame)
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
  self.last_cmd_arg := [=];

  # setup fail/exit handlers
  whenever self.agentref->["fail done exit"] do 
  {
    public.dprint(1,"exit event: ",$name,$value);
    if( $name == 'fail' )
      msg := paste('client process has died unexpectedly: ',$name,$value);
    else
      msg := paste('client process has exited: ',$name,$value);
    self.relay->app_notify_state([state=-2,state_string=to_upper($name),text=msg]);
    self.octo := F;
    self.agentref := F;
  }
    
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
  const public.setdebug := function(context,level)
  {
    wider self;
    return self.octo.setdebug(context,level);
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
  const public.get_default_args := function (command)
  {
    if( has_field(self.last_cmd_arg,command) )
      return self.last_cmd_arg[command];
    else
      return [=];
  }
  const public.set_default_args := function (command,data,update_gui=T)
  {
    wider self;
    self.last_cmd_arg[command] := data;
    if( update_gui )
      self.update_command_dialog(command,data);
  }
  # Sends an app control command to the app
  const public.command := function (message,payload=F,update_gui=T,set_default=F)
  {
    wider self;
    wider public;
    public.set_default_args(message,payload);
    if( !set_default )
    {
      # report as event to relay
      self.relay->[spaste('sending_command_',message)](payload);
      return self.octo.publish(spaste(self.appid,".In.App.Control.",message),
                  priority=10,
                  rec=payload);
    }
  }
  # Reinitializes the app with a (possibly) partial init record
  const public.reinit := function (initrec,wait=F)
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
  const public.stop := function (from_gui=F)
  {
    wider public;
    public.command("Stop");
  }
  const public.halt := function (from_gui=F)
  {
    wider public;
    public.command("Halt");
  }
  const public.pause := function (from_gui=F)
  {
    wider public;
    public.command("Pause");
  }
  public.resume := function (rec=F,from_gui=F)
  {
    wider public;
    wider self;
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
  
  const self.make_text_frame := function (parent,disabled=T,
                        size=[60,20],wrap='none',text='')
  {
    rec := [=];
    rec.tf := dws.frame(parent,side='left',borderwidth=0);
    rec.text := dws.text(rec.tf,disabled=disabled,text=text,
          relief='sunken',width=size[1],height=size[2],wrap=wrap);
    rec.vsb := dws.scrollbar(rec.tf);
    rec.bf := dws.frame(parent,side='right',borderwidth=0,expand='x');
    rec.pad := dws.frame(rec.bf,
                expand='none',width=23,height=23,relief='groove');
    rec.hsb := dws.scrollbar(rec.bf,orient='horizontal');
    whenever rec.vsb->scroll, rec.hsb->scroll do
        rec.text->view($value);
    whenever rec.text->yscroll do
        rec.vsb->view($value);
    whenever rec.text->xscroll do
        rec.hsb->view($value);
    const rec.settext := function (text)
    {
      wider rec;
      rec.text->delete('1.0','99999.99999');
      rec.text->append(text);
      rec.text->see('0.0')
    }
    return rec;
  }
  
  const self.update_command_dialog := function (command,payload)
  {
    wider self;
    if( has_field(self,'gui') && has_field(self.gui,'dialog') &&
        has_field(self.gui.dialog,command) )
    {
      text := rec2string(payload);
      self.gui.dialog[command].browser.settext(text);
    }
  }
  const self.make_command_dialog := function (name,command,button=F,size=[40,20])
  {
    wider self;
    rec := [=];
    rec.topfr := dws.frame(parent=F,title=spaste(self.appid,': ',name),
                            tlead=button,relief='groove');
    rec.topfr->unmap();
    
    rec.label := dws.label(rec.topfr,paste(self.appid,': ',name));
    if( has_field(self.last_cmd_arg,command) )
      text := rec2string(self.last_cmd_arg[command]); 
    else
      text := '';
    rec.browser := self.make_text_frame(rec.topfr,disabled=F,
            size=size,text=text);
    rec.browser.text->see('0,0');
    rec.cmdframe := dws.frame(parent=rec.topfr,side='left',expand='none');
    rec.accept := dws.button(rec.cmdframe,to_upper(name));
    rec.reset  := dws.button(rec.cmdframe,'Reset');
    rec.cancel := dws.button(rec.cmdframe,'Cancel');
    
    rec.error := dws.frame(parent=F,tlead=rec.accept,relief='groove');
    rec.error->unmap();
    rec.error_msg := dws.label(rec.error,'Failed to parse input record');
    rec.error_dismiss := dws.button(rec.error,'Dismiss');
    
    whenever rec.error_dismiss->press do rec.error->unmap();
    whenever rec.cancel->press do 
    {
      rec.topfr->unmap();
      rec.error->unmap();
    }
    whenever rec.reset->press do
    {
      if( has_field(self.last_cmd_arg,command) )
        text := rec2string(self.last_cmd_arg[command]); 
      else
        text := '';
      rec.browser.settext(text);
    }
    whenever rec.accept->press do 
    {
      str := rec.browser.text->get('0.0','end');
#      print 'browser contents: ',str;
      data := string2rec(str);
      # failed conversion?
      if( is_fail(data) )
      {
        rec.error->map();
      }
      else
      {
#      print 'executing ',command,'with',data;
        public.command(command,data,update_gui=F);
        rec.topfr->unmap();
        rec.error->unmap();
      }
    }
    # setup event handler for parent button
    if( !is_boolean(button) )
    {
      whenever button->press do rec.topfr->map();
    }
    self.gui.dialog[command] := rec;
  }
  
  const public.make_gui_frame := function (parent=F)
  {
    wider self;
    wider public;
    if( !is_record(dws) ) fail 'dws not loaded -- include widgetserver.g first';
    if( is_record(self.gui) ) fail 'gui frame already constructed';
    self.gui := [=];
    # create frame
    self.gui.topframe := dws.frame(parent=parent,title=self.appid,relief='groove');
    self.gui.topframe->unmap();
    # set 'killed' handler
    whenever self.gui.topframe->killed do 
      self.gui := F;
    # create state window
    self.gui.stateframe := dws.frame(parent=self.gui.topframe,side='left',expand='none');
    self.gui.state_lbl := dws.label(self.gui.stateframe,spaste(self.appid,': '));
    self.gui.state := dws.entry(self.gui.stateframe,width=30,
                               relief='sunken',disabled=T);
    # create command buttons
    self.gui.cmd_frame := dws.frame(parent=self.gui.topframe,side='left',expand='none');
    self.gui.pause := dws.button(self.gui.cmd_frame,'PAUSE');
    self.gui.resume := dws.button(self.gui.cmd_frame,'RESUME');
    self.gui.updstatus := dws.button(self.gui.cmd_frame,'Update status');
    self.gui.resume_pad := dws.frame(self.gui.cmd_frame,expand='none',width=20,height=5);
    self.gui.init := dws.button(self.gui.cmd_frame,'INIT');
    self.make_command_dialog('Init','Init',self.gui.init);
    self.gui.stop := dws.button(self.gui.cmd_frame,'STOP');
    self.gui.stop_pad := dws.frame(self.gui.cmd_frame,expand='none',width=20,height=5);
    self.gui.halt := dws.button(self.gui.cmd_frame,'HALT');
    # register event handlers for command buttons
    whenever self.gui.pause->press do public.pause(from_gui=T);
    whenever self.gui.resume->press do public.resume(from_gui=T);
    whenever self.gui.stop->press do public.stop(from_gui=T);
    whenever self.gui.halt->press do public.halt(from_gui=T);
    # create event logger
    self.gui.eventlog := self.make_text_frame(self.gui.topframe,disabled=T,size=[60,20]);
    self.gui.eventlog.text->config('event',foreground="#808080");
    self.gui.eventlog.text->config('text',foreground="black");
    self.gui.eventlog.text->config('error',foreground="red");
    # create second command panel
    self.gui.cmd2_frame := dws.frame(parent=self.gui.topframe,side='right',expand='none');
    self.gui.updstatus := dws.button(self.gui.cmd2_frame,'Update status');
    # create status record panel
    self.gui.statusbrowser := dws.recordbrowser(self.gui.topframe,[status='none']);
#    self.gui.statusrec_tf := dws.frame(self.gui.topframe,borderwidth=0);
#    self.gui.statusrec_f1 := dws.frame(self.gui.statusrec_tf,borderwidth=0);
#    self.gui.statusrec_f2 := dws.frame(self.gui.statusrec_tf,borderwidth=0);
    self.gui.statusrec := [=];
    whenever self.gui.updstatus->press do public.reqstatus();
    # create general event handler
    whenever self.relay->* do
    {
      report := T;
      name := $name;
      if( $name ~ m/^sending_command_/ )
        report := F;
      # update state
      else if( $name == 'app_notify_state' ) 
      {
        self.gui.state->delete('0','end');
        self.gui.state->insert($value.state_string);
      }
      # update status record
      else if( name =~ s/^app_update_status_// )
      {
        name := split(name,'/');
        if( len(name) == 1 )
        {
          self.gui.statusrec[name] := [=];
          for( f in field_names($value) )
            self.gui.statusrec[f] := $value[f];
        }
        else
        {
          if( !has_field(self.gui.statusrec,name[1]) ||
              !is_record(self.gui.statusrec[name[1]]) )
            self.gui.statusrec[name[1]] := [=];
          for( f in field_names($value[name[1]]) )
            self.gui.statusrec[name[1]][f] := $value[name[1]][f];
        }
        self.gui.statusbrowser->newrecord(self.gui.statusrec);
        report := F;
      }
      else if( $name ~ m/^app_status/ )
      {
        self.gui.statusrec := $value.value;
        self.gui.statusbrowser->newrecord(self.gui.statusrec);
        report := F;
      }
      # update event log
      if( report )
      {
        self.gui.eventlog.text->append(spaste($name,': ',$value,'\n'),'event');
        if( has_field($value,'text') )
        {
          tag := 'text';
          if( has_field($value,'error') )
            tag := 'error';
          self.gui.eventlog.text->append(spaste($value.text,'\n'),tag);
        }
      }
    }
    self.gui.topframe->map();
    return ref self.gui.topframe;
  }
  const public.kill_gui_frame := function ()
  {
    wider self;
    self.gui := F;
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
  
  # create gui if asked to do so
  if( gui || !is_boolean(parent_frame) )
  {
    public.make_gui_frame(parent_frame);
  }
  
  return T;
}

const app_proxy := function(appid,
        server=F,options=F,suspend=F,verbose=1,
        gui=F,parent_frame=F)
{
  self := [=];
  public := [=];
  ret := define_app_proxy(self,public,appid,server,options,suspend,verbose,
                          gui,parent_frame);
  if( is_fail(ret) )
    fail;
  return ref public;
}
