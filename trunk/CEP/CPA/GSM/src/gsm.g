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
  global lsm;
  global sx;
  gsm().setverbose(3);
  lsm := gsm().extract_lsm([ra=0,dec=0,radius=600]);
  if( browse )
    lsm.mepdb().table().browse();
  sx := lsm.select_sources(sort_by='I0 desc',fields="RA I0");
  print len(sx);
}
