// VisTile
#include "VisDM/VisTile.h"

// Class VisTile::ConstIterator 

//##ModelId=3DB964F701E6
VisTile::ConstIterator::ConstIterator()
  : ptile(0)
{
}

VisTile::ConstIterator::ConstIterator(const VisTile::ConstIterator &right)
{
  operator =(right);
}

//##ModelId=3DB964F70241
VisTile::ConstIterator::ConstIterator (const VisTile &tile)
{
  attach(tile);
}

//##ModelId=3DB964F70288
VisTile::ConstIterator::ConstIterator (const VisTileRef &ref)
{
  attach(ref);
}

//##ModelId=3DB964F702D0
VisTile::ConstIterator & VisTile::ConstIterator::operator=(const VisTile::ConstIterator &right)
{
  if( this != &right )
  {
    tilelock.release();
    ptile = right.ptile;
    if( ptile )
      tilelock.lock(ptile->mutex());
    if( right.tileref.valid() )
      tileref.copy(right.tileref,DMI::COPYREF|DMI::READONLY);
    ntime = right.ntime;
    itime = right.itime;
  }
  return *this;
}

VisTile::ConstIterator::~ConstIterator()
{
  // make sure lock is released before possibly releasing tile
  tilelock.release();
}

//##ModelId=3DB964F70322
void VisTile::ConstIterator::attach (const VisTile &tile)
{
  tileref.detach();
  ptile = const_cast<VisTile*>(&tile);
  tilelock.lock(ptile->mutex());
  ntime = ptile->ntime();
  itime = 0;
}

//##ModelId=3DB964F70368
void VisTile::ConstIterator::attach (const VisTileRef &ref)
{
  tileref.copy(ref,DMI::READONLY);
  ptile = const_cast<VisTile*>(tileref.deref_p()); 
  tilelock.lock(ptile->mutex());
  ntime = ptile->ntime();
  itime = 0;
}

// Additional Declarations
//##ModelId=3DB964F80132
void VisTile::ConstIterator::detach ()
{
  tilelock.release();
  ptile = 0;
  tileref.detach();
}


//##ModelId=3DB964F8019C
VisTile::Iterator::Iterator()
  : ConstIterator()
{
}

// Class VisTile::Iterator 

VisTile::Iterator::Iterator(const VisTile::Iterator &right)
  : ConstIterator()
{
  operator = (right);
}

//##ModelId=3DB964F80204
VisTile::Iterator::Iterator (VisTile &tile)
{
  attach(tile);
}

//##ModelId=3DB964F8025C
VisTile::Iterator::Iterator (const VisTileRef &ref)
{
  attach(ref);
}

//##ModelId=3DB964F8029F
VisTile::Iterator & VisTile::Iterator::operator=(const VisTile::Iterator &right)
{
  ConstIterator::operator=(right);
  return *this;
}

//##ModelId=3DB964F802EF
void VisTile::Iterator::attach (VisTile &tile)
{
  FailWhen( !tile.isWritable(),"tile not writable" );
  ConstIterator::attach(tile);
}

//##ModelId=3DB964F80334
void VisTile::Iterator::attach (const VisTileRef &ref)
{
  FailWhen( !ref.isWritable() || !ref->isWritable(),"tile not writable" );
  ConstIterator::attach(ref);
}

// Additional Declarations
//##ModelId=3DB964F900AF

// Class VisTile 

VisTile::VisTile()
  : ncorr_(0),nfreq_(0)
{
}

//##ModelId=3DB964F900B0
VisTile::VisTile (const VisTile &right, int flags,int depth)
    : ColumnarTableTile(right,flags,depth),
      ncorr_(right.ncorr_),nfreq_(right.nfreq_)
{
  if( hasFormat() )
    initArrays();
}

//##ModelId=3DB964F900BF
VisTile::VisTile (int nc, int nf, int nt)
{
  init(nc,nf,nt);
}

//##ModelId=3DB964F900D5
VisTile::VisTile (const FormatRef &form, int nt)
{
  init(form,nt);
}

