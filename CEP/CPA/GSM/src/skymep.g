pragma include once
include 'debug_methods.g'
include 'gsm/gsm_definitions.g'
include 'gsm/mep_database.g'
include 'gsm/meqnodes.g'

#
# skymep_tool is a helper class that (hopefully) encapsulates all
#     knowledge about GSM parameters, and building sub-trees for GSM 
#     source components.
#     It's only meant for use within the skymodel class.
#
const skymep_tool := function (ref lsm,ref mepdb,
                               appid='skymep',verbose=1 )
{
  self := [ appid=appid,lsm=ref lsm,mepdb=ref mepdb ];
  public := [ self=ref self ];
  define_debug_methods(self,public,verbose);

  # destructor    
  const public.done := function ()
  {}
  
  # parm_name():
  #   Forms full MEP name from source id and parameter id
  const public.parm_name := function (src,parm)
  {
    return spaste('src(',src,').',parm);  
  }
  
  # get_mep_defrec():
  #   Returns the defrec for a MeqParm node corresponding to the given
  #   source and parameter ID
  const self.get_mep_defrec := function(ref srcrec,parm,solvable)
  {
    wider self,public;
    mep := F;
    # if we have a source ID, go to MEP database
    if( srcrec.ID != '' )
    {
      name := public.parm_name(srcrec.ID,parm);
      mep := self.mepdb.get_mep(name);
      if( is_fail(mep) )
        mep := F;
    }
      
    # if it's a new source then we won't have MEPs for it in the DB yet,
    # so generate a new defrec on-the-fly
    if( has_field(srcrec,'_new_src') && srcrec._new_src )
    {
      x := self.lsm.get_source_field(srcrec,parm);
      if( is_fail(x) )
        fail x;
      defrec := meqparm(name,polc=x,solvable=solvable,new_mep=T);
      return ref defrec;
    }
    # else go to MEP db to get the MeqParm
    mep := self.mepdb.get_mep(name);
    if( is_fail(mep) )
      fail mep;
    return meqparm(name=mep.ID,polc=mep.VALUE,solvable=solvable,dbid=mep._dbid);
  }
  
  # create_defrec:
  #   Thisis a record of _functions_ responsible for creating node
  #   defrecs. To create a defrec for a node of some type, we'll be calling
  #     create_defrec[nodetype](srcrec,nodetype)
  #   Thus, create_defrec serves as a lookup table for functions
  self.create_defrec := [=];
  
  # create_defrec for atomic MEPs: maps to get_mep_defrec
  for( p in GSM.source_parameters )
    const self.create_defrec[p] := ref self.get_mep_defrec;
    
  # create_defrec('FPL'): implements the frequency power law:
  #                       (f/FREQ0)^SP_INDEX 
  const self.create_defrec.FPL := function(ref srcrec,nodetype,solvable)
  {
    wider self,public;
    freq0 := self.lsm.get_source_field(srcrec,'FREQ0');
    return ref 
      meqexpr2('pow',meqparm('',polc=[0,1/freq0],solvable=F),
                     public.get_node_defrec(srcrec,'SP_INDEX',solvable=solvable));
  }
  
  # create_defrec('I'): implements Stokes I as I0*FPL
  const self.create_defrec.I := function(ref srcrec,nodetype,solvable)
  {
    wider self,public;
    return ref 
      meqexpr2('product',
                public.get_node_defrec(srcrec,'I0',solvable=solvable),
                public.get_node_defrec(srcrec,'FPL',solvable=solvable)); 
  }
  
  # get_node_defrec 
  #   Creates and returns a node defrec of a specific type. 
  #   The defrec is cached, by reference, in srcrec.nodes
  const public.get_node_defrec := function (ref srcrec,nodetype,solvable=F)
  {
    wider self,public;
    # return cached reference from source record, if available, otherwise
    # call internal function to create the defrec
    if( !has_field(srcrec,'nodes') )
      srcrec.nodes := [=];
    if( !has_field(srcrec.nodes,nodetype) )
    {
      # else use internal function to create and cache the node
      if( !has_field(self.create_defrec,nodetype) )
        fail paste('unknown node type: ',nodetype);
      srcrec.nodes[nodetype] := ref 
          self.create_defrec[nodetype](srcrec,nodetype,solvable=solvable);
    }
    return ref srcrec.nodes[nodetype];
  }
  
  # set_node_defrec 
  #   Sets a node defrec of a specific type. This is useful when revising
  #   the model (i.e. creating new parms, etc.)
  #   The defrec is cached, by reference, in srcrec.nodes
  const public.set_node_defrec := function (ref srcrec,nodetype,ref defrec)
  {
    srcrec.nodes[nodetype] := ref defrec;
    return ref defrec;
  }
  
  # commit_source()
  #   commits all updated MEPs associated with source.
  #   If newsource=T, populates with all-new MEPs for the source
  const public.commit_source := function (ref srcrec,force=F)
  {
    wider self,public;
    # new source? Create MEPs for defined fields
    if( has_field(srcrec,'_new_src') && srcrec._new_src )
    {
    }
    
    
  }
  
  # populate_mepdb():
  #   Populates the MEP DB with records for source parameters.
  #   Default (sources=[]) populates for all sources.
  #   To populate for a subset, specify either a string vector of source IDs,
  #   or an int vector of source numbers.
  const public.populate_mepdb := function (sources=[],version='1')
  {
    wider self,public;
    # has a subset been specified?
    subset := 1:self.lsm.table().nrows();
    if( len(sources) > 0 )
    {
      if( is_integer(sources) )       # source numbers -- direct subset
        subset := sources;
      else if( is_string(sources) )   # source IDs -- do a lookup
      {
        subset := [];
        for( s in sources )
        {
          num := self.lsm.lookup_id(s);
          if( num>0 )
            subset := [subset,num];
          else
            self.dprintf(1,'populate_mepdb: source %s not found',s);
        }
        if( !len(subset) )
          fail 'no source subset found';
      }
      else
        fail 'illegal source subset specification';
    }
    self.dprintf(1,'populating mepdb with %d sources, version=%s',len(subset),version);
    # build list of parameterized columns
    parmcols := [ 'ID',GSM.source_parameters ];
    # accumulate param ids and values here, using helper function
    parm_id := "";
    parm_val := [=];
    add_parm := function (rowrec,parm)
    {
      wider parm_id,parm_val;
      parm_id := [parm_id,public.parm_name(rowrec.ID,parm) ];
      parm_val[len(parm_val)+1] := rowrec[parm];
    }
    # loop over sources, read in table rows, and create parameters for each
    tabrow := tablerow(self.lsm.table(),parmcols);
    for( irow in subset )
    {
      rowrec := tabrow.get(irow);
      # define basic params
      for( p in "RA DEC I0 SP_INDEX" )
        add_parm(rowrec,p);
      # conditionally define polzn params
      if( rowrec.Q0 != 0 || rowrec.U0 != 0 || rowrec.V0 != 0 )
        for( p in "Q0 U0 V0 RMI" )
          add_parm(rowrec,p);
      # conditionally define extended source params
      if( rowrec.XSIZE1 != 0 || rowrec.XSIZE2 != 0 )
        for( p in "XSIZE1 XSIZE2 XORIENT" )
          add_parm(rowrec,p);
    }
    tabrow.done();
    self.dprintf(1,'%d total params to be added',len(parm_id));
    # pass to mepdb
    res := self.mepdb.add_parm_batch(parm_id,parm_val,versions=version);
    if( is_fail(res) )
      fail res;
    return len(parm_id);
  }

  public::class := 'skymep_tool';
  return ref public;
}

const is_skymep_tool := function (tool)
{
  return is_record(tool) && tool::class == 'skymep_tool';
}


