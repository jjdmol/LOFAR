include 'widgetserver.g'
include 'app_proxy.g'

msname :='test.MS';

mssel := 'ANTENNA1 in 4*[0:20] && ANTENNA2 in 4*[0:20]';
datacol := 'DATA';

selection := [ ddid_index=1,field_index=1,
               selection_string=mssel ];

inputrec := [ ms_name = msname,
              data_column_name = datacol,
              tile_size = 1, selection = selection ];

prefix := hiid('a');

verbose := 1;
suspend := F;

print "Creating data repeater";
start_octopussy('./applauncher',"-d0 -rpt:M:O:Repeater",
                suspend=suspend);

parent := dws.frame(title='Repeater',side='left');
parent->unmap();

rpt := app_proxy('Repeater',verbose=verbose,parent_frame=parent,gui=T);

if( is_fail(rpt) )
{
  print 'Repeater start failed: ',rpt;
}
else
{
  parent->map();
  rpt.init([=],inputrec,[event_map_out=[default_prefix=prefix]],wait=T);
}