//##ModelId=3DB964F900E4
VisTile::VisTile (const VisTile &a, const VisTile &b)
  : ColumnarTableTile(a,b),
    ncorr_(a.ncorr_),nfreq_(a.nfreq_)
{
  FailWhen( ncorr() != b.ncorr() || nfreq() != b.nfreq(),
      "cannot concatenate incompatible tiles");
  if( hasFormat() )
    initArrays();
}


//##ModelId=3DB964F900F6
VisTile::~VisTile()
{
}


//##ModelId=3DB964F900F7
VisTile & VisTile::operator=(const VisTile &right)
{
  if( this != &right )
  {
    Thread::Mutex::Lock lock(mutex());  
    ColumnarTableTile::operator=(right);
    nfreq_ = right.nfreq_;
    ncorr_ = right.ncorr_;
    if( hasFormat() )
      initArrays();
  }
  return *this;
}



//##ModelId=3DB964F90100
void VisTile::makeDefaultFormat (Format &form, int nc, int nf)
{
  LoShape shape(nc,nf);
  form.init(MAXCOL);
  form.add(DATA,Tpfcomplex,shape)
      .add(TIME,Tpdouble)
      .add(WEIGHT,Tpfloat)
      .add(UVW,Tpdouble,LoShape(3))
      .add(FLAGS,Tpint,shape)
      .add(ROWFLAG,Tpint);
}

//##ModelId=3DB964F90117
void VisTile::init (int nc, int nf, int nt)
{
  Thread::Mutex::Lock lock(mutex());  
  FormatRef ref(new Format,DMI::ANONWR);
  makeDefaultFormat(ref.dewr(),nc,nf);
  init(ref,nt);
}

//##ModelId=3DB964F9012E
void VisTile::init (const FormatRef &form, int nt)
{
  Thread::Mutex::Lock lock(mutex());  
  LoShape shape = form->shape(DATA);
  FailWhen( shape.size() != 2 ,"Missing or misshapen DATA column in tile format");
  ColumnarTableTile::init(form,nt);
  ncorr_ = shape[0];
  nfreq_ = shape[1];
  dprintf(2)("initialized tile: %s\n",sdebug(5).c_str());
  initArrays();
}

//##ModelId=3DB964F9013E
void VisTile::reset ()
{
  Thread::Mutex::Lock lock(mutex());  
  ColumnarTableTile::reset();
  ncorr_ = nfreq_ = 0;
}

//##ModelId=3DB964F9013F
void VisTile::applyFormat (const FormatRef &form)
{
  Thread::Mutex::Lock lock(mutex());  
  ColumnarTableTile::applyFormat(form);
  initArrays();
}

//##ModelId=3DB964F90147
void VisTile::changeFormat (const FormatRef &form)
{
  Thread::Mutex::Lock lock(mutex());  
  ColumnarTableTile::changeFormat(form);
  initArrays();
}

//##ModelId=3DB964F9014F
void VisTile::copy (int it0, const VisTile &other, int other_it0, int nt)
{
  Thread::Mutex::Lock lock(mutex());  
  // did we already have a format
  bool had_format = hasFormat();
  ColumnarTableTile::copy(it0,other,other_it0,nt);
  // if a format got set up for us, reinitialize the arrays
  if( !had_format )
  {
    // since the copy succeeded, we're now guaranteed to have a format
    LoShape shape = format().shape(DATA);
    // this shouldn't happen, but just in case
    FailWhen( shape.size() != 2 ,"Missing or misshapen DATA column in tile format");
    ncorr_ = shape[0];
    nfreq_ = shape[1];
    initArrays();
  }
}

//##ModelId=3DB964F90184
void VisTile::addRows (int nr)
{
  Thread::Mutex::Lock lock(mutex());  
  ColumnarTableTile::addRows(nr);
  // if this succeeded, then we have a format
  initArrays();
}

//##ModelId=3DB964F901CD
int VisTile::fromBlock (BlockSet& set)
{
  Thread::Mutex::Lock lock(mutex());  
  int ret = ColumnarTableTile::fromBlock(set);
  if( hasFormat() )
    initArrays();
  return ret;
}

