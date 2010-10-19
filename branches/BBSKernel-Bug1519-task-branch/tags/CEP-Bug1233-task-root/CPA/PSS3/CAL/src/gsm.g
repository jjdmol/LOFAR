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

# pragma include once
print 'include gsm.g   d11nov2002';

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

    public.addpointsource := function (name='srcXXX', 
				       timerange=[0,1e20], freqrange=[0,1e20],
				       raparms=2.5, decparms=0.5,
				       iparms=1, qparms=0, uparms=0, vparms=0,
				       trace=T)
    {
	#---------------------------------------------------------------------
	funcname := paste('** gsm.addpointsource(',name,'):');
	input := [name=name, timerange=timerange, freqrange=freqrange,
		  raparms=raparms, decparms=decparms,
		  iparms=iparms, qparms=qparms, uparms=uparms, vparms=vparms];
	if (trace) print funcname,' input=',input;
	#---------------------------------------------------------------------

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
	if (trace) print funcname,input.name,' -> NUMBER=',number;
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


#Perturb the parameters of the specified source:

    public.perturb := function (name='srcXXX', number=0,
				dRA_rad=0, dDEC_rad=0, I_mult=1.0, 
				modify=T, trace=T)
    {
	wider self;
	#---------------------------------------------------------------------
	funcname := paste('** gsm.perturb(',name,'):');
	input := [name=name, number=number,  modify=modify,
		  dRA_rad=dRA_rad, dDEC_rad=dDEC_rad, I_mult=I_mult];
	if (trace) print funcname,' input=',input;
	#---------------------------------------------------------------------

	# Check the presence (and the rownr) of the specified source(s):
	# s := spaste('NAME=="',name,'" || NUMBER==[',number,']');
	# s := spaste('NAME in "',name,'" || NUMBER==[',number,']');
	# s := spaste('NUMBER==',number[1]);           # temporary
	s := spaste('NUMBER in [',number,']');      # temporary
	if (trace) print funcname,'query=',s;
	t1 := self.tab.query(s);
	if (!is_record(t1)) {
	    fail paste(funcname,'subtable t1 is not a record');
	} else {
	    rownr := t1.rownumbers();           # row nrs in self.tab
	    if (trace) print funcname,len(rownr),' rownr=',rownr;
	    t1.close();
	    if (len(rownr) == 0) {
		fail paste(funcname,'Source', name, number,'not in GSM');
	    } else if (len(rownr) > 1) {
		fail paste(funcname,'Only one source at a time');
	    }
	}

	# Modify the source.
	if (modify) {
	    if (dRA_rad != 0) {
		colname := 'RAPARMS';
		v := self.tab.getcell (colname, rownr);
		v[1,1] +:= dRA_rad;
		self.tab.putcell (colname, rownr, v);
		if (trace) print rownr, colname,'->',v; 
	    }
	    if (dDEC_rad != 0) {
		colname := 'DECPARMS';
		v := self.tab.getcell (colname, rownr);
		v[1,1] +:= dDEC_rad;
		self.tab.putcell (colname, rownr, v);	
		if (trace) print rownr, colname,'->',v; 
	    }
	    if (I_mult != 1.0) {
		colname := 'IPARMS';
		v := self.tab.getcell (colname, rownr);
		v[1,1] *:= I_mult;
		self.tab.putcell (colname, rownr, v);
		if (trace) print rownr, colname,'->',v; 
	    }
	}
	return T;
    }

# Table access:

    public.table := function()
    {
	return ref self.tab;
    }

    return ref public;
}
