pragma include once
include 'table.g'
include 'debug_methods.g'

const mep_database := function (tablename='mep.db',readonly=T,create=F,
                                verbose=1,appid='mepdb')
{
  self := [ appid=appid ];
  public := [ self=ref self ];
  define_debug_methods(self,public,verbose);
  
  # this record maps table columns to meprec fields 
  const self.col_to_rec := [ ID='name',VALUE='polc',
      FREQ_DOMAIN='freq',TIME_DOMAIN='time',
      VERSION='version',COMMENT='comment' ];
  # this record maps meprec fields to table columns
  # (constructed automatically) 
  self.rec_to_col := [=];
  for( col in field_names(self.col_to_rec) )
    self.rec_to_col[self.col_to_rec[col]] := col; 
  const self.rec_to_col := self.rec_to_col;
  
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
  #   Gets the mep record for an existing MEP, specified by name or #
  const public.get_mep := function (parm,fields="name polc")
  {
    wider self,public;
    if( is_string(parm) )
      parm := public.lookup_id(parm);
    if( parm<1 )
      fail 'parameter not found';
    self.dprintf(3,'reading mep from row %d',parm);
    # this translates a table row into a meprec
    prec := self.tbl_row.get(parm);
    meprec := [ _dbid=parm ];
    for( f in fields )
      if( has_field(self.rec_to_col,f) )
      {
        col := self.rec_to_col[f];
        if( has_field(prec,col) )
          meprec[f] := prec[col];
      }
    return ref meprec;
  }
  
  # commit_mep()
  #   Commits a MEP to the database, if the MEP is marked as updated:
  #     meprec._updated = 0 (or undefined): no commit
  #                     > 0: commit polcs
  #                     > 1: commit everything
  #   If force>0, then this overrides meprec._updated
  #   If MEP is new (meprec._new_mep=T), then a new row is inserted,
  #   and a commit of everything is forced.
  const public.commit_mep := function (ref meprec,force=0)
  {
    wider self,public;
    # allocate new row if just creating the mep
    if( has_field(meprec,'_new_mep') && meprec._new_mep )
    {
      res := self.tbl.addrows(1);
      if( is_fail(res) )
        fail res;
      meprec._dbid := self.tbl.nrows();
      meprec._new_mep := F;
      if( !has_field(meprec,'freq') )
        meprec.freq := [0.,0.];
      if( !has_field(meprec,'time') )
        meprec.time := [0.,0.];
      if( !has_field(meprec,'version') )
        meprec.version := '0';
      force := 10;
      self.dprintf(2,'commit_mep: created new mep at row %d',meprec._dbid);
    }
    # write table if forced or updated
    if( has_field(meprec,'_updated') && meprec._updated > force )
      force := meprec_.updated;
    if( force>0 )
    {
      if( !has_field(meprec,'_dbid') )
        fail 'confusion, no dbid field in mep record';
      irow := meprec._dbid;
      self.dprintf(2,'commit_mep: writing %s to row %d',meprec.name,irow);
      self.dprint(4,'commit_mep: value is ',meprec.polc);
#      # this translates a meprec into a table row
#      prec := [ ID=meprec.name,VALUE=meprec.polc ];  
#      res := self.tbl_row.put(irow,prec);
      res := force>0 && self.tbl.putcell('VALUE',irow,meprec.polc);
      if( force>1 )
        for( f in "name freq time version comment" )
          if( has_field(meprec,f) )
            res +:= self.tbl.putcell(self.rec_to_col[f],irow,meprec[f]);
      if( is_fail(res) )
        fail res;
      meprec._updated := F;
    }
    else
      self.dprintf(4,'commit_mep: skipping %s: not updated',meprec.name);
    return ref meprec;
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

