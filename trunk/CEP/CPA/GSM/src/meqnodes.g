pragma include once

# meqnodes.g
#   Includes some useful definitions for meqnodes

# meqparm
#   Returns a defrec describing a MeqParm node
const meqparm := function (name=F,polc=F,meprec=[=],solvable=F,new_mep=F)
{
  defrec := meprec;
  # put in name and polc arguments
  if( is_string(name) )
    defrec.name := name;
  if( !is_boolean(polc) )
    defrec.polc := polc;
  defrec.class := 'meqparm';
  defrec.solvable := solvable;
  # either a DBID must be available, or creation of a new MEP requested
  if( !has_field(defrec,'_dbid') )
  {
    if( new_mep )
      defrec._new_mep := T;
    else if( is_string(name) )
      fail 'must create meqparm from valid meprec, or use unnamed, or use new_mep=T';
  }
  return ref defrec;
}

# meqconst
#   Returns a defrec describing a MeqConst node (a t,f polynomical with constant
#   coefficients)
#   params is an optional record of constant parameters
const meqconst := function (polc,name='')
{
  return ref [ class='meqconst',polc=polc,name=name ];
}
# meqexpr
#   Returns a defrec describing a MeqExpression node
#   args is a record of refs to expression arguments
#   params is an optional record of constant parameters
const meqexpr := function (funcname,ref args,params=F)
{
  return ref [ class=spaste('meqexpr_',funcname),children=ref args,params=params ];
}

# meqexpr1, 2, 3
#   Aliases for meqexprs of 1, 2 or 3 arguments
const meqexpr1 := function (funcname,ref arg1,params=F)
{
  return ref meqexpr(funcname,[a1=ref arg1],params=params); 
}
const meqexpr2 := function (funcname,ref arg1,ref arg2,params=F)
{
  return ref meqexpr(funcname,[a1=ref arg1,a2=ref arg2],params=params); 
}
const meqexpr3 := function (funcname,ref arg1,ref arg2,ref arg3,params=F)
{
  return ref meqexpr(funcname,[a1=ref arg1,a2=ref arg2,a3=ref arg3],params=params); 
}
