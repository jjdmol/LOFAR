pragma include once
include 'gsm/skymodel.g'

if( !is_string(default_gsm_table_name) )
  default_gsm_table_name := 'gsm.tab';


#
# this creates a global gsm object when first called, and
# returns a ref to it on subsequent calls
#
const gsm := function (gsmname=default_gsm_table_name,readonly=T,verbose=1)
{
  global __gsm_tool;
  if( !is_skymodel(__gsm_tool) )
  {
    __gsm_tool := skymodel(gsmname,readonly=readonly,meptable='',verbose=verbose,appid='gsm');
    if( is_fail(__gsm_tool) )
    {
      print 'failed to create global GSM object: ',__gsm_tool;
      return F;
    }
    __gsm_tool.dprintf(1,'attached global GSM object to table %s',gsmname);
  }
  return ref __gsm_tool;
}

gsm(verbose=1);


gsm_test := function (browse=F)
{
  global lsm,sx,newsrc;
  gsm().setverbose(5);
 
  # extract LSM from GSM
  gsm().dprint(0,'------------------------ test: extract_lsm ---------------');
  lsm := gsm().extract_lsm([ra=0,dec=0,radius=600],readonly=F);
  lsm.setverbose(5);
  lsm.mepdb().setverbose(5);
  
  # browse the MEP DB table, if needed
  if( browse )
    lsm.mepdb().table().browse();
    
  # select sources from LSM, in specific order
  lsm.dprint(0,'------------------------ test: select_sources ------------');
  sx := lsm.select_sources(sort_by='I0 desc',fields="RA I0");
  print len(sx);
  
  # construct defrec for Stokes I of source 1. This should be ==I0, since 
  # SP_INDEX is 0 and non-solvable
  lsm.dprint(0,'------------------------ test: get_node_defrec(1) --------');
  print 'defrec_i: ',lsm.get_node_defrec(sx[1],'I');
  
  # construct defrec for XSIZE1 of source 1. A new MEP will be created
  lsm.dprint(0,'------------------------ test: get_node_defrec(2) --------');
  print 'defrec_xsize1: ',lsm.get_node_defrec(sx[1],'XSIZE1');
  print 'sx[1]: ',sx[1];
  
  # Source 2: first, make SP_INDEX solvable. Then get a defrec for Stokes I.
  # This should now use spectral index representatrion
  lsm.dprint(0,'------------------------ test: get_node_defrec(3) --------');
  print 'defrec_spi: ',lsm.get_node_defrec(sx[2],'SP_INDEX',solvable=T);
  print 'defrec_i: ',lsm.get_node_defrec(sx[2],'I',solvable=T);
  print 'sx[2]: ',sx[2];
  
  
  # Create a new source on-the-fly (without writing to LSM)
  lsm.dprint(0,'------------------------ test: make_source ---------------');
  newsrc := lsm.make_source(0,0,1);
  print 'newsrc: ',newsrc;
  
  # construct I defrec for new source. This will automatically create
  # new MEPs
  lsm.dprint(0,'------------------------ test: get_node_defrec(4) --------');
  print 'defrec_spi: ',lsm.get_node_defrec(newsrc,'SP_INDEX',solvable=T);
  print 'defrec_i: ',lsm.get_node_defrec(newsrc,'I');
  print 'newsrc: ',newsrc;

  # commit changes
  lsm.dprint(0,'------------------------ test: commit_source -------------');
  # this should commit the new XSIZE1 MEP
  print lsm.commit_source(sx[1]);
  # this should commit the new source, plus its MEPs
  print lsm.commit_source(newsrc);
}
