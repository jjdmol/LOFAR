# Script to take a subset of an MS and store it in a new MS.
# The DATA, FLAG and UVW are stored with tiled storage managers such that
# their files can be used by the BBS software using file mapping.

include 'table.g'

mssplit := function (msin, msout, datacolumn, nparts, machs, remdir)
{
    t:=table (msin);
    if (is_fail(t)) fail;
    # Determine nr of spectral windows, subarrays, etc.
    t1:=t.query(sortlist='unique DATA_DESC_ID, ARRAY_ID');
    if (is_fail(t1)) fail;
    nr := t1.nrows();
    arrids := t1.getcol ('ARRAY_ID');
    ddids  := t1.getcol ('DATA_DESC_ID');
    t1.close();
    if (nr == 0) {
	fail paste("No rows in MS",msin);
    }
    if (nr > nparts) {
	fail paste('Nr of subarrays and spectral windows (=', nr,
		   ') exceeds nparts =(', nparts, ')');
    }
    if (nr > 1) {
	fail 'Multiple subarray por spectral windows cannot be handled yet';
    }
    # Determine nr of baselines
    t1:=t.query(sortlist='unique ANTENNA1 ANTENNA2');
    if (is_fail(t1)) fail;
    nrbl := t1.nrows();
    ant1 := t1.getcol('ANTENNA1');
    ant2 := t1.getcol('ANTENNA2');
    t1.close();
    # Fill matrix telling which baselines are available.
    #  0 = not available
    #  1 = available
    # -1 = available mirrored (thus 1 in opposite cell)
    nrant := max(max(ant1), max(ant2));
    blmat := array(0, nrant, nrant);
    for (i in [1:len(ant1)]) {
	blmat[ant1[i],ant2[i]] := 1;     # real baseline
	blmat[ant2[i],ant1[i]] := -1;    # mirrored baseline
    }
    # Determine nr of baselines per part (evenly divided).
    nrblpart := as_integer(nrbl / nparts);
    nfpart := nrbl - nrblpart*nparts;         # nr of full parts
    # Loop through all baselines and try to group them such that each group
    # contains the fewest nr of antennas.
    # First find the width of a square containing antennas giving a group.
    width := 1;
    a1 := [nrant];
    a2 := [1];
    while (sum(abs(blmat[a1,a2])) < nrpbl) {
	a1 := [nrant-width:nrant];
	width +:= 1;
	a2 := [1:width];
    }
    nrpbl := nrblpart;
    if (partnr <= nfpart) nrpbl +:= 1;
	    
    # Start with the first group.
    inx1 := nrant;
    inx2 := 1;
    grpnr := 1;
    nrpbl := 0;
    while (nrpbl < nrblpart) {
	
    
    # Create as many parts as needed.
    for (seqnr in [1:nparts]) {
    t1 := t.query (selcommand, sortlist='TIME,ANTENNA1,ANTENNA2',
		   name=spaste(msout,'_tmp'));
    t.close();
    if (is_fail(t1)) fail;
    nr := t1.nrows();
    t1.close();
    if (nr == 0) {
	fail paste('no rows selected from table', msin);
    }
	t1 := table (spaste(msout,'_tmp'), readonly=F);
	# Check if data column is present.
	cols := t1.colnames();
	if (len(cols == datacolumn) == 0) {
	    print 'Data column',datacolumn,'does not exist;',
		'column DATA will be used instead';
	    datacolumn := 'DATA';
	}
	if (len(cols == datacolumn) == 0) {
	    fail paste('Data column',datacolumn,'does not exist');
	}
	# Get the shape of the flags.
	shp := shape(t1.getcell('FLAG',1));
	# Skip all other data columns.
	cols := cols[cols ~ m/.*DATA$/];
	cols := cols[cols != datacolumn];
	print spaste('Copying the MS selection (',t1.nrows(),' rows) ...');
	print ' copying data column',datacolumn
	if (len(cols) > 0) {
	    print ' not copying columns',cols;
	    t1.removecols (cols);
	}
	# Create new dminfo for DATA, FLAG and UVW column.
	nmd := -1;
	nmf := -1;
	nmu := -1;
	dmo := t1.getdminfo();
	dm := dmo;
	for (i in [1:len(dm)]) {
	    cols := dm[i].COLUMNS;
	    colsn := cols[cols != datacolumn];
	    if (len(colsn) == 0) {
		nmd := i;
	    } else {
		colsn := colsn[colsn != 'FLAG'];
		if (len(colsn) == 0) {
		    nmf := i;
		} else {
		    colsn := colsn[colsn != 'UVW'];
		    if (len(colsn) == 0) {
			nmu := i;
		    }
		}
	    }
	    if (len(colsn) > 0) {
		dm[i].COLUMNS := colsn;
	    }
	}
	# Store for DATA all pol,freq and 1 row per tile.
	if (nmd < 0) {
	    nmd := 1+len(dm);
	}
	dm[nmd] := [TYPE='TiledColumnStMan', NAME='TiledDataNew',
		    SPEC=[DEFAULTTILESHAPE=[4,100000,1],
			  TILESHAPE=[4,100000,1],
			  MAXIMUMCACHESIZE=0], COLUMNS=[datacolumn]];
	# Determine how many rows fill a tile of 1KBytes.
	# Use 8 times more rows, because the flags are stored as bits.
	# In this way all bits in all bytes in a tile are used and the
	# size of a tile is usually reasonable.
	if (nmf < 0) {
	    nmf := 1+len(dm);
	}
	nrrt := as_integer(1024 / (shp[1] * shp[2]));
	if (nrrt < 1) nrrt:=1;
	dm[nmf] := [TYPE='TiledColumnStMan', NAME='TiledFlagNew',
		    SPEC=[DEFAULTTILESHAPE=[4,100000,8*nrrt],
			  TILESHAPE=[4,100000,8*nrrt],
			  MAXIMUMCACHESIZE=0], COLUMNS=['FLAG']];
	# Store for UVW with a reasonable tile size (1200 bytes).
	if (nmu < 0) {
	    nmu := 1+len(dm);
	}
	dm[nmu] := [TYPE='TiledColumnStMan', NAME='TiledUVWNew',
		    SPEC=[DEFAULTTILESHAPE=[3,100], TILESHAPE=[3,100],
			  MAXIMUMCACHESIZE=0], COLUMNS=['UVW']];
	t1.copy (msout, T, T, dm, 'local');
	t1.close();
	# Delete the temporary table.
	tabledelete (spaste(msout,'_tmp'));
	# Set all flags if a row flag is true.
	print 'Setting flags if FLAG_ROW is set ...'
	t1 := tablecommand (paste('UPDATE',msout,'SET FLAG=T WHERE FLAG_ROW'));
	if (is_fail(t1)) fail;
	print 'Flags in',t1.nrows(),'rows have been set';
	t1.close();
	# Open the new table for update.
	t1 := table(msout, readonly=F);
	# Remove the SORT keywords (if existing).
	keys := t1.keywordnames();
	keyss := keys[keys=='SORT_COLUMNS'];
	if (len(keyys) > 0) {
	    t1.removekeyword ('SORT_COLUMNS');
	}
	keyss := keys[keys=='SORTED_TABLE'];
	if (len(keyys) > 0) {
	    snm := t1.getkeyword('SORTED_TABLE');
	    if (tableexists(snm)) {
		tabledelete(snm, F);
	    }
	    t1.removekeyword ('SORTED_TABLE');
	}
	# Rename the selected column to DATA.
	if (datacolumn != 'DATA') {
	    t1.renamecol (datacolumn, 'DATA');
	    print 'Renamed column',datacolumn,'to DATA';
	}
	dm := t1.getdminfo();
	t1.close();
	for (i in [1:len(dm)]) {
	    if (dm[i].NAME == 'TiledDataNew') {
		print ' DATA stman =', i-1;
		nmd := i-1;
	    } else if (dm[i].NAME == 'TiledFlagNew') {
		print ' FLAG stman =', i-1;
		nmf := i-1;
	    } else if (dm[i].NAME == 'TiledUVWNew') {
		print ' UVW stman =', i-1;
		nmu := i-1;
	    }
	}
	# Create symlinks for the data, flags and uvw file.
	shell(spaste('ln -s ','table.f',nmd,'_TSM0 ',msout,'/vis.dat'));
	shell(spaste('ln -s ','table.f',nmf,'_TSM0 ',msout,'/vis.flg'));
	shell(spaste('ln -s ','table.f',nmu,'_TSM0 ',msout,'/vis.uvw'));
    }   
}
