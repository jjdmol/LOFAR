include 'mqsinit_test.g'
include 'table.g'
include 'measures.g'
include 'quanta.g'


# creates all source-related nodes and subtrees:
#   'stokesI':          flux
#   'ra' 'dec':         source position
#   'ra0' 'dec0':       phase center
#   'lmn','n':          LMN coordinates
const create_source_subtrees := function (sti,ra,dec,ra0,dec0)
{
  mqs.createnode(meq.parm('stokes_i',sti,groups=hiid("a")));
  mqs.createnode(meq.node('MeqLMN','lmn',children=[
                  ra_0  =meq.parm('ra0',ra0),
                  dec_0 =meq.parm('dec0',dec0),
                  ra    =meq.parm('ra',ra,groups=hiid("a")),
                  dec   =meq.parm('dec',dec,groups=hiid("a")) ]));
  mqs.createnode(meq.node('MeqSelector','n',[index=3],children="lmn"));
}

# creates fully-qualified node name, by pasting suffixes after a dot
const fq_name := function (name,...) 
{
  return paste(name,...,sep='.');
}

const sta_dft_tree := function (st,x,y,z)
{
  # dft-sN node
  return meq.node('MeqStatPointSourceDFT',fq_name('dft',st),[link_or_create=T],
           children=[
              lmn = 'lmn',
              uvw = meq.node('MeqUVW',fq_name('uvw',st),children=[
                               x = meq.parm(fq_name('x',st),x),
                               y = meq.parm(fq_name('y',st),y),
                               z = meq.parm(fq_name('z',st),z),
                               ra = 'ra0',dec = 'dec0',
                               x_0='x0',y_0='y0',z_0='z0' ]) ]);
}

const ifr_predict_tree := function (st1,st2)
{
  global ms_antpos;
  pos1 := ms_antpos[st1];
  pos2 := ms_antpos[st2];
  return meq.node('MeqMultiply',fq_name('predict',st1,st2),children=meq.list(
      'stokes_i',
      meq.node('MeqPointSourceDFT',fq_name('dft',st1,st2),children=[
               st_dft_1 = sta_dft_tree(st1,pos1.x,pos1.y,pos1.z),
               st_dft_2 = sta_dft_tree(st2,pos2.x,pos2.y,pos2.z),
               n = 'n' ] ) ));
}


const make_shared_nodes := function (sti=1,dra=0,ddec=0)
{
  global ms_phasedir;
  ra0  := ms_phasedir[1];  # phase center
  dec0 := ms_phasedir[2];
  # setup source parameters and subtrees
  sti  := 1;
  ra   := ra0 + dra;
  dec  := dec0 + ddec;
  create_source_subtrees(sti,ra,dec,ra0,dec0);
  # setup zero position
  global ms_antpos;
  names := "x0 y0 z0";
  for( i in 1:3 )
    mqs.createnode(meq.node('MeqConstant',names[i],[value=ms_antpos[1][i]]));
}

const make_predict_tree := function (st1,st2,msname='test.ms')
{
  global ms_phasedir,ms_antpos;

  sinkname := fq_name('sink',st1,st2);
  
  # create predict sub-tree
  pred_tree := ifr_predict_tree(st1,st2);
  
  # create a sink
  mqs.createnode(meq.node('MeqSink',sinkname,
                         [ output_col      = 'PREDICT',
                           station_1_index = st1,
                           station_2_index = st2,
                           corr_index      = [1] ],
                         children=meq.list(pred_tree)));
  
  return sinkname;
}

const make_subtract_tree := function (st1,st2,msname='test.ms')
{
  global ms_phasedir,ms_antpos;

  sinkname := fq_name('sink',st1,st2);
  spigname := fq_name('spigot',st1,st2);
  mqs.createnode(meq.node('MeqSpigot',spigname,[ 
            station_1_index=st1,
            station_2_index=st2,
            input_column='DATA']));
  
  
  # create a sink
  mqs.createnode(
    meq.node('MeqSink',sinkname,
                         [ output_col      = 'PREDICT',
                           station_1_index = st1,
                           station_2_index = st2,
                           corr_index      = [1] ],
                         children=meq.list(
      meq.node('MeqSubtract',fq_name('sub',st1,st2),children=meq.list(
        meq.node('MeqSelector',fq_name('xx',st1,st2),[index=1],children=spigname),
        ifr_predict_tree(st1,st2)
      ))
    ))
  );
  return sinkname;
}


