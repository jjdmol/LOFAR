include 'mqsinit_test.g'
include 'table.g'
include 'measures.g'
include 'quanta.g'

# helper func
# creates fully-qualified node name, by pasting a bunch of suffixes after
# the name, separated by dots.
const fq_name := function (name,...) 
{
  return paste(name,...,sep='.');
}


# creates all source-related nodes and subtrees:
#   'stokesI':          flux
#   'ra' 'dec':         source position
#   'ra0' 'dec0':       phase center
#   'lmn','n':          LMN coordinates, N coordinate
const create_source_subtrees := function (sti,ra,dec,ra0,dec0)
{
  # meq.parm(), meq.node() return init-records
  # mqs.createnode() actually creates a node from an init-record.
  
  mqs.createnode(meq.parm('stokes_i',sti,groups="a"));
  # note the nested-record syntax here, to create child nodes implicitly
  mqs.createnode(meq.node('MeqLMN','lmn',children=[
                  ra_0  =meq.parm('ra0',ra0),
                  dec_0 =meq.parm('dec0',dec0),
                  ra    =meq.parm('ra',ra,groups="a"),
                  dec   =meq.parm('dec',dec,groups="a")]));
  mqs.createnode(meq.node('MeqSelector','n',[index=3],children="lmn"));
}

# builds an init-record for a "dft" tree for one station (st)
const sta_dft_tree := function (st)
{
  global ms_antpos; # station positions from MS
  global mepuvw;    # MEP table with UVWs
  pos := ms_antpos[st];
  # create a number of UVW nodes, depending on settings
  uvwlist := dmi.list();
  # parms from MEP table
  if( mepuvw )
  {
    dmi.add_list(uvwlist,
      meq.node('MeqComposer',fq_name('mep.uvw',st),children=meq.list(
        meq.node('MeqParm',fq_name('U',st),[table_name=mepuvw]),
        meq.node('MeqParm',fq_name('V',st),[table_name=mepuvw]),
        meq.node('MeqParm',fq_name('W',st),[table_name=mepuvw])
      )) );
  }
  # const values based on what was read from the MS
  if( !is_boolean(ms_antuvw) )
  {
    dmi.add_list(uvwlist,
      meq.node('MeqComposer',fq_name('ms.uvw',st),children=meq.list(
        meq.node('MeqConstant',fq_name('u',st),[value=ms_antuvw[1,st]]),
        meq.node('MeqConstant',fq_name('v',st),[value=ms_antuvw[2,st]]),
        meq.node('MeqConstant',fq_name('w',st),[value=ms_antuvw[3,st]])
      )) );
  }
  # finally, this node computes UVWs directly
  dmi.add_list(uvwlist,
    meq.node('MeqUVW',fq_name('uvw',st),children=[
                         x = meq.parm(fq_name('x',st),pos.x),
                         y = meq.parm(fq_name('y',st),pos.y),
                         z = meq.parm(fq_name('z',st),pos.z),
                         ra = 'ra0',dec = 'dec0',
                         x_0='x0',y_0='y0',z_0='z0' ]) );
                         
  uvw := meq.node('MeqReqSeq',fq_name('uvw.seq',st),[result_index=1],children=uvwlist);
    
  # builds an init-rec for a node called 'dft.N' with two children: 
  # lmn and uvw.N
  return meq.node('MeqStatPointSourceDFT',fq_name('dft',st),[link_or_create=T],children=[
              lmn = 'lmn',uvw = uvw ]);
}

# builds an init-record for a "dft" tree for two stations (st1,st2)
const ifr_predict_tree := function (st1,st2)
{
  return meq.node('MeqMultiply',fq_name('predict',st1,st2),children=meq.list(
      'stokes_i',
      meq.node('MeqPointSourceDFT',fq_name('dft',st1,st2),children=[
               st_dft_1 = sta_dft_tree(st1),
               st_dft_2 = sta_dft_tree(st2),
               n = 'n' ] ) ));
}

# creates nodes shared among trees: source parms, array center (x0,y0,z0)
const make_shared_nodes := function (stokesi=1,dra=0,ddec=0)
{
  global ms_phasedir;
  ra0  := ms_phasedir[1];  # phase center
  dec0 := ms_phasedir[2];
  # setup source parameters and subtrees
  ra   := ra0 + dra;
  dec  := dec0 + ddec;
  create_source_subtrees(stokesi,ra,dec,ra0,dec0);
  # setup zero position
  global ms_antpos;
  names := "x0 y0 z0";
  for( i in 1:3 )
    mqs.createnode(meq.node('MeqConstant',names[i],[value=ms_antpos[1][i]]));
}

