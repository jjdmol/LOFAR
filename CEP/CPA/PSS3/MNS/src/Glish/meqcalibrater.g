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
    public.predict := function(modelcolname) {
    
        wider self;
        
        # argument assignment
        self.predictRec.modelcolname := modelcolname
        
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
    
    self.savedataRec := [_method="savedata", _sequence=self.id._sequence]
    public.savedata := function(datacolname) {
    
        wider self;
        
        # argument assignment
        self.savedataRec.datacolname := datacolname
        
        # return
        return defaultservers.run(self.agent, self.savedataRec);
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
    public.getparms := function(parmpatterns) {
    
        wider self;
        
        # argument assignment
        self.getparmsRec.parmpatterns := parmpatterns
        
        # return
        return defaultservers.run(self.agent, self.getparmsRec);
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
const meqcalibrater := function(msname, meqmodel = 'WSRT', skymodel = 'GSM',
				mepdb = 'MEP', spw = 0,
				host='',forcenewserver=F) {
    agent := defaultservers.activate('meqcalibrater', host, forcenewserver)
    id := defaultservers.create(agent, 'meqcalibrater', 'meqcalibrater',
                                [msname=msname, meqmodel=meqmodel,
				 skymodel=skymodel, mepdb=mepdb, spw=spw]);
    return ref _define_meqcalibrater(agent, id);
}

#
# Test function demonstrating the solve loop.
#
const meqcalibratertest := function()
{
    local mc := meqcalibrater('test','meqModel','skyModel','mepDB');

    if (is_fail(mc)) {
	print "TEST> could not instantiate meqcalibrater"
	fail
    }

    mc.settimeintervalsize(3600); # calibrate per 1 hour
    mc.clearsolvableparms();
    mc.setsolvableparms("a.*.b d.e.*", T);
    
    mc.resettimeiterator()
    while (mc.nexttimeinterval())
    {
	print 'TEST> iteration';

 	fit := 1.0;
	while (fit > 0.0001 || fit < -0.0001)
	{
	  mc.predict('MODEL_DATA');
	  fit := mc.solve();
	  
	  print 'TEST> fit = ', fit
	}

	mc.savedata('MODEL_DATA');
	mc.saveresidualdata('DATA', 'MODEL_DATA', 'RESIDUAL_DATA');
	mc.saveparms();
	
	p := mc.getparms("a.b.* a.b.c.*.d");
    }

    mc.done();
    
    return T;
}

