# Script to take a subset of an MS and store it in a new MS.
# The DATA and UVW are stored with tiled storage managers such that their
# files can be used by the BBS software using file mapping.

include 'table.g'

mssplit := function (msin, selcommand, msout, deepcopy=T)
{
    t:=table (msin);
    if (is_fail(t)) fail;
    if (deepcopy) {
	t1 := t.query (selcommand, name=spaste(msout,'_tmp'));
    } else {
	t1 := t.query (selcommand, name=msout);
    }
    t.close();
    if (is_fail(t1)) fail;
    t1.close();
    if (deepcopy) {
	t1 := table (spaste(msout,'_tmp'), readonly=F);
	# Do not copy other DATA columns.
	cols := t1.colnames();
	cols := cols[cols ~ m/.*_DATA$/];
	if (len(cols) > 0) {
	    print 'Not copying columns',cols;
	    t1.removecols (cols);
	}
	# Create new dminfo for DATA and UVW column.
	nmd := -1;
	nmu := -1;
	dmo := t1.getdminfo();
	dm := dmo;
	for (i in [1:len(dm)]) {
	    cols := dm[i].COLUMNS;
	    colsn := cols[cols != 'DATA'];
	    if (len(colsn) == 0) {
		nmd := i;
	    } else {
		colsn := colsn[colsn != 'UVW'];
		if (len(colsn) == 0) {
		    nmu := i;
		}
	    }
	    if (len(colsn) > 0) {
		dm[i].COLUMNS := colsn;
	    }
	}
	if (nmd < 0) {
	    nmd := 1+len(dm);
	}
	dm[nmd] := [TYPE='TiledColumnStMan', NAME='TiledDataNew',
		    SPEC=[DEFAULTTILESHAPE=[4,100000,1],TILESHAPE=[4,100000,1],
			  MAXIMUMCACHESIZE=0], COLUMNS=['DATA']];
	if (nmu < 0) {
	    nmu := 1+len(dm);
	}
	dm[nmu] := [TYPE='TiledColumnStMan', NAME='TiledUVWNew',
		    SPEC=[DEFAULTTILESHAPE=[3,200], TILESHAPE=[3,200],
			  MAXIMUMCACHESIZE=0], COLUMNS=['UVW']];
	t1.copy (msout, T, T, dm, 'local');
	t1.close();
	# Delete the temporary table.
	tabledelete (spaste(msout,'_tmp'));
	t1 := table(msout);
	dm := t1.getdminfo();
	t1.close();
	for (i in [1:len(dm)]) {
	    if (dm[i].NAME == 'TiledDataNew') {
		print 'DATA stman =', i-1;
	    } else if (dm[i].NAME == 'TiledUVWNew') {
		print ' UVW stman =', i-1;
	    }
	}
    }   
}