# builds a predict tree for stations st1, st2
const make_predict_tree := function (st1,st2)
{
  sinkname := fq_name('sink',st1,st2);
  
  # create a sink
  mqs.createnode(meq.node('MeqSink',sinkname,
                         [ output_col      = 'PREDICT',   # init-rec for sink
                           station_1_index = st1,
                           station_2_index = st2,
                           corr_index      = [1] ],
                            children=meq.list(       # meq.list() builds a list
                            ifr_predict_tree(st1,st2))));
  
  return sinkname;
}

# builds a read-predict-subtract tree for stations st1, st2
const make_subtract_tree := function (st1,st2)
{
  sinkname := fq_name('sink',st1,st2);
  
  # create a sink & subtree attached to it
  # note how meq.node() can be passed a record in the third argument, to specify
  # other fields in the init-record
  mqs.createnode(
    meq.node('MeqSink',sinkname,
                         [ output_col      = 'PREDICT',
                           station_1_index = st1,
                           station_2_index = st2,
                           corr_index      = [1] ],
                         children=meq.list(
      meq.node('MeqSubtract',fq_name('sub',st1,st2),children=meq.list(
        meq.node('MeqSelector',fq_name('xx',st1,st2),[index=1],children=meq.list(
          meq.node('MeqSpigot',fq_name('spigot',st1,st2),[ 
            station_1_index=st1,
            station_2_index=st2,
            input_column='DATA'])
        )),
        ifr_predict_tree(st1,st2)
      ))
    ))
  );
  return sinkname;
}


# builds a solve tree for stations st1, st2
const make_solve_tree := function (st1,st2,subtract=F)
{
  sinkname := fq_name('sink',st1,st2);
  predtree := ifr_predict_tree(st1,st2);
  predname := predtree.name;
  mqs.createnode(predtree);
  
  # create condeq tree (solver will plug into this)
  mqs.createnode(
    meq.node('MeqCondeq',fq_name('ce',st1,st2),children=meq.list(
      predname,
      meq.node('MeqSelector',fq_name('xx',st1,st2),[index=1],children=meq.list(
        meq.node('MeqSpigot',fq_name('spigot',st1,st2),[ 
              station_1_index=st1,
              station_2_index=st2,
              input_column='DATA'])
      ))
    ))
  );
  # create subtract sub-tree
  if( subtract )
    mqs.createnode(meq.node('MeqSubtract',subname:=fq_name('sub',st1,st2),
                      children=[fq_name('xx',st1,st2),predname]));
  else
    subname := fq_name('spigot',st1,st2);
  
  
  # create root tree (plugs into solver & subtract)     
  mqs.createnode(
    meq.node('MeqSink',sinkname,[ output_col      = 'PREDICT',
                                  station_1_index = st1,
                                  station_2_index = st2,
                                  corr_index      = [1] ],children=meq.list(
      meq.node('MeqReqSeq',fq_name('seq',st1,st2),[result_index=2],
        children=['solver',subname])
   ))
 );

  return sinkname;
}

