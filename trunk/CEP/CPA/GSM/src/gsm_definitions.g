pragma include once
include 'table.g'

# define the GSM namespace
GSM := [=];

# GSM.column_info_rec:
#   This contains info on each GSM table column
#
#   (this is a helper function to define the record)
const GSM._define_column_info_rec := function ()
{
  # local function to define info for one columns
  local Define := function (ref templ,name,value,unit=F,category=F,param=T,ucd=T)
  {
    ct := [ name=name,value=value,
            param=param,
            typename=type_name(value) ];
    # by default, use name for ucd
    if( is_boolean(ucd) && ucd)
      ucd := name;
    # fill in optional fields
    if( is_string(unit) )
      ct.unit := unit;
    if( is_string(category) )
      ct.category := category;
    if( is_string(ucd) )
      ct.ucd := ucd; 
    # add to template   
    templ[name] := const ct;
  }
  
  templ := [=];
  # name of object
  Define(templ,'ID','',category='id',param=F,ucd='ID_MAIN');
  # J2000 position (rad)
  Define(templ,'RA',0.0,'rad','position',ucd='POS_EQ_RA_MAIN');
  Define(templ,'DEC',0.0,'rad','position',ucd='POS_EQ_DEC_MAIN');
  # Stokes fluxes (Jy)
  Define(templ,'I0',0.0,'Jy','flux',ucd='POL_STOKES_I');
  Define(templ,'Q0',0.0,'Jy','flux:pol',ucd='POL_STOKES_Q');
  Define(templ,'U0',0.0,'Jy','flux:pol',ucd='POL_STOKES_U');
  Define(templ,'V0',0.0,'Jy','flux:pol',ucd='POL_STOKES_V');
  # Frequency at which fluxes above are valid (MHz)
  Define(templ,'FREQ0',0.0,'MHz','reference/freq',ucd=F,param=F);
  # spectral index (0 for none)
  Define(templ,'SP_INDEX',0.0,category='spectrum',ucd='SPECT_SP-INDEX');
  # rotation measure (rad*MHz^2)
  Define(templ,'RMI',0.0,'rad/m2','pol',ucd='PHYS_ROTATION-MEASURE');
  # for extended sources only
  #   * size of major and minor axes (rad)
  Define(templ,'XSIZE1',0.0,'arcsec','extended',ucd='PHYS_SIZE_MAJ');
  Define(templ,'XSIZE2',0.0,'arcsec','extended',ucd='PHYS_SIZE_MIN');
  #   * positional angle of major axis (rad)
  Define(templ,'XORIENT',0.0,'rad','extended',ucd='POS_POS-ANG');
  # other IDs (cross-identifications, etc.) 
  Define(templ,'ID_ALTERNATIVE','',category='id',param=F,ucd='ID_ALTERNATIVE');
  # additional comments, cross-identifications. etc. (any text) 
  Define(templ,'COMMENTS','',category='id',param=F);
  
  return templ;
}

#     (define the actual record here)
const GSM.column_info_rec := GSM._define_column_info_rec();
if( is_fail(GSM.column_info_rec) )
  fail GSM.column_info_rec;
  
# GSM.column_names: vector of column names
const GSM.column_names := field_names(GSM.column_info_rec);

# GSM.num_columns: total # of columns
const GSM.num_columns := len(GSM.column_info_rec);

# GSM.column_info(col):
#   returns column info for a particular column (by name or #)
const GSM.column_info := function(column)
{
  return GSM.column_info_rec[column];
}

# GSM.source_parameters
#   list of all source parameters: generated automagically from column info
GSM.source_parameters := "";
for( col in GSM.column_names )
  if( GSM.column_info(col).param )
    GSM.source_parameters := [GSM.source_parameters,col];
const GSM.source_parameters := GSM.source_parameters;

# GSM.required_columns
#   These columns are always present in source records returned by select_sources
const GSM.required_columns := "ID";
# GSM.default_columns
#   These columns are put into source records by default, in addition to 
#   required_columns
const GSM.default_columns := "RA DEC I0 FREQ0";

# GSM.row_template:
#   template record for a single GSM row (fields are columns, values
#   are default (null) values)
GSM.row_template := [=];
for( col in GSM.column_names )
  GSM.row_template[col] := GSM.column_info_rec[col].value;
const GSM.row_template := GSM.row_template;


# This defines the GSM table descriptor
    # ugly hack here, to get around Glish limitations. What we really want
    # to do is to build up a '...' set of arguments, to pass it to 
    # tablecreatedesc. But we can't, hence the ugly recursive hack:
GSM.build_recursive_tabledesc := function(icol,...)
{
  ci := GSM.column_info(icol);
  coldesc := tablecreatescalarcoldesc(ci.name,ci.value);
  # out of columns? Create the tabledesc, using all coldescs accumulated in ...
  if( icol < 2 )
    return tablecreatedesc(coldesc,...);
  # else keep on accumulating
  else
    return GSM.build_recursive_tabledesc(icol-1,coldesc,...);
}

GSM_cached_tabledesc := F;
const GSM.tabledesc := function ()
{
  # on first call, build the tabledesc, and cache it for later calls
  if( is_boolean(GSM_cached_tabledesc) )
  {
    global GSM_cached_tabledesc; 
    const GSM_cached_tabledesc := GSM.build_recursive_tabledesc(len(GSM.column_info_rec));
  }
  if( is_fail(GSM_cached_tabledesc) )
    fail GSM_cached_tabledesc;
  return const ref GSM_cached_tabledesc;
}

# GSM.inittable:
#   Initialize a GSM table
const GSM.inittable := function (tablename='gsm.tab',nrows=0,zero=F)
{
  wider GSM;
  tabledelete(tablename);
  if( is_fail(tablename) )
    print 'creating new GSM table',tablename;
  # create table using the tabledesc above
  tbl := table(tablename,GSM.tabledesc(),nrows);
  # fill in various column keywords
  for( icol in 1:GSM.num_columns )
  {
    ci := GSM.column_info(icol);
    # copy over keywords from cell template record
    for( keyword in "unit category param ucd" )
      if( has_field(ci,keyword) )
        tbl.putcolkeyword(ci.name,to_upper(keyword),ci[keyword]);
    # fill in zeroes, if so requested
    if( zero )
      tbl.putcol(ci.name,ci.value,nrow=nrows);
  }
  return ref tbl;
}

# GSM.make_source
#   Makes a source record from supplied components
const GSM.make_source := function (id,ra,dec,flux,other)
{
  srcrec := [=];
  srcrec.ID := id;
  srcrec.RA := ra;
  srcrec.DEC := dec;
  # flux is either one element (I), or all four
  srcrec.I0 := flux[1];
  if( len(flux) > 1 )
  {
    if( len(flux) != 4 )
      fail 'flux must be specified as I or [I,Q,U,V] array';
    srcrec.Q0 := flux[2];
    srcrec.U0 := flux[3];
    srcrec.V0 := flux[4];
  }
  else
    srcrec.Q0 := srcrec.U0 := srcrec.V0 := 0;
  # copy other fields
  if( is_record(other) )
    for( f in field_names(other) )
      srcrec[f] := other[f];
  return srcrec;
}

const GSM := GSM;
