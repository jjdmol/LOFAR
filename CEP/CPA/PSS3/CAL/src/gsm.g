### TFStoredParmPolc.cc: Stored parameter with polynomial coefficients
###
### Copyright (C) 2002
### ASTRON (Netherlands Foundation for Research in Astronomy)
### P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
###
### This program is free software; you can redistribute it and/or modify
### it under the terms of the GNU General Public License as published by
### the Free Software Foundation; either version 2 of the License, or
### (at your option) any later version.
###
### This program is distributed in the hope that it will be useful,
### but WITHOUT ANY WARRANTY; without even the implied warranty of
### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
### GNU General Public License for more details.
###
### You should have received a copy of the GNU General Public License
### along with this program; if not, write to the Free Software
### Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
###
### $Id$

pragma include once

include 'table.g'

gsm := function (name, create=F)
{
    self:=[=];
    public:=[=];

    if (create) {
	# Create the main table.
	d1 := tablecreatescalarcoldesc ('NUMBER', as_integer(0));
	d2 := tablecreatescalarcoldesc ('NAME', '');
	d3 := tablecreatescalarcoldesc ('TYPE', as_integer(0));
	d4 := tablecreatearraycoldesc  ('RAPARMS', as_double(0));
	d5 := tablecreatearraycoldesc  ('DECPARMS', as_double(0));
	d6 := tablecreatearraycoldesc  ('TDOMAIN', as_double(0),
					shape=2, options=1);
	d7 := tablecreatearraycoldesc  ('FDOMAIN', as_double(0),
					shape=2, options=1);
	d8 := tablecreatearraycoldesc  ('IPARMS', as_double(0));
	d9 := tablecreatearraycoldesc  ('QPARMS', as_double(0));
	d10:= tablecreatearraycoldesc  ('UPARMS', as_double(0));
	d11:= tablecreatearraycoldesc  ('VPARMS', as_double(0));
	td := tablecreatedesc (d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11);
	self.tab := table (name, td);
	if (is_fail(self.tab)) {
	    fail;
	}
	info := self.tab.info();
	info.type := 'GSM';
	self.tab.putinfo (info);
	self.tab.addreadmeline ('PSS Global Sky Model (Point sources only)');
    } else {
	self.tab := table (name, readonly=F);
	if (is_fail(self.tab)) {
	    fail;
	}
    }

    public.done := function()
    {
	wider self, public;
	self.tab.close();
	val self := F;
	val public := F;
	return T;
    }

    public.addpointsource := function (name, timerange, freqrange,
				       raparms, decparms,
				       iparms, qparms, uparms, vparms)
    {
	if (length(timerange) != 2) {
	    fail 'timerange should be a vector of 2 elements (start,end)';
	}
	if (length(freqrange) != 2) {
	    fail 'freqrange should be a vector of 2 elements (start,end)';
	}
	t1 := self.tab.query (spaste('NAME=="',name,'" ',
				     '&& near(TDOMAIN[1],',timerange[1],') ',
				     '&& near(TDOMAIN[2],',timerange[2],') ',
				     '&& near(FDOMAIN[1],',freqrange[1],') ',
				     '&& near(FDOMAIN[2],',freqrange[2],') '));
	nr := t1.nrows();
	t1.close();
	if (nr != 0) {
	    fail paste('Source', name, 'already defined for given domain');
	}
	# Turn a scalar into a matrix.
	if (length(shape(raparms)) == 1  &&  length(raparms) == 1) {
	    raparms := array (raparms, 1, 1);
	}
	if (length(shape(decparms)) == 1  &&  length(decparms) == 1) {
	    decparms := array (decparms, 1, 1);
	}
	if (length(shape(iparms)) == 1  &&  length(iparms) == 1) {
	    iparms := array (iparms, 1, 1);
	}
	if (length(shape(qparms)) == 1  &&  length(qparms) == 1) {
	    qparms := array (qparms, 1, 1);
	}
	if (length(shape(uparms)) == 1  &&  length(uparms) == 1) {
	    uparms := array (uparms, 1, 1);
	}
	if (length(shape(vparms)) == 1  &&  length(vparms) == 1) {
	    vparms := array (vparms, 1, 1);
	}
	# Check if the parm values are 2-dim numerical arrays.
	if (length(shape(raparms)) != 2  ||  !is_numeric(raparms)) {
	    fail paste('raparms should be a scalar or a 2-dim numerical array');
	}
	if (length(shape(decparms)) != 2  ||  !is_numeric(decparms)) {
	    fail paste('decparms should be a scalar or a 2-dim numerical array');
	}
	if (length(shape(iparms)) != 2  ||  !is_numeric(iparms)) {
	    fail paste('iparms should be a scalar or a 2-dim numerical array');
	}
	if (length(shape(qparms)) != 2  ||  !is_numeric(qparms)) {
	    fail paste('qparms should be a scalar or a 2-dim numerical array');
	}
	if (length(shape(uparms)) != 2  ||  !is_numeric(uparms)) {
	    fail paste('uparms should be a scalar or a 2-dim numerical array');
	}
	if (length(shape(vparms)) != 2  ||  !is_numeric(vparms)) {
	    fail paste('vparms should be a scalar or a 2-dim numerical array');
	}
	# Add the source.
	# Its number is the highest number + 1..
	self.tab.addrows(1);
	rownr := self.tab.nrows();
	if (rownr == 1) {
	    number := 1;
	} else {
	    number := 1 + max(self.tab.getcol('NUMBER'));
	}
	self.tab.putcell ('NAME', rownr, name);
	self.tab.putcell ('NUMBER', rownr, number);
	self.tab.putcell ('TYPE', rownr, 1);    # point source
	self.tab.putcell ('TDOMAIN', rownr, timerange);
	self.tab.putcell ('FDOMAIN', rownr, freqrange);
	self.tab.putcell ('RAPARMS', rownr, as_double(raparms));
	self.tab.putcell ('DECPARMS', rownr, as_double(decparms));
	self.tab.putcell ('IPARMS', rownr, as_double(iparms));
	self.tab.putcell ('QPARMS', rownr, as_double(qparms));
	self.tab.putcell ('UPARMS', rownr, as_double(uparms));
	self.tab.putcell ('VPARMS', rownr, as_double(vparms));
    }

    return ref public;
}
