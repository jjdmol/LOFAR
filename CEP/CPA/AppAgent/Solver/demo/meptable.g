pragma include once
include 'table.g'

const read_mep_table := function (fname,ref mep=[=],verbose=0)
{
  tbl := table(fname,readonly=T);
  if( is_fail(tbl) )
    fail;
  cols := tbl.colnames();
  if( is_fail(cols) )
    fail;
  # read the names column
  names := tbl.getcol('NAME');
  if( is_fail(names) )
    fail 'missing NAME column';
  # read each column into a record
  colrec := [=];
  indexrec := [=];   # record of indexing records, for multidim arrays
  for( col in cols )
  {
    if( col == 'NAME' )
      next;
    x := tbl.getcol(col);
    if( is_fail(x) )
    {
      if( verbose>0 ) print 'failed to read column ',col,' as whole';
      next;
    }
    if( has_field(x::,'shape') && len(x::shape) > 1 )  # array column
    {
      if( x::shape[len(x::shape)] != len(names) )
      {
        if( verbose>0 ) print 'array column ',col,': length mismatch, skipping';
        next;
      }
      if( verbose>2 ) print 'read array column ',col;
      indexrec[col] := [=];
      for( i in 1:len(x::shape) )
        indexrec[col][i] := [];
      colrec[col] := x;
    }
    else # scalar column
    {
      if( len(x) != len(names) )
      {
        if( verbose>0 ) { print 'column ',col,': length mismatch, skipping'; }
      }
      else
      {
        if( verbose>2 ) print 'read scalar column ',col;
        colrec[col] := x;
      }
    }
  }
  # create name-indexed record
  for( i in 1:len(names) )
  {
    mep[names[i]] := [=];
    for( col in cols )  
    {
      if( col == 'NAME' )
        next;
      if( has_field(colrec,col) ) # column already fetched
      {
        if( has_field(indexrec,col) ) # array column
        {
          index := indexrec[col];
          index[len(index)] := i;
          mep[names[i]][col] := colrec[col][index];
        }
        else # scalar column
          mep[names[i]][col] := colrec[col][i];
      }
      else  # fall back to getcell
      {
        cell := tbl.getcell(col,i);
        if( !is_fail(cell) )
          mep[names[i]][col] := cell;
      }
    }
  }
  return ref mep;
}

# demomep := read_mep_table('demo.MEP');
# demomep := read_mep_table('demo_gsm.MEP',demomep);

