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
	d9 := tablecreatearraycoldesc  ('ORIG_RVALUES', as_double(0));
	d10:= tablecreatearraycoldesc  ('CVALUES', as_dcomplex(0));
	d11:= tablecreatearraycoldesc  ('ORIG_CVALUES', as_dcomplex(0));
	d12:= tablecreatearraycoldesc  ('MASK', T);
	d13:= tablecreatescalarcoldesc ('PERTURBATION', as_double(0));
	d14:= tablecreatescalarcoldesc ('PERT_REL', T);
	td := tablecreatedesc (d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11,
			       d12, d13, d14);
	self.tab := table (name, td);
	if (is_fail(self.tab)) {
	    fail;
	}
	info := self.tab.info();
	info.type := 'MEP';
	self.tab.putinfo (info);
	self.tab.addreadmeline ('PSS ME parameter values');
	
	# Create the table with initial values.
	itabname := spaste(name,'/INITIALVALUES');
	td := tablecreatedesc (d1, d2, d3, d8, d9, d10, d11, d12, d13, d14);
	self.itab := table (itabname, td);
	if (is_fail(self.itab)) {
	    fail;
	}
	info := self.itab.info();
	info.type := 'MEPinit';
	self.itab.putinfo (info);
	self.itab.addreadmeline ('Initial PSS ME parameter values');
	
	# Make it a subtable of the main table.
	self.tab.putkeyword ('INITIALVALUES', spaste('Table: ',itabname));
    } else {
	self.tab := table (name, readonly=F);
	if (is_fail(self.tab)) {
	    fail;
	}
	self.itab := table (self.tab.getkeyword ('INITIALVALUES'), readonly=F);
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
                                perturbation=1e-6, pertrelative=T, trace=T)
    {
	#----------------------------------------------------------------
	funcname := paste('** pamtable.putinit(',parmname,'):');
	input := [parmname=parmname, values=values, mask=mask,
		  perturbation=perturbation, pertrelative=pertrelative];
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
	    self.itab.putcell ('CVALUES', rownr, as_dcomplex(values));
	    self.itab.putcell ('ORIG_CVALUES', rownr, as_dcomplex(values));
	} else {
	    self.itab.putcell ('RVALUES', rownr, as_double(values));
	    self.itab.putcell ('ORIG_RVALUES', rownr, as_double(values));
	}
	self.itab.putcell ('PERTURBATION', rownr, perturbation);
	self.itab.putcell ('PERT_REL', rownr, pertrelative);
	return T;
    }

    public.put := function (parmname='parmYYY', srcnr=-1, statnr=-1,
			    timerange=[1,1e20], freqrange=[1,1e20], 
			    values=0, mask=unset,
                            perturbation=1e-6, pertrelative=T,
			    trace=F)
    {
	#----------------------------------------------------------------
	funcname := paste('** pamtable.put(',parmname,'):');
	input := [parmname=parmname, values=values, mask=mask,
		  timerange=timerange, freqrange=freqrange,
		  perturbation=perturbation, pertrelative=pertrelative];
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
	    self.tab.putcell ('CVALUES', rownr, as_dcomplex(values));
	    self.tab.putcell ('ORIG_CVALUES', rownr, as_dcomplex(values));
	} else {
	    self.tab.putcell ('RVALUES', rownr, as_double(values));
	    self.tab.putcell ('ORIG_RVALUES', rownr, as_double(values));
	}
	self.tab.putcell ('PERTURBATION', rownr, perturbation);
	self.tab.putcell ('PERT_REL', rownr, pertrelative);
	return T;
    }

    public.perturb := function (where='',
				perturbation=1e-6, pertrelative=T,
				trace=F)
    {
	#----------------------------------------------------------------
	funcname := paste('** pamtable.perturb(',where,'):');
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
	    if (t1.nrows() > 0) {
		for (row in [1:t1.nrows()]) {
		    if (t1.iscelldefined ('RVALUES',row)) {
			a := t1.getcell ('RVALUES', row);
			if (pertrelative) {
			    a *:= perturbation;
			} else {
			    a +:= perturbation;
			}
			t1.putcell ('RVALUES', row, a);
		    } else {
			a := t1.getcell ('CVALUES', row);
			if (pertrelative) {
			    a *:= perturbation;
			} else {
			    a +:= perturbation;
			}
			t1.putcell ('CVALUES', row, a);
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
