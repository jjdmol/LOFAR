pragma include once
include 'table.g'
include 'debug_methods.g'
include 'gsm/gsm_definitions.g'
include 'gsm/mep_database.g'
include 'gsm/skymep.g'

const skymodel := function ( tablename='',readonly=T,
                             ref use_existing_table=F,
                             meptable='lsm_mep.tab',init_mep=F,
                             appid='skymodel',verbose=1 )
{
  self := [ appid=appid ];
  public := [ self=ref self ];
  define_debug_methods(self,public,verbose);
  self.mepdb := F;
  # use existing table object?
  if( is_table(use_existing_table) )
  {
    self.tbl := ref use_existing_table;
    self.tablename := self.tbl.name();
  }
  else # else create new table object
  {
    self.tablename := tablename;
    self.tbl := table(tablename,readonly=readonly);
    if( is_fail(self.tbl) )
      fail;
  }
  self.tbl_row := tablerow(self.tbl);
  self.dprintf(1,'attached to table %s, %d rows',self.tablename,self.tbl.nrows());
  
  # done():
  #   destructor
  const public.done := function ()
  {
    wider public,self;
    self.skymep := F;
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
  
  # table():
  #   define accessor to table object  
  const public.table := function () 
  { wider self; return ref self.tbl; }
  # mepdb():
  #   define accessor to mepdb object  
  const public.mepdb := function () 
  { wider self; return ref self.mepdb; }
  
  # lookup_id():
  #   looks up source ID and returns row number (0 for none)
  const public.lookup_id := function (id)
  {
    wider self;
    # construct an index for the ID column, if not already done so
    if( !has_field(self,'tbl_index') )
      self.tbl_index := tableindex(self.tbl,'ID');
    return self.tbl_index.rownr(id);
  }
  
  const public.row_template := function ()
  { return GSM.row_template; }
  
  # query():
  #   generic internal query method
  const self.query := function (qrec,sort_by='')
  {
    wider self;
    qstr := '';
    # do we have a query region?
    if( has_field(qrec,'ra') || has_field(qrec,'dec') || has_field(qrec,'radius') )
    {
      if( !has_field(qrec,'ra') || !has_field(qrec,'dec') || !has_field(qrec,'radius') )
        fail 'all or none of ra/dec/radius must be specified in query record';
      rad := (as_double(qrec.radius)/60)*(pi/180);
      qstr := sprintf('%s*sin(DEC)+%s*cos(DEC)*cos(RA-%s)>=%s',
        as_string(sin(qrec.dec)),as_string(cos(qrec.dec)),
        as_string(qrec.ra),as_string(cos(rad)));   
    }
    self.dprintf(2,'query [%s], sort by [%s]',qstr,sort_by);
    # empty query string? Replace with query matching all
    if( qstr == '' )
      qstr := 'ROWID()>-1';
    subtbl := self.tbl.query(qstr,sortlist=sort_by);
    if( is_fail(subtbl) )
      fail subtbl;
    self.dprintf(2,'query returns %d rows',subtbl.nrows());
    return ref subtbl;
  }
  
  # select_sources():
  #   selects sources and returns source records
  const public.select_sources := function(
                sort_by='I0 desc',subset=[=],
                fields=GSM.default_columns)
  {
    wider self;
    subtbl := self.query(subset,sort_by=sort_by);
    if( is_fail(subtbl) )
      fail subtbl;
    rownums := subtbl.rownumbers();
    columns := [=];
    for( f in [GSM.required_columns,fields] )
    {
      if( has_field(columns,f) ) # already read in?
        next;
      col := subtbl.getcol(f);
      if( is_fail(col) )
        self.dprintf(1,"warning: requested column %s does not exist",f);
      columns[f] := col;
    }
    sources := [=];
    for( i in 1:len(rownums) )
    {
      src := [_dbid=rownums[i]];
      for( f in field_names(columns) )
        src[f] := columns[f][i];
      sources[i] := src;
    }
    return sources;
  }
  
  # get_source_field():
  #   this gets a specific source field for a source
  #   source can be a source record (in which case the field will be looked
  #   up in or added to the record), a source ID, or a _dbid
  const public.get_source_field := function (ref source,field)
  {
    wider self,public;
    if( is_record(source) )
    {
      # is the field already in the record? return it
      if( has_field(source,field) )
        return source[field];
      # source exists in DB -- read
      if( has_field(source,'_dbid') )
      {
        x := self.tbl.getcell(field,source._dbid);
        if( is_fail(x) )
          fail x;
      } 
      else # else must be a new (uncommitted) source -- use default value
        x := GSM.row_template[field];
      source[field] := x;
      return x;
    }
    else if( is_integer(source) )
      return self.tbl.getcell(field,source);
    else if( is_string(source) )
    {
      row := public.lookup_id(source);
      if( row<1 )
        fail paste('source',source,'not found');
      return self.tbl.getcell(field,row);
    }
    else
      fail 'illegal source argument';
  }
  
  # make_source():
  #   Creates a new source and returns a source record for it.
  #   If commit=T, the source is immediately written out to the DB, along
  #   with relevant MEPs.
  #   If commit=F, nothing is written until commit_source() below is called.
  const public.make_source := function (ra,dec,flux,id='',other=F,commit=F)
  {
    wider self,public;
    if( !is_skymep_tool(self.skymep) )
      fail 'no mepdb attached';
    # check for ID uniqueness, if one is supplied
    if( id != '' && public.lookup_id(id)>0 )
      fail paste('source id',id,'already exists');
    # create source record
    srcrec := GSM.make_source(id,ra,dec,flux,other);
    srcrec._new_src := T;
    if( commit )
      public.commit_source(srcrec);
    return ref srcrec;
  }
  
  # commit_source():
  #   Commits source entry and relevant MEPs to databases.
  #   If force=F, only updated stuff is committed. If force=T, all
  #   MEPs are committed (this is useful when changing things manually)
  const public.commit_source := function (ref srcrec,force=F)
  {
    wider self,public;
    if( !is_skymep_tool(self.skymep) )
      fail 'no mepdb attached';
    # no row number in source record? Must be a new source then
    if( has_field(srcrec,'_new_src') && srcrec._new_src )
    {
      self.tbl.addrows(1);
      srcrec._dbid := self.tbl.nrows();
      # allocate new ID if none has been given 
      srcrec.
      if( !has_field(srcrec,'ID') || srcrec.ID == '' )
        srcrec.ID := spaste('new',srcrec._dbid);
      srcrec.updated := T;
    }
    # write out to table if updated
    if( has_field(srcrec,'updated') && srcrec.updated )
    {
      self.tbl_row.put(srcrec);
      srcrec.updated := F;
    }
    # write out MEPs
    self.skymep.commit_source(srcrec,force);
    srcrec._new_src := F;
    return T;
  }

  # attach_mepdb():
  #   Attach to a MEP DB. You can only do this once
  const public.attach_mepdb := function (ref mepdb=F,meptable='mep.db',
                                         readonly=T,create=F)
  {
    wider self,public;
    # attach only once 
    if( is_mep_database(self.mepdb) )
      fail 'already attached to a mep db';
    # if only a table name is specified, then create private mepdb object
    if( !is_mep_database(mepdb) )
    {
      self.dprintf(1,'opening and attaching MEP DB %s',meptable);
      mepdb := mep_database(meptable,readonly=readonly,create=create,
                            verbose=self.verbose,appid='mepdb_lsm');
      if( is_fail(mepdb) )
      {
        self.dprintf(1,'MEP DB constructor failed');
        fail mepdb;
      }
      self.mepdb_destroy_on_detach := T;
    }
    else
    {
      self.dprintf(1,'attaching to MEP DB %s',mepdb.table().tablename());
      self.mepdb_destroy_on_detach := F;
    }
    # attach to database
    self.mepdb := ref mepdb;
    # create a skymep tool
    self.skymep := skymep_tool(public,self.mepdb);
    # create links to skymep methods
    const public.get_node_defrec := ref self.skymep.get_node_defrec;
    
    return T;
  }
  
  # populate_mepdb():
  #   Populates the MEP DB with records for source parameters.
  #   Default (sources=[]) populates for all sources.
  #   To populate for a subset, specify either a string vector of source IDs,
  #   or an int vector of source numbers.
  const public.populate_mepdb := function (sources=[],version='1')
  {
    wider self,public;
    if( !is_skymep_tool(self.skymep) )
      fail 'no mepdb attached';
    return self.skymep.populate_mepdb(sources,version);
  }
  
  # extract_lsm():
  #   extracts a sub-model
  const public.extract_lsm := function(qrec,lsmname='lsm.tab',readonly=T,
                                       meptable='lsm_mep.tab',init_mep=T)
  {
    wider self,public;
    lsmtab := self.query(qrec);
    if( is_fail(lsmtab) )
      fail;
    res := lsmtab.copy(lsmname,T);
    if( is_fail(res) )
    {
      self.dprint(1,'table copy failed');
      fail res;
    }
    return skymodel(lsmname,readonly=readonly,
                    meptable=meptable,init_mep=init_mep,
                    appid='lsm',verbose=self.verbose);
  }

  # rest of constructor code
  
  # create & populate MEP if so requested  
  if( init_mep )
  {
    res := public.attach_mepdb(meptable=meptable,create=T);
    if( is_fail(res) ) fail res;
    res := public.populate_mepdb(version='0');
    if( is_fail(res) ) fail res;
  }
  else if( meptable != '' ) # try to attach to existing MEP
  {
    res := public.attach_mepdb(meptable=meptable,readonly=readonly,create=F);
    if( is_fail(res) )
      self.dprint(1,'no MEP DB attached');
  }

  public::class := 'skymodel';  
  return ref public;
}

const is_skymodel := function (tool)
{
  return is_record(tool) && tool::class == 'skymodel';
}


