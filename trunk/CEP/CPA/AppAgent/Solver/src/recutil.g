pragma include once

# returns array of strings in the form of 'string' (one element),
# or "x y z", or ['x','y','z'], depending on whitespace content
const str2string := function (x,skip_braces)
{
  if( len(x) == 1 )
    return spaste("'",x,"'");
  else
  {
    # do the individual strings contain whitespace?
    for( str in x )
      if( str ~ m/[ \t]/ )  # got whitespace -- return as explicit array
      {
        out := spaste("'",paste(x,sep="','"),"'");
        if( !skip_braces )
          out := spaste('[',out,']');
        return out;
      }
    # no whitespace -- return in "x y z" form
    return spaste('"',paste(x),'"');
  }
}

# Converts a value to a string (or an array of strings, if dealing with
# a long record that won't fit around maxlen characters).
# If skip_braces=F, then braces will be added around the value as needed
# (i.e. if record or array)
const val2string := function (x,maxlen=60,skip_braces=F)
{
  if( is_numeric(x) )
  {
    if( len(x) == 1 )
      return as_string(x);
    else
    {
      out := paste(as_string(x),sep=',');
      if( has_field(x::,'shape') )  # multi-dimensional array?
        return spaste('array([',out,'],',paste(as_string(x::shape),sep=','),')');
      else
      {
        if( !skip_braces )
          out := paste('[',out,']');
        return out;
      }
    }
  }
  else if( is_string(x) )
  {
    multidim := has_field(x::,'shape');
    is_hiid := has_field(x::,'is_hiid') && x::is_hiid;
    # convert to string form -- skip braces if hiid, add braces if multidim
    out := str2string(x,is_hiid || (skip_braces && !multidim));
    # add 'hiid(x)' if hiid
    if( is_hiid )
      out := spaste("hiid(",out,")");
    # if multidimensional, add 'array(x,shape)'
    if( multidim )
      return spaste('array(',out,',',paste(as_string(x::shape),sep=','),')');
  }
  else if( is_record(x) )
  {
    out := rec2lines(x,maxlen);
    # one line, or several short lines? Return inline form
    if( len(out) == 1 || sum(strlen(out)) <= maxlen )
    {
      out := paste(out);
      if( !skip_braces )
        return paste('[',out,']');
    }
    else if( !skip_braces) # multiple lines, gotta add braces/indent
    {
      out[1] := spaste('[ ',out[1]);
      for( i in 2:len(out) )
        out[i] := spaste('  ',out[i]);
      out[len(out)] := spaste(out[len(out)],' ]');
    }
  }
  return out;
}

# Helper function: returns record as
#   'a = value,'
#   'b = [ x=value,y=value],'
#   'd = [ x=really long value,
#   '      y=another really long value ],
#   'c = value'
# i.e. with no indents or surrounding braces
const rec2lines := function (rec,maxlen)
{
  # handle empty record
  fields := field_names(rec);
  nf := len(fields);
  if( !nf )
    return '=';
  # convert each record field into a separate line
  out := "";
  for( f in 1:nf )
  {
    prefix := spaste(fields[f],' = ');
    # convert field value into lines, with braces
    lines := val2string(rec[f],maxlen,F);
    if( len(lines) > 1 ) comma := ','; else comma := '';
    for( i in 1:len(lines) )
    {
      # change prefix starting at line 2
      if( i==2 )  
        prefix := spaste(array(' ',strlen(prefix)));
      out[len(out)+1] := spaste(prefix,lines[i]);
    }
    # add traling comma if not last field
    if( f < nf )
      out[len(out)] := spaste(out[len(out)],',');
  }
  return out;
}

const string2val := function(str)
{
  str := paste(str);
  # trim leading/traling whitespaces
  str =~ s/^\s+//;
  str =~ s/\s+$//;
  # if no leading brace, surround with braces
  if( !(str ~ m/^\[/) )
    str := spaste('[',str,']');
  return eval(str);
}

## include 'octopussy.g'
##  
## d := "jdhdhjdfhdfjfjd dsfjjsdfsdfjdfsjhjf sdfjsfdjkfsdjdfsjk";
## arr1 := array('x',2,2);
## arr2 := array('f g',2,2);
## arr3 := arr2;
## arr3::is_hiid := T;
## arr4 := "a.1 b.2 c.3 d.4";
## arr4::is_hiid := T;
## rec := [ a="help me",b=[1,2,3],c=hiid("x.y.z"), d=[x=d,y=d,z=d],
##          e=[a=0,b=1],f=array(2,3,3,3),
##          g=arr1,h=arr2,i=arr3,j=arr4 ];#
## 
## str := val2string(rec,skip_braces=T);
## 
## print paste(str,sep='\n');
## 
## print string2val(str);
## 
## 
##  
