### parmtable.g: Glish script to add parameters to the MEP table
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
print 'include parmtable.g   d11nov2002';

include 'table.g'

parmtable := function (name, create=F)
{
    self:=[=];
    public:=[=];

    if (create) {
	# Create the main table.
	d1 := tablecreatescalarcoldesc ('NAME', '');
	d2 := tablecreatescalarcoldesc ('SRCNR', as_integer(0));
	d3 := tablecreatescalarcoldesc ('STATNR', as_integer(0));
	d4 := tablecreatescalarcoldesc ('STARTTIME', as_double(0));
	d5 := tablecreatescalarcoldesc ('ENDTIME', as_double(0));
	d6 := tablecreatescalarcoldesc ('STARTFREQ', as_double(0));
	d7 := tablecreatescalarcoldesc ('ENDFREQ', as_double(0));
	d8 := tablecreatearraycoldesc  ('RVALUES', as_double(0));
	d9 := tablecreatearraycoldesc  ('SIM_RVALUES', as_double(0));
	d10:= tablecreatearraycoldesc  ('SIM_RPERT', as_double(0));
	d11:= tablecreatearraycoldesc  ('CVALUES', as_dcomplex(0));
	d12:= tablecreatearraycoldesc  ('SIM_CVALUES', as_dcomplex(0));
	d13:= tablecreatearraycoldesc  ('SIM_CPERT', as_dcomplex(0));
	d14:= tablecreatearraycoldesc  ('MASK', T);
	d15:= tablecreatescalarcoldesc ('DIFF', as_double(0));
	d16:= tablecreatescalarcoldesc ('DIFF_REL', T);
	td := tablecreatedesc (d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11,
			       d12, d13, d14, d15, d16);
	self.tab := table (name, td);
	if (is_fail(self.tab)) {
	    fail;
	}
	info := self.tab.info();
	info.type := 'MEP';
	self.tab.putinfo (info);
	self.tab.addreadmeline ('PSS ME parameter values');
	
	# Create the table with initial values.
	itabname := spaste(name,'/DEFAULTVALUES');
	td := tablecreatedesc (d1, d2, d3, d8, d9, d10, d11, d12, d13, d14,
			       d15, d16);
	self.itab := table (itabname, td);
	if (is_fail(self.itab)) {
	    fail;
	}
	info := self.itab.info();
	info.type := 'MEPinit';
	self.itab.putinfo (info);
	self.itab.addreadmeline ('Initial PSS ME parameter values');
	
	# Make it a subtable of the main table.
	self.tab.putkeyword ('DEFAULTVALUES', spaste('Table: ',itabname));
    } else {
	self.tab := table (name, readonly=F);
	if (is_fail(self.tab)) {
	    fail;
	}
	self.itab := table (self.tab.getkeyword ('DEFAULTVALUES'), readonly=F);
	if (is_fail(self.itab)) {
	    fail;
	}
    }

    public.done := function()
    {
	wider self, public;
	self.tab.close();
	self.itab.close();
	val self := F;
	val public := F;
	return T;
    }

    public.putinit := function (parmname='parmXXX', srcnr=-1, statnr=-1,
				values=0, mask=unset,
                                diff=1e-6, diffrelative=T, trace=T)
    {
	#----------------------------------------------------------------
	funcname := paste('** parmtable.putinit(',parmname,'):');
	input := [parmname=parmname, values=values, mask=mask,
		  diff=diff, diffrelative=diffrelative];
	if (trace) print funcname,' input=',input;
	#----------------------------------------------------------------

	t1 := self.itab.query (spaste('NAME=="',parmname,'"'));
	nr := t1.nrows();
	t1.close();
	if (nr != 0) {
	    fail paste('Parameter',parmname,'already has an initial value');
	}
	# Turn a scalar into a matrix.
	if (length(shape(values)) == 1  &&  length(values) == 1) {
	    values := array (values, 1, 1);
	}
	if (length(shape(values)) != 2  ||  !is_numeric(values)) {
	    fail paste('values should be a 2-dim numerical array');
	}
	if (!is_unset(mask)) {
  	  if (length(shape(mask)) == 1  &&  length(mask) == 1) {
	    mask := array (mask, 1, 1);
	  }
          if (!is_boolean(mask) || length(shape(mask)) != 2) {
	    fail paste('mask should be unset or a 2-dim boolean array');
          }
	}
	self.itab.addrows(1);
	rownr := self.itab.nrows();
	self.itab.putcell ('NAME', rownr, parmname);
	self.itab.putcell ('SRCNR', rownr, srcnr);
	self.itab.putcell ('STATNR', rownr, statnr);
	if (! is_unset(mask)) {
	    self.itab.putcell ('MASK', rownr, mask);
	}
	if (is_dcomplex(values) || is_complex(values)) {
	    vals := as_dcomplex(values);
	    self.itab.putcell ('CVALUES', rownr, vals);
	    self.itab.putcell ('SIM_CVALUES', rownr, vals);
	    vals[,] := as_dcomplex(0);
	    self.itab.putcell ('SIM_CPERT', rownr, vals);
	} else {
	    vals := as_double(values);
	    self.itab.putcell ('RVALUES', rownr, vals);
	    self.itab.putcell ('SIM_RVALUES', rownr, vals);
	    vals[,] := as_double(0);
	    self.itab.putcell ('SIM_RPERT', rownr, vals);
	}
	self.itab.putcell ('DIFF', rownr, diff);
	self.itab.putcell ('DIFF_REL', rownr, diffrelative);
	return T;
    }

    public.put := function (parmname='parmYYY', srcnr=-1, statnr=-1,
			    timerange=[1,1e20], freqrange=[1,1e20], 
			    values=0, mask=unset,
                            diff=1e-6, diffrelative=T,
			    trace=F)
    {
	#----------------------------------------------------------------
	funcname := paste('** parmtable.put(',parmname,'):');
	input := [parmname=parmname, values=values, mask=mask,
		  timerange=timerange, freqrange=freqrange,
		  diff=diff, diffrelative=diffrelative];
	if (trace) print funcname,' input=',input;
	#----------------------------------------------------------------


	if (length(timerange) != 2) {
	    fail 'timerange should be a vector of 2 elements (start,end)';
	}
	if (length(freqrange) != 2) {
	    fail 'freqrange should be a vector of 2 elements (start,end)';
	}
	t1 := self.tab.query (spaste('NAME=="',parmname,'" ',
				     '&& SRCNR==',srcnr,
				     '&& STATNR==',statnr,
				     '&& near(STARTTIME,', timerange[1],') ',
				     '&& near(ENDTIME,'  , timerange[2],') ',
				     '&& near(STARTFREQ,', freqrange[1],') ',
				     '&& near(ENDFREQ,'  , freqrange[2],') '));
	nr := t1.nrows();
	t1.close();
	if (nr != 0) {
	    fail paste('Parameter',parmname,'srcnr',srcnr,'statnr',statnr,
                       'already defined for given domain');
	}
	if (length(shape(values)) != 2  ||  !is_numeric(values)) {
	    fail paste('values should be a 2-dim numerical array');
	}
	if (!is_unset(mask)  &&  (!is_boolean(mask) || length(shape(mask)) != 2)) {
	    fail paste('mask should be unset or a 2-dim boolean array');
	}
	self.tab.addrows(1);
	rownr := self.tab.nrows();
	self.tab.putcell ('NAME', rownr, parmname);
	self.tab.putcell ('SRCNR', rownr, srcnr);
	self.tab.putcell ('STATNR', rownr, statnr);
	self.tab.putcell ('STARTTIME', rownr, timerange[1]);
	self.tab.putcell ('ENDTIME', rownr, timerange[2]);
	self.tab.putcell ('STARTFREQ', rownr, freqrange[1]);
	self.tab.putcell ('ENDFREQ', rownr, freqrange[2]);
	if (! is_unset(mask)) {
	    self.tab.putcell ('MASK', rownr, mask);
	}
	if (is_dcomplex(values) || is_complex(values)) {
	    vals := as_dcomplex(values);
	    self.tab.putcell ('CVALUES', rownr, vals);
	    self.tab.putcell ('SIM_CVALUES', rownr, vals);
	    vals[,] := as_dcomplex(0);
	    self.tab.putcell ('SIM_CPERT', rownr, vals);
	} else {
	    vals := as_double(values);
	    self.tab.putcell ('RVALUES', rownr, vals);
	    self.tab.putcell ('SIM_RVALUES', rownr, vals);
	    vals[,] := as_double(0);
	    self.tab.putcell ('SIM_RPERT', rownr, vals);
	}
	self.tab.putcell ('DIFF', rownr, diff);
	self.tab.putcell ('DIFF_REL', rownr, diffrelative);
	return T;
    }

    public.loadgsm := function(gsmname, where='')
    {
	t := table(gsmname);
	if (is_fail(t)) fail;
	tab := t.query (where, sortlist='NUMBER');
	fnd := tab.nrows() > 0;
	if (fnd) {
	    for (row in [1:tab.nrows()]) {
		src := tab.getcell ('NUMBER', row);
		name := tab.getcell('NAME', row);
		public.putinit (spaste('RA.', name),
				src, -1,
				values=tab.getcell ('RAPARMS', row));
#				diff=1e-7, diffrelative=F);
		public.putinit (spaste('DEC.', name),
				src, -1,
				values=tab.getcell ('DECPARMS', row));
#				diff=1e-7, diffrelative=F);
		public.putinit (spaste('StokesI.', name),
				src, -1,
				values=tab.getcell ('IPARMS', row));
		public.putinit (spaste('StokesQ.', name),
				src, -1,
				values=tab.getcell ('QPARMS', row));
		public.putinit (spaste('StokesU.', name),
				src, -1,
				values=tab.getcell ('UPARMS', row));
		public.putinit (spaste('StokesV.', name),
				src, -1,
				values=tab.getcell ('VPARMS', row));
	    }
	}
	print 'Wrote',tab.nrows(),'sources into MEP';
	tab.done();
	t.done();
	if (!fnd) {
	    fail 'no sources found in the gsm';
	}
	return T;
    }

    public.perturb := function (where='',
				perturbation=1e-6, pertrelative=T,
				trace=F)
    {
	#----------------------------------------------------------------
	funcname := paste('** parmtable.perturb(',where,'):');
	input := [where=where,
		  perturbation=perturbation, pertrelative=pertrelative];
	if (trace) print funcname,' input=',input;
	#----------------------------------------------------------------

	for (i in [1,2]) {
	    t1 := ref self.itab;
	    if (i==1) {
		if (where != '') {
		    t1 := self.itab.query (where);
		}
	    } else {
		if (where != '') {
		    t1 := self.tab.query (where);
		} else {
		    t1 := ref self.tab;
		}
	    }
	    if (is_fail(t1)) fail;
	    if (t1.nrows() > 0) {
		for (row in [1:t1.nrows()]) {
		    if (t1.iscelldefined ('RVALUES',row)) {
			vals := t1.getcell ('SIM_RVALUES', row);
			if (pertrelative) {
			    valp := vals * perturbation;
			} else {
			    valp := vals + perturbation;
			}
			t1.putcell ('RVALUES', row, valp);
			t1.putcell ('SIM_RPERT', row, valp-vals);
		    } else {
			vals := t1.getcell ('SIM_CVALUES', row);
			if (pertrelative) {
			    valp := vals * perturbation;
			} else {
			    valp := vals + perturbation;
			}
			t1.putcell ('CVALUES', row, valp);
			t1.putcell ('SIM_CPERT', row, valp-vals);
		    }
		}
	    }
	}
	if (where != '') {
	    t1.close();
	}
	return T;
    }

    public.table := function()
    {
	return ref self.tab;
    }

    public.inittable := function()
    {
	return ref self.itab;
    }

    return ref public;
}