const make_solve_tree := function (st1,st2,msname='test.ms')
{
  global ms_phasedir,ms_antpos;

  sinkname := fq_name('sink',st1,st2);
  predtree := ifr_predict_tree(st1,st2);
  predname := predtree.name;
  mqs.createnode(predtree);
  spigname := fq_name('spigot',st1,st2);
  mqs.createnode(meq.node('MeqSpigot',spigname,[ 
            station_1_index=st1,
            station_2_index=st2,
            input_column='DATA']));
  
  # create condeq tree (solver will plug into this)
  mqs.createnode(
    meq.node('MeqCondeq',fq_name('ce',st1,st2),children=meq.list(
      predname,
      meq.node('MeqSelector',fq_name('xx',st1,st2),[index=1],children=spigname)
    ))
  );
  # create subtract sub-tree
  mqs.createnode(meq.node('MeqSubtract',fq_name('sub',st1,st2),
                    children=[fq_name('xx',st1,st2),predname]));
  
  
  # create root tree (plugs into solver & subtract)     
  mqs.createnode(
    meq.node('MeqSink',sinkname,[ output_col      = 'PREDICT',
                                  station_1_index = st1,
                                  station_2_index = st2,
                                  corr_index      = [1] ],children=meq.list(
      meq.node('MeqReqSeq',fq_name('seq',st1,st2),[result_index=2],
        children=['solver',fq_name('sub',st1,st2)])
   ))
 );

  return sinkname;
}


const get_ms_info := function (msname='test.ms')
{
  global ms_phasedir,ms_antpos;
  
  ms := table(msname);
  msant := table(ms.getkeyword('ANTENNA'));
  pos := msant.getcol('POSITION');
  msant.done();
  
  # convert position to x y z
  ms_antpos := [=];
  for(i in 1:len(pos[1,]))
  {
    height := dq.quantity(pos[1,i],'m');
    lon := dq.quantity(pos[2,i],'rad');
    lat := dq.quantity(pos[3,i],'rad');
    xyz := dm.addxvalue(dm.position('itrf',lon,lat,height));
    ms_antpos[i] := [ x=xyz[1].value, y=xyz[2].value, z=xyz[3].value ];
  }
  
  msfld := table(ms.getkeyword('FIELD'));
  ms_phasedir := msfld.getcol('PHASE_DIR');
  msfld.done();
  
  ms.done();  
  
  print 'Antenna position 1: ',ms_antpos[1];
  print 'Antenna position 2: ',ms_antpos[2];
  print 'Phase dir: ',ms_phasedir[1],' ',ms_phasedir[2];
  print 'Does this look sane?';
  
  return T;
}

use_initcol := T;       # initialize output column with zeroes

# subtract=T: subtract tree only
# solve=T:    solve+subtract trees
#   both false: predict tree only
#
# run=F: build trees only, run=T: run sinks
const do_test := function (predict=F,subtract=F,solve=F,run=T,
    msname='test.ms',
    outcol='PREDICTED_DATA',
    st1set=[1],st2set=[2,3,4],
    load='',save='',
    publish=3)
{
  # remove output column from table
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
  
  get_ms_info(msname);
  
  if( is_fail(mqsinit()) )
  {
    print mqs;
    fail;
  }

  if( load )
    mqs.meq('Load.Forest',[file_name=load])
  else
  {  
    # create common nodes (source parms and such)
    make_shared_nodes();

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
          parm_group = hiid('a'),
          default    = [ num_iter = 3 ],
          solvable   = meq.solvable_list("stokes_i") ],
        children=condeqs);
      mqs.createnode(rec);
    }

    rootnodes := [];
    # make predict/condeq trees
    for( st1 in st1set )
      for( st2 in st2set )
        if( st1 < st2 )
        {
          if( solve )
          {
            rootnodes := [rootnodes,make_solve_tree(st1,st2,msname)];
            if( publish>1 )
              mqs.meq('Node.Publish.Results',[name=fq_name('ce',st1,st2)]);
          }
          else if( subtract )
            rootnodes := [rootnodes,make_subtract_tree(st1,st2,msname)];
          else
            rootnodes := [rootnodes,make_predict_tree(st1,st2,msname)];
          if( publish>2 )
            mqs.meq('Node.Publish.Results',[name=fq_name('predict',st1,st2)]);
        }
    # resolve children on all root nodes
    print 'Root nodes are: ',rootnodes;
    for( r in rootnodes )
      mqs.meq('Resolve.Children',[name=r]);
  }
  if( save )
    mqs.meq('Save.Forest',[file_name=save]);
  
  nodelist := mqs.getnodelist();
  print 'Nodes: ',nodelist.name;
  
  if( solve && publish>0 )
    mqs.meq('Node.Publish.Results',[name='solver']);

  if( run )
  {
    # activate input and watch the fur fly  
    global inputrec,outputrec;
    inputrec := [ ms_name = msname,data_column_name = 'DATA',tile_size=5,
                  selection = [=]  ];
    outputrec := [ write_flags=F,predict_column=outcol ]; 
    mqs.init(input=inputrec,output=outputrec); 
  }
}


# do_test(solve=T,run=T,st1set=1:10,st2set=1:10,publish=1,save='solve.forest');
do_test(solve=T,run=T,publish=1,load='solve.forest');

print 'errors reported:',mqs.num_errors();
