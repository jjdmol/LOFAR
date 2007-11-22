### 
### meqcalibrater.g: Glish script to control MesTree based self-calibration.
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

include 'servers.g'

#
# meqcalibrater closure
#
const _define_meqcalibrater := function(ref agent, id) {

    self       := [=]
    public     := [=]
    self.agent := ref agent;
    self.id    := id;
    public     := defaultservers.init_object(self)
    
    
    self.settimeintervalRec := [_method='settimeinterval',
			        _sequence=self.id._sequence]
    public.settimeinterval := function(secondsinterval) {
    
        wider self;
        
        # argument assignment
        self.settimeintervalRec.secondsinterval := secondsinterval
        
        # return
        return defaultservers.run(self.agent, self.settimeintervalRec);
    }
    
    self.resetiteratorRec := [_method='resetiterator',
			      _sequence=self.id._sequence]
    public.resetiterator := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.resetiteratorRec);
    }
    
    self.nextintervalRec := [_method='nextinterval',
			     _sequence=self.id._sequence]
    public.nextinterval := function() {
    
        wider self;
        
        # return
	return defaultservers.run(self.agent, self.nextintervalRec);
    }
    
    self.clearsolvableparmsRec := [_method='clearsolvableparms',
				   _sequence=self.id._sequence]
    public.clearsolvableparms := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.clearsolvableparmsRec);
    }
    
    self.setsolvableparmsRec := [_method='setsolvableparms',
				 _sequence=self.id._sequence]
    public.setsolvableparms := function(parmpatterns, excludepatterns='', issolvable=T) {
    
        wider self;
        
        # argument assignment
        self.setsolvableparmsRec.parmpatterns    := parmpatterns
	self.setsolvableparmsRec.excludepatterns := excludepatterns
        self.setsolvableparmsRec.issolvable      := issolvable
        
        # return
        return defaultservers.run(self.agent, self.setsolvableparmsRec);
    }
    
    self.predictRec := [_method='predict', _sequence=self.id._sequence]
    public.predict := function(modeldatacolname = 'MODEL_DATA') {
    
        wider self;
        
        # argument assignment
        self.predictRec.modeldatacolname := modeldatacolname
        
        # return
        return defaultservers.run(self.agent, self.predictRec);
    }
    
    self.selectRec := [_method='select', _sequence=self.id._sequence]
    public.select := function (where='', firstchan=0, lastchan=-1) {
    
        wider self;
        
        # argument assignment
        self.selectRec.where := where
        self.selectRec.firstchan := firstchan
        self.selectRec.lastchan := lastchan

        # return
        return defaultservers.run(self.agent, self.selectRec);
    }
    
    self.solveselectRec := [_method='solveselect', _sequence=self.id._sequence]
    public.solveselect := function (where='') {
    
        wider self;
        
        # argument assignment
        self.solveselectRec.where := where

        # return
        return defaultservers.run(self.agent, self.solveselectRec);
    }
    
    self.peelRec := [_method='peel', _sequence=self.id._sequence]
    public.peel := function (peelsourcenrs, extrasourcenrs) {
    
        wider self;
        
        # argument assignment
        self.peelRec.peelsourcenrs  := peelsourcenrs
        self.peelRec.extrasourcenrs := extrasourcenrs

        # return
        return defaultservers.run(self.agent, self.peelRec);
    }
    
    self.solveRec := [_method='solve', _sequence=self.id._sequence]
    public.solve := function(useSVD=F) {
    
        wider self;

        # argument assignment
        self.solveRec.realsol := useSVD;

        # return
        return defaultservers.run(self.agent, self.solveRec);
    }
    
    self.saveparmsRec := [_method='saveparms', _sequence=self.id._sequence]
    public.saveparms := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.saveparmsRec);
    }
    
    self.saveresidualdataRec := [_method='saveresidualdata',
				 _sequence=self.id._sequence]
    public.saveresidualdata := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.saveresidualdataRec);
    }
    
    self.getresidualdataRec := [_method='getresidualdata',
				_sequence=self.id._sequence]
    public.getresidualdata := function() {
    
        wider self;
        
        # argument assignment
        self.getresidualdataRec.datacolname := datacolname

        # return
        return defaultservers.run(self.agent, self.getresidualdataRec);
    }
    
    self.getparmsRec := [_method='getparms', _sequence=self.id._sequence]
    public.getparms := function(parmpatterns, excludepatterns='',
				issolvable=unset, denormalize=F) {
    
        wider self;
        
        # argument assignment
        self.getparmsRec.parmpatterns    := parmpatterns
	self.getparmsRec.excludepatterns := excludepatterns
	self.getparmsRec.issolvable := -1;
	if (is_boolean(issolvable)) {
	    self.getparmsRec.issolvable := 0;
	    if (issolvable) self.getparmsRec.issolvable := 1;
	}
	self.getparmsRec.denormalize := denormalize;
        # return
        return defaultservers.run(self.agent, self.getparmsRec);
    }

    self.getsolvedomainRec := [_method='getsolvedomain', _sequence=self.id._sequence]
    public.getsolvedomain := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.getsolvedomainRec);
    }

    self.getstatisticsRec := [_method='getstatistics', _sequence=self.id._sequence]
    public.getstatistics := function (detailed=F, clear=T) {
    
        wider self;
        
        # argument assignment
        self.getstatisticsRec.detailed := detailed
        self.getstatisticsRec.clear := clear

        # return
        return defaultservers.run(self.agent, self.getstatisticsRec);
    }
    
    self.getparmnamesRec := [_method='getparmnames', _sequence=self.id._sequence]
    public.getparmnames := function(parmpatterns='*',
				    excludepatterns="u.* v.* w.*") {
    
        wider self;
        
        # argument assignment
        self.getparmnamesRec.parmpatterns    := parmpatterns
	self.getparmnamesRec.excludepatterns := excludepatterns
        
        # return
        return defaultservers.run(self.agent, self.getparmnamesRec);
    }

    public.id := function() {
	wider self;
	return self.id.objectid;
    }

    public.done := function() {
    	wider self, public;
	ok := defaultservers.done(self.agent, public.id());
	if (ok)
	{
	    self := F;
	    val public := F;
	}
	return ok;
    }

    return ref public;
}

