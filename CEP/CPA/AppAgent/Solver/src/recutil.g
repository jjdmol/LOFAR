pragma include once

const val2lines := function (x,indent="",maxlen=60)
{
  if( is_numeric(x) )
  {
    if( len(x) == 1 )
      return as_string(x);
    else
      return paste('[',paste(as_string(x),sep=','),']');
  }
  else if( is_string(x) )
  {
    if( x::is_hiid )
      return spaste('hiid("',paste(x),'")');
    else
    {
      if( len(x) == 1 )
        return spaste("'",x,"'");
      else
        return spaste("['",paste(rec[f],sep="','"),"']");
    }
  }
  else if( is_record(x) )
    return rec2lines(rec);
}

const rec2lines := function (rec,indent="",maxlen=60)
{
  out := "";
  for( f in field_names(rec) )
  {
    iout := len(out)+1;
    if( is_numeric(rec[f]) )
    {
      if( len(rec[f]) == 1 )
        out[iout] := paste(f,'=',rec[f]);
      else
        out[iout] := paste(f,'= [',paste(as_string(rec[f]),sep=','),']');
    }
    else if( is_string(rec[f]) )
    {
      if( rec[f]::is_hiid )
        out[iout] := paste(f,'=',spaste('hiid("',paste(rec[f]),'")'));
      else
      {
        if( len(rec[f]) == 1 )
          out[iout] := spaste(f,' = ',"'",rec[f],"'");
        else
          out[iout] := spaste(f,' = [',"'",paste(rec[f],sep="','"),"']");
      }
    }
    else if( is_record(rec[f]) )
    {
      reclines := rec2lines(rec[f],spaste(indent,'  '));
      if( len(reclines) )
      {
        if( sum(strlen(reclines)) <= maxlen )
          out[iout] := paste(f,'= [',paste(reclines,sep=','),']');
        else
        {
          out[iout] := paste(f,'= [');
          indent1 := spaste(array(' ',strlen(out[iout])+1));
          first := T;
          for( l in reclines )
          {
            if( first )
            {
              out[iout] := paste(out[iout],l);
              first := F;
            }
            else
            {
              iout+:=1;
              out[iout] := spaste(indent1,l);
            }
          }
          out[iout] := paste(out[iout],']');
        }
      }
      else
        out[iout] := paste(f,"= [=]");
    }
#    print iout,f,out[iout];
  }
  #  print paste(out,sep='\n')
  return out;
}

const rec2string := function (rec)
{
  lines := rec2lines(rec);
  if( sum(strlen(lines)) <= maxlen )
    return paste(lines,sep=', ');
  else
    return paste(lines,sep=',\n');
}

const string2rec := function(str)
{
  str =~ s/^\s+//;
  str =~ s/\s+$//;
  if( !strlen(str) )
    return [=];
  else
    return eval(spaste("[",str,"]"));
}

## include 'octopussy.g'
## 
## d := "jdhdhjdfhdfjfjd dsfjjsdfsdfjdfsjhjf sdfjsfdjkfsdjdfsjk";
## rec := [ a="help me",b=[1,2,3],c=hiid("x.y.z"), d=[x=d,y=d,z=d] ];
## 
## print paste(rec2lines(rec),sep='\n')
## 
## s := rec2string(rec);
## print s;
