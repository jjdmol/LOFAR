# Script to take a subset of an MS and store it in a new MS.
# The DATA, FLAG and UVW are stored with tiled storage managers such that
# their files can be used by the BBS software using file mapping.

include 'table.g'

mssplit := function (msin, selcommand, msout, datacolumn='DATA', deepcopy=T)
{
    t:=table (msin);
    if (is_fail(t)) fail;
    if (deepcopy) {
	t1 := t.query (selcommand, sortlist='TIME,ANTENNA1,ANTENNA2',
		       name=spaste(msout,'_tmp'));
    } else {
	t1 := t.query (selcommand, sortlist='TIME,ANTENNA1,ANTENNA2',
		       name=msout);
    }
    t.close();
    if (is_fail(t1)) fail;
    nr := t1.nrows();
    t1.close();
    if (nr == 0) {
	fail paste('no rows selected from table', msin);
    }
    if (deepcopy) {
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
	# Write the description file.
	shell(paste('MSDesc',msout,'DATA'));
	# Create symlinks for the data, flags and uvw file.
	shell(spaste('ln -s ','table.f',nmd,'_TSM0 ',msout,'/vis.dat'));
	shell(spaste('ln -s ','table.f',nmf,'_TSM0 ',msout,'/vis.flg'));
	shell(spaste('ln -s ','table.f',nmu,'_TSM0 ',msout,'/vis.uvw'));
	# Make it readable for the world.
	shell(paste('chmod -R +r',msout));
    }   
}
