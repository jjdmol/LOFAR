include 'table.g'
include '../src/gsm_definitions.g'

const tablename := 'gsm.tab';
const filename := '4C-Vizier.txt';

# open 4C source file
infile := open(['<',filename]);
# read sources from 4C file
started := F;
ra := [];
dec := [];
flux := [];
id_alt := "";
comment := "";
n := 0;
while( line := read(infile) ) 
{
  # skip header
  if( line =~ m/^---/ )
  {
    started := T;
    next;
  }
  if( !started )
    next;
  # parse the line
  f := split(line,'\t');
  if( len(f) < 9 )
    next;
  n +:= 1;
  # ID - use 4C number
  id[n] := spaste('4C',f[3]) ~s/(^ *| *$)//g;
  # parse position
  fra := split(f[1]);
  fdec := split(f[2]);  
  ra[n]  := (as_double(fra[1]) + as_double(fra[2])/60 + as_double(fra[3])/3600)*(2*pi/24);
  dec[n] := (as_double(fdec[1]) + as_double(fdec[2])/60)*(pi/180);
  # I flux
  flux[n] := as_double(f[4]);
  # do we have a comment column?
  comment[n] := '';
  id_alt[n] := id[n];
  if( len(split(f[9])) )
  {
    # if comment is a 3C identifier, use that for main ID, and put the 4C
    # number in the alt_id column
    if( f[9] =~ m/3C[0-9]+/ )
    {
      id3c := f[9];
      id3c =~ s/[^C0-9]//g;
      id_alt[n] := paste(id3c,id[n]);
      id[n] := id3c;
    }
    else
      comment[n] := f[9];
  }
}

print 'read ',n,' 4C sources';

# Create GSM table
tbl := GSM.inittable(tablename,nrows=n,zero=T);
if( is_fail(tbl) )
  fail tbl;

# put in data from 4C catalogue
tbl.putcol('ID',id);
tbl.putcol('RA',ra);
tbl.putcol('DEC',dec);
tbl.putcol('I0',flux);
tbl.putcol('ID_ALTERNATIVE',id_alt);
tbl.putcol('COMMENTS',comment);
# ref frequency for 4C sources is 178 MHz
tbl.putcol('FREQ0',array(178.0,n));

tbl.flush();
tbl.done();

exit