# reads antenna positions and phase center from MS,
# puts them into global variables
const get_ms_info := function (msname='test.ms',uvw=T)
{
  global ms_phasedir,ms_antpos;
  
  ms := table(msname);
  msant := table(ms.getkeyword('ANTENNA'));
  pos := msant.getcol('POSITION');
  num_ant := msant.nrows();
  msant.done();
  
  time0 := ms.getcell('TIME',1);
  
  # convert position to x y z
  ms_antpos := [=];
  for(i in 1:len(pos[1,]))
    ms_antpos[i] := [ x=pos[1,i],y=pos[2,i],z=pos[3,i] ];
    
  msfld := table(ms.getkeyword('FIELD'));
  ms_phasedir := msfld.getcol('PHASE_DIR');
  msfld.done();
  
  if( uvw )
  {
    global ms_antuvw;
    ms_antuvw := array(0.,3,num_ant);
    mss := ms.query('DATA_DESC_ID==0');
    # get UVW coordinates from ms
    ant1 := mss.getcol('ANTENNA1');
    ant2 := mss.getcol('ANTENNA2');
    uvw  := mss.getcol('UVW');
    mask1 := ant1 == 0;
    uvw0 := uvw[,mask1];
    ant2 := ant2[mask1];
    for( a2 in 2:num_ant )
    {
      t := uvw0[,ant2==(a2-1)];
      if( len(t) != 3 )
      {
        print 'cannot use UVWs from MS if more than one time slot';
        fail 'cannot use UVWs from MS if more than one time slot';
      }
      ms_antuvw[,a2] := t;
    }
    print 'Antenna UVWs:',ms_antuvw;
  }
  
  # get some UVWs, just for shits and giggles
  a0 := ms_antpos[1];
  pos0 := dm.position('itrf',dq.unit(a0.x,'m'),dq.unit(a0.y,'m'),dq.unit(a0.z,'m'));
  a1 := ms_antpos[1];
  a2 := ms_antpos[2];
  ba1 := dm.baseline('itrf',dq.unit(a2.x-a1.x,'m'),dq.unit(a2.y-a1.y,'m'),dq.unit(a2.z-a1.z,'m'));
  ba2 := dm.baseline('itrf',dq.unit(a1.x,'m'),dq.unit(a1.y,'m'),dq.unit(a1.z,'m'));
  dm.doframe(pos0);
  dm.doframe(dm.direction('j2000',dq.unit(ms_phasedir[1],"rad"),dq.unit(ms_phasedir[2],"rad")));
  dm.doframe(dm.epoch('utc',dq.unit(time0,'s')));
  local uvw1a;
  local dot;
  uvw1b := dm.touvw(ba1,dot,uvw1a);
  uvw1c := dm.addxvalue(uvw1b);
  uvw2b := dm.touvw(ba2,dot,uvw2a);
  uvw2c := dm.addxvalue(uvw2b);
  
  ms.done();  
  
  print 'Antenna position 1: ',ms_antpos[1];
  print 'Antenna position 2: ',ms_antpos[2];
  print 'Phase dir: ',ms_phasedir[1],' ',ms_phasedir[2];
  print 'UVW1a:',uvw1a;
  print 'UVW1b:',uvw1b;
  print 'UVW1c:',uvw1c;
  print 'UVW2a:',uvw2a;
  print 'UVW2b:',uvw2b;
  print 'UVW2c:',uvw2c;
  print 'Does this look sane?';
  
  return T;
}

use_initcol := T;       # initialize output column with zeroes

# predict=T:  predict tree only (writes predict to output column)
# subtract=T: predict+subtract trees (writes residual to output column)
# solve=T,subtract=T: solve+subtract trees (writes residual to output column)
# solve=T,subtract=F: solve but no subtract
#
# run=F: build trees and stop, run=T: run over the measurement set
const do_test := function (predict=F,subtract=F,solve=F,run=T,
    msname='test.ms',
    outcol='PREDICTED_DATA',        # output column of MS
    st1set=[1],st2set=[2,3,4],      # stations for which to make trees
    msuvw=F,                        # use UVW values from MS
    mepuvw=F,                       # use UVW from MEP table (should be filled already)
    load='',                        # load forest from file 
    save='',                        # save forest to file
    publish=3)    # node publish: higher means more detail
{
  # clear output column in MS
  # (I still can't get this to work from C++, hence this bit of glish here)
  if( use_initcol )
  {
    print 'Clearing',outcol,'column, please ignore error messages'
    tbl := table(msname,readonly=F);
    desc := tbl.getcoldesc(outcol);
    if( is_fail(desc) )
      desc := [=];
    # insert column anew, if no shape
    if( has_field(desc,'shape') )
      cellshape := desc.shape;
    else
    {
      if( len(desc) )
        tbl.removecols(outcol);
      cellshape := tbl.getcell('DATA',1)::shape;
      desc := tablecreatearraycoldesc(outcol,complex(0),2,shp);
      tbl.addcols(desc);
    }
    # insert zeroes
    tbl.putcol(outcol,array(complex(0),cellshape[1],cellshape[2],tbl.nrows()));
    tbl.done();
  }

  # read antenna positions, etc.  
  get_ms_info(msname,uvw=msuvw);
  
  # initialize meqserver (see mqsinit_test.g)
  if( is_fail(mqsinit()) )
  {
    print mqs;
    fail;
  }

  # load forest if asked
  if( load )
    mqs.meq('Load.Forest',[file_name=load])
  else # else build trees
  {  
    # create common nodes (source parms and such)
    dra :=  2 * pi/(180*60*60); # perturb position by # seconds
    ddec := 2 * pi/(180*60*60);
#    dra := ddec := 0;
    make_shared_nodes(1.1,dra,ddec);

    # make a solver node (since it's only one)
    if( solve )
    {
      condeqs := [];
      for( st1 in st1set )
        for( st2 in st2set )
          if( st1 < st2 )
            condeqs := [condeqs,fq_name('ce',st1,st2)];
      # note that child names will be resolved later
      rec := meq.node('MeqSolver','solver',[
          parm_group = hiid("a"),
          default    = [ num_iter=10,save_polcs=F,last_update=F ],
          solvable   = meq.solvable_list("stokes_i ra dec") ],
        children=condeqs);
      mqs.createnode(rec);
    }
    if( publish>0 )
    {
      mqs.meq('Node.Publish.Results',[name="ra"]);
      mqs.meq('Node.Publish.Results',[name="dec"]);
      mqs.meq('Node.Publish.Results',[name="stokes_i"]);
    }
    rootnodes := [];
    # make predict/condeq trees
    for( st1 in st1set )
      for( st2 in st2set )
        if( st1 < st2 )
        {
          if( solve )
          {
            rootnodes := [rootnodes,make_solve_tree(st1,st2,subtract=subtract)];
            if( publish>1 )
              mqs.meq('Node.Publish.Results',[name=fq_name('ce',st1,st2)]);
          }
          else if( subtract )
            rootnodes := [rootnodes,make_subtract_tree(st1,st2)];
          else
            rootnodes := [rootnodes,make_predict_tree(st1,st2)];
          if( publish>2 )
            mqs.meq('Node.Publish.Results',[name=fq_name('predict',st1,st2)]);
        }
    # resolve children on all root nodes
    print 'Root nodes are: ',rootnodes;
    for( r in rootnodes )
      mqs.resolve(r);
  }
  # save forest if requested
  if( save )
    mqs.meq('Save.Forest',[file_name=save]);
  
  # get a list of nodes
  nodelist := mqs.getnodelist(children=T);
  print 'Nodes: ',nodelist.name;
  
  # enable publishing of solver results
  if( solve && publish>0 )
    mqs.meq('Node.Publish.Results',[name='solver']);

  # run over MS
  if( run )
  {
    # activate input and watch the fur fly  
    global inputrec,outputrec;
    mqs.init(input=inputrec,output=outputrec); 
  }
}

