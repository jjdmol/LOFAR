# use_suspend  := T;
# use_gui := T;
# use_nostart  := T;
# use_valgrind := T;
 use_valgrind_opts := [ "",
#  "--gdb-attach=yes",          # use either this...
  "--logfile=vg.meqserver",       # ...or this, not both
# "--skin=helgrind --logfile=hg.meqserver";
#  "--gdb-path=/home/oms/bin/valddd", 
  ""];
  
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
  mqs.createnode(meq.parm('stokes_i',sti));
  mqs.createnode(meq.node('MeqLMN','lmn',children=[
                  ra_0  =meq.parm('ra0',ra0),
                  dec_0 =meq.parm('dec0',dec0),
                  ra    =meq.parm('ra',ra),
                  dec   =meq.parm('dec',dec)    ]));
  rec := meq.node('MeqSelector','n',children="lmn");
  rec.index := 3;
  mqs.createnode(rec);
}

# creates fully-qualified node name, by pasting suffixes after a dot
const fq_name := function (name,...) 
{
  return paste(name,...,sep='.');
}

const sta_dft_tree := function (st,x,y,z)
{
  # dft-sN node
  rec := meq.node('MeqStatPointSourceDFT',fq_name('dft',st),children=[
          lmn = 'lmn',
          uvw = meq.node('MeqUVW',fq_name('uvw',st),children=[
                           x = meq.parm(fq_name('x',st),x),
                           y = meq.parm(fq_name('y',st),y),
                           z = meq.parm(fq_name('z',st),z),
                           ra = 'ra0',dec = 'dec0',
                           x_0='x0',y_0='y0',z_0='z0' ]) ]);
  rec.link_or_create := T;                          
  return rec;
}

const ifr_predict_tree := function (st1,st2)
{
  global ms_antpos;
  pos1 := ms_antpos[st1];
  pos2 := ms_antpos[st2];
  rec := meq.node('MeqMultiply',fq_name('predict',st1,st2),children=[
      a = 'stokes_i',
      b = meq.node('MeqPointSourceDFT',fq_name('dft',st1,st2),children=[
            st_dft_1 = sta_dft_tree(st1,pos1.x,pos1.y,pos1.z),
            st_dft_2 = sta_dft_tree(st2,pos2.x,pos2.y,pos2.z),
            n = 'n' ] ) ]);
  return rec;
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
  {
    rec := meq.node('MeqConstant',names[i]);
    rec.value := ms_antpos[1][i];
    mqs.createnode(rec);
  }
}

const make_predict_tree := function (st1,st2,msname='test.ms')
{
  global ms_phasedir,ms_antpos;

  sinkname := fq_name('sink',st1,st2);
  
  # create predict sub-tree
  pred_tree := ifr_predict_tree(st1,st2);
  
  # create a sink
  sinkrec := meq.node('MeqSink',sinkname,children=[a=pred_tree]);
  sinkrec.output_col := 'PREDICT'; 
  sinkrec.station_1_index := st1;
  sinkrec.station_2_index := st2;
  sinkrec.corr_index := [1];      # get first correlation only
  print mqs.createnode(sinkrec);
  
  # resolve its children
  print mqs.meq('Resolve.Children',[name=sinkname],F);
  
  return sinkname;
}


const predict_test := function (msname='test.ms',
    outcol='PREDICTED_DATA',
    st1set=[1],st2set=[2,3,4],publish=T,
    verbose=default_verbosity,gui=use_gui)
{
  # remove output column from table
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
  
  get_ms_info(msname);
  
  if( is_fail(mqsinit(verbose=verbose,gui=gui)) )
  {
    print mqs;
    fail;
  }

  make_shared_nodes();
  
  for( st1 in st1set )
    for( st2 in st2set )
    {
      make_predict_tree(st1,st2,msname);
      if( publish )
        mqs.meq('Node.Publish.Results',[name=fq_name('predict',st1,st2)]);
    }
  
  nodelist := mqs.getnodelist();
  print 'Created nodes: ',nodelist.name;

  # activate input and watch the fur fly  
  global inputrec,outputrec;
  inputrec := [ ms_name = msname,data_column_name = 'DATA',tile_size=5,
                selection = [=]  ];
  outputrec := [ write_flags=F,predict_column=outcol ]; 
  mqs.init(input=inputrec,output=outputrec); 
}


predict_test();
