pragma include once

include 'servers.g'

const _define_meqcalibrater := function(ref agent, id) {

    self       := [=]
    public     := [=]
    self.agent := ref agent;
    self.id    := id;
    public     := defaultservers.init_object(self)
    
    
    self.setTimeIntervalSizeRec := [_method="setTimeIntervalSize",
				    _sequence=self.id._sequence]
    public.setTimeIntervalSize := function(secondsInterval) {
    
        wider self;
        
        # argument assignment
        self.setTimeIntervalSizeRec.secondsInterval := secondsInterval
        
        # return
        return defaultservers.run(self.agent, self.setTimeIntervalSizeRec);
    }
    
    self.resetTimeIteratorRec := [_method="resetTimeIterator",
				  _sequence=self.id._sequence]
    public.resetTimeIterator := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.resetTimeIteratorRec);
    }
    
    self.nextTimeIntervalRec := [_method="nextTimeInterval",
				 _sequence=self.id._sequence]
    public.nextTimeInterval := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.nextTimeIntervalRec);
    }
    
    self.clearSolvableParmsRec := [_method="clearSolvableParms",
				   _sequence=self.id._sequence]
    public.clearSolvableParms := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.clearSolvableParmsRec);
    }
    
    self.setSolvableParmsRec := [_method="setSolvableParms",
				 _sequence=self.id._sequence]
    public.setSolvableParms := function(parmPatterns, isSolvable) {
    
        wider self;
        
        # argument assignment
        self.setSolvableParmsRec.parmPatterns := parmPatterns
        self.setSolvableParmsRec.isSolvable := isSolvable
        
        # return
        return defaultservers.run(self.agent, self.setSolvableParmsRec);
    }
    
    self.predictRec := [_method="predict", _sequence=self.id._sequence]
    public.predict := function(modelColName) {
    
        wider self;
        
        # argument assignment
        self.predictRec.modelColName := modelColName
        
        # return
        return defaultservers.run(self.agent, self.predictRec);
    }
    
    self.solveRec := [_method="solve", _sequence=self.id._sequence]
    public.solve := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.solveRec);
    }
    
    self.saveParmsRec := [_method="saveParms", _sequence=self.id._sequence]
    public.saveParms := function() {
    
        wider self;
        
        # return
        return defaultservers.run(self.agent, self.saveParmsRec);
    }
    
    self.saveDataRec := [_method="saveData", _sequence=self.id._sequence]
    public.saveData := function(dataColName) {
    
        wider self;
        
        # argument assignment
        self.saveDataRec.dataColName := dataColName
        
        # return
        return defaultservers.run(self.agent, self.saveDataRec);
    }
    
    self.saveResidualDataRec := [_method="saveResidualData", _sequence=self.id._sequence]
    public.saveResidualData := function(colAName, colBName, residualColName) {
    
        wider self;
        
        # argument assignment
        self.saveResidualDataRec.colAName := colAName
        self.saveResidualDataRec.colBName := colBName
        self.saveResidualDataRec.residualColName := residualColName
        
        # return
        return defaultservers.run(self.agent, self.saveResidualDataRec);
    }
    
    self.getParmsRec := [_method="getParms", _sequence=self.id._sequence]
    public.getParms := function(parmPatterns) {
    
        wider self;
        
        # argument assignment
        self.getParmsRec.parmPatterns := parmPatterns
        
        # return
        return defaultservers.run(self.agent, self.getParmsRec);
    }
    
}

const meqcalibrater := function(msName, meqModel = "WSRT", skyModel = "GSM",
				mepDB = "MEP", spw = 0,
				host='',forcenewserver=F) {
    agent := defaultservers.activate('meqcalibrater', host, forcenewserver)
    id := defaultservers.create(agent, 'meqcalibrater', 'meqcalibrater',
                                [msName, meqModel, skyModel, mepDB, spw]);
    return ref _define_MeqCalibrater(agent, id);
}

