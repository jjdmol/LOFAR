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
    
    
    self.settimeintervalsizeRec := [_method="settimeintervalsize",
				    _sequence=self.id._sequence]
    public.settimeintervalsize := function(secondsinterval) {
    
        wider self;
        
        # argument assignment
        self.settimeintervalsizeRec.secondsinterval := secondsinterval
        
        # return
        return defaultservers.run(self.agent, self.settimeintervalsizeRec);
    }
    
    self.resettimeiteratorRec := [_method="resettimeiterator",
				  _sequence=self.id._sequence]
    public.resettimeiterator := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.resettimeiteratorRec);
    }
    
    self.nexttimeintervalRec := [_method="nexttimeinterval",
				 _sequence=self.id._sequence]
    public.nexttimeinterval := function() {
    
        wider self;
        
        # return
	return defaultservers.run(self.agent, self.nexttimeintervalRec);
    }
    
    self.clearsolvableparmsRec := [_method="clearsolvableparms",
				   _sequence=self.id._sequence]
    public.clearsolvableparms := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.clearsolvableparmsRec);
    }
    
    self.setsolvableparmsRec := [_method="setsolvableparms",
				 _sequence=self.id._sequence]
    public.setsolvableparms := function(parmpatterns, issolvable) {
    
        wider self;
        
        # argument assignment
        self.setsolvableparmsRec.parmpatterns := parmpatterns
        self.setsolvableparmsRec.issolvable := issolvable
        
        # return
        return defaultservers.run(self.agent, self.setsolvableparmsRec);
    }
    
    self.predictRec := [_method="predict", _sequence=self.id._sequence]
    public.predict := function(modeldatacolname) {
    
        wider self;
        
        # argument assignment
        self.predictRec.modeldatacolname := modeldatacolname
        
        # return
        return defaultservers.run(self.agent, self.predictRec);
    }
    
    self.solveRec := [_method="solve", _sequence=self.id._sequence]
    public.solve := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.solveRec);
    }
    
    self.saveparmsRec := [_method="saveparms", _sequence=self.id._sequence]
    public.saveparms := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.saveparmsRec);
    }
    
    self.savepredicteddataRec := [_method="savepredicteddata", _sequence=self.id._sequence]
    public.savepredicteddata := function(modeldatacolname) {
    
        wider self;
        
        # argument assignment
        self.savepredicteddataRec.modeldatacolname := modeldatacolname
        
        # return
        return defaultservers.run(self.agent, self.savepredicteddataRec);
    }
    
    self.saveresidualdataRec := [_method="saveresidualdata", _sequence=self.id._sequence]
    public.saveresidualdata := function(colaname, colbname, residualcolname) {
    
        wider self;
        
        # argument assignment
        self.saveresidualdataRec.colaname := colaname
        self.saveresidualdataRec.colbname := colbname
        self.saveresidualdataRec.residualcolname := residualcolname
        
        # return
        return defaultservers.run(self.agent, self.saveresidualdataRec);
    }
    
    self.getparmsRec := [_method="getparms", _sequence=self.id._sequence]
    public.getparms := function(parmpatterns, excludepatterns="") {
    
        wider self;
        
        # argument assignment
        self.getparmsRec.parmpatterns    := parmpatterns
	self.getparmsRec.excludepatterns := excludepatterns
        
        # return
        return defaultservers.run(self.agent, self.getparmsRec);
    }

    self.getsolvedomainRec := [_method="getsolvedomain", _sequence=self.id._sequence]
    public.getsolvedomain := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.getsolvedomainRec);
    }

    self.timeiteratorpastendRec := [_method="timeiteratorpastend", _sequence=self.id._sequence]
    public.timeiteratorpastend := function() {

	wider self;

	# return
	return defaultservers.run(self.agent, self.timeiteratorpastendRec);
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
const meqcalibrater := function(msname, meqmodel = 'LOFAR', skymodel = 'GSM', spw = 0,
				host='',forcenewserver=F) {
    agent := defaultservers.activate('meqcalibrater', host, forcenewserver)
    id := defaultservers.create(agent, 'meqcalibrater', 'meqcalibrater',
                                [msname=msname, meqmodel=meqmodel,
				 skymodel=skymodel, spw=spw]);
    return ref _define_meqcalibrater(agent, id);
}

#
# Test function demonstrating the solve loop.
#
const meqcalibratertest := function()
{
    local mc := meqcalibrater('myms.ms', 'TEST', 'TEST');

    if (is_fail(mc)) {
	print "meqcalibratertest(): could not instantiate meqcalibrater"
	fail
    }

    mc.settimeintervalsize(3600); # calibrate per 1 hour
    mc.clearsolvableparms();
    mc.setsolvableparms("a.b.* f.g.*[34]", T);
    
    mc.resettimeiterator()
    i := 0
    while (! mc.timeiteratorpastend())
    {
	d := mc.getsolvedomain();
	print 'solvedomain = ', d;

 	fit := 1.0;
	while (fit > 0.0001 || fit < -0.0001)
	{
	  mc.predict('MODEL_DATA');
	  fit := mc.solve();

	  print 'iteration = ', i, ' fit = ', fit
	}

	mc.savepredicteddata('MODEL_DATA');
	# mc.saveresidualdata('DATA', 'MODEL_DATA', 'RESIDUAL_DATA');
	# mc.saveparms();
	
	parms := mc.getparms("f.g.h.i.j.[1-5]");
	print 'getparms = ', parms

	i+:=1;

	mc.nexttimeinterval();
    }

    mc.done();
    
    return T;
}

