include 'meq/meqserver.g'

default_verbosity := 1;
max_debug := 1;

if( has_field(environ,'verbose') )
  default_verbosity := as_integer(environ.verbose);
if( has_field(environ,'debug') )
  max_debug := as_integer(environ.debug);

default_debuglevels := [  MeqNode       =3,
                          MeqForest     =3,
                          MeqSink       =3,
                          MeqSpigot     =3,
                          MeqVisHandler =3,
                          MeqServer     =3,
                          meqserver     =1      ];

for( f in 1:len(default_debuglevels) )
  default_debuglevels[f] := min(max_debug,default_debuglevels[f]);
  
print '======= Default verbosity level: ',default_verbosity;
print '=======         Max debug level: ',max_debug;

# inits a meq.server
const mqsinit := function (verbose=default_verbosity,debug=[=],gui=use_gui)
{
  global mqs;
  if( !is_record(mqs) )
  {
    mqs := meq.server(verbose=verbose,options="-d0 -nogw -meq:M:M:MeqServer",gui=gui);
    if( is_fail(mqs) )
      fail;
#    mqs.setdebug('Glish',5);
    mqs.init([output_col="PREDICT"],wait=T);
    if( is_record(debug) )
    {
      for( lev in field_names(default_debuglevels) )
        mqs.setdebug(lev,default_debuglevels[lev]);
      for( lev in field_names(debug) )
        mqs.setdebug(lev,debug[lev]);
    }
  }
  return mqs;
}