msname := 'test.ms';
# msname := 'test-wsrt.ms';
mepuvw := T;
filluvw := F;

# fill UVW parms from MS if requested
if( mepuvw )
{
  include 'meq/msuvw_to_mep.g'
  mepuvw := msname ~ s/.ms/.mep/;
  if( filluvw )
    fill_uvw(msname,mepuvw);
}
else
  mepuvw := F;

inputrec := [ ms_name = msname,data_column_name = 'DATA',tile_size=5,
              selection = [ channel_start_index=1,channel_end_index=1 ] ];
outputrec := [ write_flags=F,predict_column=outcol ]; 

#do_test(predict=T,run=T,st1set=1,st2set=2,publish=2);
# do_test(solve=T,run=T,st1set=1,st2set=1,publish=2);
do_test(msname=msname,solve=T,subtract=F,run=T,
  st1set=1:2,st2set=1:100,
  publish=1,mepuvw=mepuvw,msuvw=msuvw);
#do_test(solve=T,run=T,publish=2,load='solve-100.forest');

print 'errors reported:',mqs.num_errors();

# mqs.halt();

# useful command-line arguments
# (use, e.g., glish -l meqsolve.g <arguments>)
#
# verbose=lev   verbosity level for Glish (2 is useful)
# debug=lev     restrict max debug level for C++ (use 0 to disable all debug 
#               messages)
# -d<Context>=level changes the debug message level for a specific context
# -gui          starts a small gui (at this stage, only useful for tracking messages)
# -nostart      does not automatically start a meqserver. Instead, prints the 
#               following message:
#               ===============================================
#               === Waiting for server to be manually started
#               === Please start it with the following command:
#               === ./meqserver  -d0 -nogw -meq:M:M:MeqServer
#               ===============================================
#               and waits for a server connection. This allows you to start 
#               a meqserver process manually under a debugger or valgrind.
#


#
# other useful glish functions for when the script is finished:
#
# mqs.num_errors()    checks for accumulated errors (should be 0)
# mqs.get_error_log() gets list of accumulated errors and clears the log
# 
# mqs.getnodelist()  get list of nodes
# mqs.getnodestate('name') get a node's state record (see esp. the request field
#   for the last request, and the cache_result field)
# mqs.execute('nodename',req)  sends a request to the node, returns result
#   see meqtest.g for examples of creating request objects
#
# recprint(rec) pretty-prints a record. Try e.g.:
#   recprint(mqs.getnodestate('solver'))






