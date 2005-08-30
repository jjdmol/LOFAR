# Script to take a subset of an MS and store it in a new MS.
# The DATA, FLAG and UVW are stored with tiled storage managers such that
# their files can be used by the BBS software using file mapping.

include 'table.g'

msconv := function (t, msout, selcommand='', datacolumn='DATA',
		    makeresidual=F, usedouble=F)
{
    print 'Selecting subset',selcommand;
    t1 := t.query (selcommand, sortlist='TIME,ANTENNA1,ANTENNA2',
		   name=spaste(msout,'_tmp'));
    if (is_fail(t1)) fail;
    nr := t1.nrows();
    t1.close();
    if (nr == 0) {
	fail paste('no rows selected from table', msin);
    }
    t1 := table (spaste(msout,'_tmp'), readonly=F);
    # Check if data column is present.
    cols := t1.colnames();
    if (len(cols[cols == datacolumn]) == 0) {
	print 'Data column',datacolumn,'does not exist;',
	      'column DATA will be used instead';
	datacolumn := 'DATA';
    }
    if (len(cols[cols == datacolumn]) == 0) {
	fail paste('Data column',datacolumn,'does not exist');
    }
    # Get the shape of the flags.
    shp := shape(t1.getcell('FLAG',1));

    # Skip all all data columns and the FLAG column.
    colsn := cols[cols ~ m/.*DATA$/];
    colsn := colsn[colsn != datacolumn];
    print spaste('Copying the MS selection (',t1.nrows(),' rows) ...');
    if (len(colsn) > 0) {
	print ' not copying columns',colsn;
    }
    colsc := cols[cols !~ m/.*DATA$/];
    colsc := colsc[colsc != 'FLAG'];
    t1c := t1.query(columns=paste(colsc,sep=','));
    # Create new dminfo for UVW column.
    # (use TiledColumnStMan to get it into a separate file).
    nmu := -1;
    dmo := t1c.getdminfo();
    dm := dmo;
    for (i in [1:len(dm)]) {
	cols := dm[i].COLUMNS;
	cols := cols[cols != 'UVW'];
	if (len(cols) == 0) {
	    nmu := i;
	} else {
	    dm[i].COLUMNS := cols;
	}
    }
    # Store for UVW with a reasonable tile size (1200 bytes).
    if (nmu < 0) {
	nmu := 1+len(dm);
    }
    dm[nmu] := [TYPE='TiledColumnStMan', NAME='TiledUVWNew',
		SPEC=[DEFAULTTILESHAPE=[3,100], TILESHAPE=[3,100],
		      MAXIMUMCACHESIZE=0], COLUMNS=['UVW']];
    # Now copy the table and open the result.
    # Copy to local endian format.
    if (is_fail(t1c.copy (msout, T, T, dm, 'local'))) fail;
    t1c.close();
    t2 := table(msout, readonly=F);
    if (is_fail(t2)) fail;

    # The DATA and FLAG column still have to be copied.
    # First add columns for them.
    # Make them fixed shape arrays, so TiledColumnStMan can be used.
    # Store for DATA all pol,freq and 1 row per tile.
    print ' copying FLAG column and data column',datacolumn;
    dmd := [TYPE='TiledColumnStMan', NAME='TiledDataNew',
	    SPEC=[DEFAULTTILESHAPE=[4,100000,1],
		  TILESHAPE=[4,100000,1],
		  MAXIMUMCACHESIZE=0]];
    coldes := [name='DATA', desc=t1.getcoldesc (datacolumn)];
    coldes.desc.shape := shp;
    if (is_fail(t2.addcols (coldes, dmd))) fail;
    # Determine how many rows fill a tile of 1KBytes.
    # Use 8 times more rows, because the flags are stored as bits.
    # In this way all bits in all bytes in a tile are used and the
    # size of a tile is usually reasonable.
    nrrt := as_integer(1024 / (shp[1] * shp[2]));
    if (nrrt < 1) nrrt:=1;
    dmf := [TYPE='TiledColumnStMan', NAME='TiledFlagNew',
	    SPEC=[DEFAULTTILESHAPE=[4,100000,8*nrrt],
		  TILESHAPE=[4,100000,8*nrrt],
		  MAXIMUMCACHESIZE=0], COLUMNS=['FLAG']];
    coldes := [name='FLAG', desc=t1.getcoldesc ('FLAG')];
    coldes.desc.shape := shp;
    if (is_fail(t2.addcols (coldes, dmf))) fail;
    # Now copy the DATA and the FLAG column.
    # Do it by using copyrows which copies all rows for columns with
    # equal names in source and target table.
    tr1 := t1.query (columns=spaste(datacolumn,',FLAG'));
    if (is_fail(tr1)) fail;
    tr2 := t2.query (columns='DATA,FLAG');
    if (is_fail(tr2)) fail;
    if (datacolumn != 'DATA') {
	if (is_fail(tr1.renamecol (datacolumn, 'DATA'))) fail;
    }
    if (is_fail(tr1.copyrows (tr2, 1, 1, tr1.nrows()))) fail;
print 'copied'

    t1.close();
    t2.close();
    # Delete the temporary table.
    tabledelete (spaste(msout,'_tmp'));
    # Set all flags if a row flag is true.
    print 'Setting flags if FLAG_ROW is set ...';
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

    if (makeresidual) {
	# Add a column containing the residuals.
	# Use a tiled column storage manager for it.
	# If double complex has to be used, name it RESIDUAL_DATA and create
	# a mapping column CORRECTED_DATA.
	# Otherwise name it CORRECTED_DATA.
	dmres := [TYPE='TiledColumnStMan',
		  NAME='TiledDataRes',
		  SPEC=[DEFAULTTILESHAPE=[4,100000,1],
			TILESHAPE=[4,100000,1],
			MAXIMUMCACHESIZE=0]];
	coldes := [name='CORRECTED_DATA', desc=t1.getcoldesc ('DATA')];
	if (usedouble) {
	    coldes.name := 'RESIDUAL_DATA';
	    coldes.desc.valueType := 'dcomplex';
	}
	coldes.desc.comment := 'The residual data column';
	if (is_fail(t1.addcols (coldes, dmres))) fail;
	# Add a column CORRECTED_DATA mapping RESIDUAL_DATA to single precision.
	dmcor := [TYPE='MappedArrayEngine<Complex ,DComplex>',
		  NAME='CORRECTED_DATA',
		  SPEC=[SOURCENAME='CORRECTED_DATA',
			TARGETNAME='RESIDUAL_DATA']];
	if (usedouble) {
	    coldes := [name='CORRECTED_DATA', desc=t1.getcoldesc ('DATA')];
	    coldes.desc.comment := 'The corrected data column';
	    if (is_fail(t1.addcols (coldes, dmcor))) fail;
	}
	# Copy all data to CORRECTED_DATA (thus to RESIDUAL_DATA).
	# Do it by using copyrows which copies all rows for columns with
	# equal names in source and target table.
	# Make 2 tables with a single column and give them the same column name.
	print 'Copying DATA to RESIDUAL_DATA ...';
	tr1 := t1.query (columns='DATA');
	tr2 := t1.query (columns='CORRECTED_DATA');
	if (is_fail(tr2.renamecol ('CORRECTED_DATA', 'DATA'))) fail;
	if (is_fail(tr1.copyrows (tr2, 1, 1, tr1.nrows()))) fail;
    }

    # Get the storage manager numbers of the various data files.
    dm := t1.getdminfo();
    t1.close();
    for (i in [1:len(dm)]) {
	if (dm[i].NAME == 'TiledDataNew') {
	    print ' DATA stman =', i-1;
	    nmd := i-1;
	} else if (dm[i].NAME == 'TiledDataRes') {
	    print ' RESIDUAL_DATA stman =', i-1;
	    nmr := i-1;
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
    if (makeresidual) {
	shell(spaste('ln -s ','table.f',nmr,'_TSM0 ',msout,'/vis.res'));
    }
    # Create the description file.
    print 'Creating the description file vis.des ...';
    shell(paste('MSDesc',msout,'DATA'));
    # Set the protections such that the world can read the MS created.
    shell(paste('chmod -R +r',msout));
    return T;
}


mssplit := function (msin, nparts, datacolumn='DATA',
		     makeresidual=F, usedouble=F)
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
	fail 'Multiple subarrays or spectral windows cannot be handled yet';
    }
    # No split needed if only 1 part.
    if (nparts < 2) {
	res := msconv (t, spaste(msin,'_p1'), '', datacolumn,
		       makeresidual, usedouble);
	t.close();
	return res;
    }
    include 'trysplit.g'
    # Determine nr of baselines
    t1:=t.query(sortlist='unique ANTENNA1,ANTENNA2');
    if (is_fail(t1)) fail;
    nrbl := t1.nrows();
    ant1 := t1.getcol('ANTENNA1');
    ant2 := t1.getcol('ANTENNA2');
    t1.close();
    # Divide the baselines in nparts with a minimum nr of antennas per part.
    blmat := dosplit2 (1+ant1, 1+ant2, nparts);
    # Flatten the matrix to a vector (easier to select in).
    blvec := array (blmat, len(blmat));
    vecnrs := [0:(len(blvec)-1)];
    nra1 := shape(blmat)[1];
    # Create the MS subset for each part.
    tnrrow := 0;
    rows := [];
    for (part in [1:nparts]) {
	vecsel := vecnrs[blvec == part];
        vecsela1 := spaste(vecsel%nra1) ~ s/].*/]/ ~ s/ /,/g;
        vecsela2 := spaste(as_integer(vecsel/nra1)) ~ s/].*/]/ ~ s/ /,/g;
        if (is_fail(msconv (t, spaste(msin,'_p',part),
			    spaste('any(ANTENNA1 == ', vecsela1,
				   ' && ANTENNA2 == ', vecsela2, ')'),
			    datacolumn, makeresidual, usedouble))) fail;
        t1 := table (spaste(msin,'_p',part));
        tnrrow +:= t1.nrows();
        rows := [rows, t1.rownumbers(t)];
        t1.close();
    }
    if (tnrrow != t.nrows()) {
	print 'error:',tnrrow,'rows selected instead of',t.nrows();
	return F;
    }
    if (sort(rows) != [1:tnrrow]) {
	print 'error: selected rows are not disjoint';
	return F;
    }
    t.close();
    return T;
}