#
# meqcalibrater constructor
#
const meqcalibrater := function(msname, meqmodel = 'LOFAR', skymodel = 'GSM',
                                ddid = 0, ant=[], ant1=[], ant2=[], 
				modeltype='LOFAR', calcuvw=T, 
				datacolname='MODEL_DATA',
				residualcolname='CORRECTED_DATA',
				dbtype="aips", dbname="", dbpwd="",
				host='', forcenewserver=F) {
    if (len(ant1) == 0) ant1:=ant;
    if (len(ant2) == 0) ant2:=ant;
    agent := defaultservers.activate('meqcalibrater', host, forcenewserver)
    id := defaultservers.create(agent, 'meqcalibrater', 'meqcalibrater',
                                [msname=msname, meqmodel=meqmodel,
				 skymodel=skymodel, ddid=ddid,
				 ant1=ant1, ant2=ant2, modeltype=modeltype,
				 calcuvw=calcuvw, datacolname=datacolname,
				 residualcolname=residualcolname,
				 dbtype=dbtype, dbname=dbname, dbpwd=dbpwd])
    return ref _define_meqcalibrater(agent, id);
}

#
# Test function demonstrating the solve loop.
#
const meqcalibratertest := function()
{
    local mc := meqcalibrater('myms.ms', 'TEST', 'TEST');

    if (is_fail(mc)) {
	print 'meqcalibratertest(): could not instantiate meqcalibrater'
	fail
    }

    mc.settimeinterval(3600); # calibrate per 1 hour
    mc.clearsolvableparms();
    mc.setsolvableparms("a.b.* f.g.*[34]");
    mc.setsolvableparms("Leakage.*");
    
    mc.resetiterator()
    i := 0
    while (mc.nextinterval())
    {
	d := mc.getsolvedomain();
	print 'solvedomain = ', d;

 	fit := 1.0;
	while (fit > 0.0001 || fit < -0.0001)
	{
	  fit := mc.solve();
	  print 'iteration = ', i, ' fit = ', fit
	}

	mc.predict('MODEL_DATA');

	# mc.saveresidualdata('DATA', 'MODEL_DATA', 'RESIDUAL_DATA');
	# mc.saveparms();
	
	parms := mc.getparms("f.g.h.i.j.[1-5]");
	print 'getparms = ', parms

	i+:=1;
    }

    mc.done();
    
    return T;
}

