pragma include once

# meqnodes.g
#   Includes some useful definitions for meqnodes

# meqparm
#   Returns a defrec describing a MeqParm node
const meqparm := function (name,polc,solvable=F,dbid=F,new_mep=F)
{
  defrec := [ class='meqparm',name=name,solvable=solvable,polc=polc ];
  if( !is_boolean(dbid) )
    defrec._dbid := dbid;
  if( new_mep )
    defrec._new_mep := T;
  return ref defrec;
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