//##ModelId=3DB964F901D6
CountedRefTarget* VisTile::clone (int flags, int depth) const
{
  return new VisTile(*this,flags,depth);
}

// Additional Declarations
//##ModelId=3DB964F901F6
void VisTile::initArrays ()
{
#if defined(LORRAYS_USE_BLITZ)
  initArrays_Blitz();
#elif defined(LORRAYS_USE_AIPSPP)
  initArrays_AIPSPP();
#endif
}

void VisTile::initArrays_Blitz ()
{
#if defined(LORRAYS_USE_BLITZ)
  LoCubeShape cubeshape(ncorr(),nfreq(),ntime());
  datacube.reference( LoCube_fcomplex(static_cast<fcomplex*>(wcolumn(DATA)),cubeshape,blitz::neverDeleteData) );
  flagcube.reference( LoCube_int(static_cast<int*>(wcolumn(FLAGS)),cubeshape,blitz::neverDeleteData) );
  
  LoVecShape vecshape(ntime());
  timevec.reference( LoVec_double(static_cast<double*>(wcolumn(TIME)),vecshape,blitz::neverDeleteData) );
  weightvec.reference( LoVec_float(static_cast<float*>(wcolumn(WEIGHT)),vecshape,blitz::neverDeleteData) );
  rowflagvec.reference( LoVec_int(static_cast<int*>(wcolumn(ROWFLAG)),vecshape,blitz::neverDeleteData) );
  
  uvwmatrix.reference( LoMat_double(static_cast<double*>(wcolumn(UVW)),LoMatShape(3,ntime()),blitz::neverDeleteData) );
  // additional columns go here
#endif
}

void VisTile::initArrays_AIPSPP ()
{
#if defined(LORRAYS_USE_AIPSPP)
  // this initializes internal arrays to represent columns
  // note the SHARE argument point the arrays at the tile data itself
  IPosition cubeshape(3,ncorr(),nfreq(),ntime());
  datacube.takeStorage(cubeshape,static_cast<fcomplex*>(wcolumn(DATA)),SHARE); 
  flagcube.takeStorage(cubeshape,static_cast<int*>(wcolumn(FLAGS)),SHARE);
  
  IPosition vecshape(1,ntime());
  timevec.takeStorage(vecshape,static_cast<double*>(wcolumn(TIME)),SHARE);
  weightvec.takeStorage(vecshape,static_cast<float*>(wcolumn(WEIGHT)),SHARE);
  rowflagvec.takeStorage(vecshape,static_cast<int*>(wcolumn(ROWFLAG)),SHARE);
  
  uvwmatrix.takeStorage(IPosition(2,3,ntime()),static_cast<double*>(wcolumn(UVW)),SHARE);
  // additional columns go here
#endif
}

//##ModelId=3DD3C6CB02E9
string VisTile::sdebug ( int detail,const string &prefix,const char *name ) const
{
  return ColumnarTableTile::sdebug(detail,prefix,name?name:"VisTile");
}
    

//##ModelId=3DD3CB0003D0
string VisTile::ConstIterator::sdebug ( int detail,const string &prefix,const char *name ) const
{
  using Debug::append;
  using Debug::appendf;
  using Debug::ssprintf;
  
  string out;
  if( detail >= 0 ) // basic detail
  {
    appendf(out,"%s/%08x",name?name:"CI:VisTile",(int)this);
  }
  if( detail >= 1 || detail == -1 )
  {
    if( ptile )
      appendf(out,"@ts %d/%d of %s",itime,ntime,
          ptile->sdebug(abs(detail),prefix).c_str());
    else
      append(out,"no tile");
    if( ptile && abs(detail) == 1 )
      append(out,tileref.valid() ? "(ref)" : "(no ref)" );
  }
  if( detail >= 2 || detail <= -2 )
  {
    if( tileref.valid() )
      append(out,"("+tileref.sdebug(1,"","VisTile::Ref")+")");
    else
      append(out,"(no ref)");
  }
  return out;
}

