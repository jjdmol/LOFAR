# Script to store station positions to an AIPS++ ANTENNA table
# with the positions in ITRF (m,m,m).

include 'measures.g'
include 'quanta.g'
include 'table.g'

doit := function()
{
    t:=table('LOFAR_ANTENNA',readonly=F);
    filec:=open('< lofar_stations.coord');
    read(filec);     #skip header
    nrl:=0;
    while (line := read(filec)) {
	nrl+:=1;
	vals := split(line,';');
	lat  := as_double(vals[2]);
	long := as_double(vals[3]);
	pos := dm.position('wgs84', dq.unit(long,'deg'),
			   dq.unit(lat,'deg'), dq.unit(0.,'m'));
	name := spaste('S', vals[1] ~ s/["]//g);    #"; add quote for emacs
	# Convert to ITRF and to units m,m,m.
        posi := dm.addxvalue (dm.measure (pos, 'itrf'));
	posvec := [posi[1].value, posi[2].value, posi[3].value];
	print name, posvec;
	t.putcell('NAME', nrl, name);
	t.putcell('POSITION', nrl, posvec);
	units := [posi[1].unit, posi[2].unit, posi[3].unit];
    }
    print nrl;
    t.putcolkeyword ('POSITION', 'QuantumUnits', units);
    t.close();
}
