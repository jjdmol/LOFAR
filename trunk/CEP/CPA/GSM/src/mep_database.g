pragma include once
include 'table.g'
include 'debug_methods.g'

const mep_database := function (tablename='mep.db',readonly=T,create=F,
                                verbose=1,appid='mepdb')
{
  self := [ appid=appid ];
  public := [ self=ref self ];
  define_debug_methods(self,public,verbose);
  
  # internal table initializer
  const self.init_table := function ()
  {
    wider self;
    d1 := tablecreatescalarcoldesc('ID','');
    # domains
    d2 := tablecreatearraycoldesc('FREQ_DOMAIN',0.,ndim=1,shape=[2]);
    d3 := tablecreatearraycoldesc('TIME_DOMAIN',0.,ndim=1,shape=[2]);
    # values
    d4 := tablecreatearraycoldesc('VALUE',0.);
    # version and date info
    d5 := tablecreatescalarcoldesc('VERSION','');
    d6 := tablecreatescalarcoldesc('COMMENT','');
    td := tablecreatedesc(d1,d2,d3,d4,d5,d6);
    self.tbl := table(self.tablename,td);
    if( is_fail(self.tbl) )
      fail self.tbl;
    self.dprint(1,'created new table ',self.tablename);
    return T;
  }
  
  # done():
  #   destructor
  const public.done := function ()
  {
    wider public,self;
    if( has_field(self,'tbl_index') )
    {
      self.dprintf(1,'closing table index');
      self.tbl_index.done();
    }
    self.dprintf(1,'closing table %s',self.tablename);
    self.tbl_row.done();
    self.tbl.done();
    val self := F;
    val public := F;
    return T;
  }
  
  # table(): accessor to table object
  const public.table := function ()
  { wider self; return ref self.tbl; }
  
  # lookup_id():
  #   looks up source ID and returns row number (0 for none)
  #   time and freq are meant to specify domain, but are ignored for now
  const public.lookup_id := function (id,time=F,freq=F)
  {
    wider self;
    # construct an index for the ID column, if not already done so
    if( !has_field(self,'tbl_index') )
      self.tbl_index := tableindex(self.tbl,'ID');
    return self.tbl_index.rownr(id);
  }
  
  # get_mep()
  #   Gets the mep record for a MEP specified by name or #
  const public.get_mep := function (parm)
  {
    wider self,public;
    if( is_string(parm) )
      parm := public.lookup_id(parm);
    if( parm<1 )
      fail 'parameter not found';
    meprec := self.tbl_row.get(parm);
    meprec._dbid := parm;
    return meprec;
  }
  
  # add_parm():
  #   adds a single MEP
  const public.add_parm := function (id,value,freq=F,time=F,
                                     version='1',comment='')
  {
    wider self,public;
    nparms := len(id);
    # check for existing instances of this parameter
    nrow := self.tbl.nrows()+1;
    self.tbl.add
    return nrow;
  }
  
  # add_parm_batch()
  #   Add a batch of new MEPs
  #   ids: vector of string IDs
  #   values: vector or record of values. Use a record if values are arrays.
  const public.add_parm_batch := function (ids,values,freq=F,time=F,
                                           versions='1',comments='')
  {
    wider self,public;
    row0 := self.tbl.nrows() + 1;
    nparms := len(ids);
    self.dprintf(2,'batch-adding %d params',nparms);
    self.tbl.addrows(nparms);
    # put IDs and values
    res := self.tbl.putcol('ID',ids,row0);
    if( is_fail(res) ) fail res;
    for( i in 1:len(values) )
    {
      res := self.tbl.putcell('VALUE',row0+i-1,values[i]);
      if( is_fail(res) ) fail res;
    }
    # renormalize time and freq arrays
    if( is_boolean(freq) )
      freq := array(0.,2,nparms);
    else if( len(freq) == 2 )
      freq := array(freq,2,nparms);
    if( is_boolean(time) )
      time := array(0.,2,nparms);
    else if( len(time) == 2 )
      time := array(time,2,nparms);
    self.tbl.putcol('TIME_DOMAIN',time,row0);
    self.tbl.putcol('FREQ_DOMAIN',freq,row0);
    self.tbl.putcol('VERSION',array(versions,nparms),row0);
    self.tbl.putcol('COMMENT',array(comments,nparms),row0);
    return row0:(row0+nparms-1);
  }

  #
  # rest of constructor code
  #
  
  self.tablename := tablename;
  # init table if asked to do so
  if( create )
  {
    tabledelete(tablename);
    res := self.init_table();
    if( is_fail(res) )
    {
      self.dprint(1,'table init failed');
      fail res;
    }
  }
  # else attach to existing table
  else
  {
    self.tbl := table(tablename,readonly=readonly);
    if( is_fail(self.tbl) )
      fail;
    self.dprintf(1,'attached to table %s, %d rows',self.tablename,self.tbl.nrows());
  }
  self.tbl_row := tablerow(self.tbl);

  public::class := 'mep_database';
  return ref public; 
}

const is_mep_database := function (tool)
{
  return is_record(tool) && tool::class == 'mep_database';
}

