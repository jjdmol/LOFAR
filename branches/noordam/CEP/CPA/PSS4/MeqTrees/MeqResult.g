# file: ../MeqTrees/MeqResult.g

#---------------------------------------------------------

# pragma include once
print '\n\n\n\n\n\n=================================================';
print 'include MeqResult.g    d15d19sep2003';

include 'genericClosureFunctions.g';




############################################################################
# Glish counterpart of C++ MeqResult object.
############################################################################

MeqResult := function (name=F) {
    public := [=];
    private := [=];
    genericClosureFunctions(public=public, private=private, name=name,
			    type='MeqResult');

    private.init := function () {
	wider private;
	return T;
    }

#===========================================================================
# Public interface:
#===========================================================================

#===========================================================================
# Private functions:
#===========================================================================

# Summary of the contents of this object: 
# NB: Called by genericClosureFunction .summary()

    private.summary := function () {
	ss := "";
	ss := paste(ss,'\n-','<specific part>');
	return ss;
    }

# Format part of the contents of this object:
# NB: Called by genericClosureFunction .show()

    private.show := function (what=F, aux=F) {
	ss := "";
	ss := paste(ss,'\n-','<specific part>');
	return ss;
    } 

#===========================================================================
# Finished:
#===========================================================================

    private.init();
    return public;
}











############################################################################
# Glish counterpart of C++ MeqDomain object.
############################################################################

MeqDomain := function (name=F) {
    public := [=];
    private := [=];
    genericClosureFunctions(public=public, private=private, name=name,
			    type='MeqDomain');

    private.init := function () {
	wider private;
	return T;
    }

#===========================================================================
# Public interface:
#===========================================================================

#===========================================================================
# Private functions:
#===========================================================================

# Summary of the contents of this object: 
# NB: Called by genericClosureFunction .summary()

    private.summary := function () {
	ss := "";
	ss := paste(ss,'\n-','<specific part>');
	return ss;
    }

# Format part of the contents of this object:
# NB: Called by genericClosureFunction .show()

    private.show := function (what=F, aux=F) {
	ss := "";
	ss := paste(ss,'\n-','<specific part>');
	return ss;
    } 

#===========================================================================
# Finished:
#===========================================================================

    private.init();
    return public;
}






############################################################################
# Glish counterpart of C++ MeqCells object.
############################################################################

MeqCells := function (name=F) {
    public := [=];
    private := [=];
    genericClosureFunctions(public=public, private=private, name=name,
			    type='MeqCells');

    private.init := function () {
	wider private;
	return T;
    }

#===========================================================================
# Public interface:
#===========================================================================

#===========================================================================
# Private functions:
#===========================================================================

# Summary of the contents of this object: 
# NB: Called by genericClosureFunction .summary()

    private.summary := function () {
	ss := "";
	ss := paste(ss,'\n-','<specific part>');
	return ss;
    }

# Format part of the contents of this object:
# NB: Called by genericClosureFunction .show()

    private.show := function (what=F, aux=F) {
	ss := "";
	ss := paste(ss,'\n-','<specific part>');
	return ss;
    } 

#===========================================================================
# Finished:
#===========================================================================

    private.init();
    return public;
}






############################################################################
# Glish counterpart of C++ MeqVells object.
############################################################################

MeqVells := function (name=F) {
    public := [=];
    private := [=];
    genericClosureFunctions(public=public, private=private, name=name,
			    type='MeqVells');

    private.init := function () {
	wider private;
	return T;
    }

#===========================================================================
# Public interface:
#===========================================================================

#===========================================================================
# Private functions:
#===========================================================================

# Summary of the contents of this object: 
# NB: Called by genericClosureFunction .summary()

    private.summary := function () {
	ss := "";
	ss := paste(ss,'\n-','<specific part>');
	return ss;
    }

# Format part of the contents of this object:
# NB: Called by genericClosureFunction .show()

    private.show := function (what=F, aux=F) {
	ss := "";
	ss := paste(ss,'\n-','<specific part>');
	return ss;
    } 

#===========================================================================
# Finished:
#===========================================================================

    private.init();
    return public;
}






############################################################################
# Glish counterpart of C++ MeqPolc object.
############################################################################

MeqPolc := function (name=F) {
    public := [=];
    private := [=];
    genericClosureFunctions(public=public, private=private, name=name,
			    type='MeqPolc');

    private.init := function () {
	wider private;
	return T;
    }

#===========================================================================
# Public interface:
#===========================================================================

#===========================================================================
# Private functions:
#===========================================================================

# Summary of the contents of this object: 
# NB: Called by genericClosureFunction .summary()

    private.summary := function () {
	ss := "";
	ss := paste(ss,'\n-','<specific part>');
	return ss;
    }

# Format part of the contents of this object:
# NB: Called by genericClosureFunction .show()

    private.show := function (what=F, aux=F) {
	ss := "";
	ss := paste(ss,'\n-','<specific part>');
	return ss;
    } 

#===========================================================================
# Finished:
#===========================================================================

    private.init();
    return public;
}



############################################################################
# Glish counterpart of C++ MeqRequest object.
############################################################################

MeqRequest := function (name=F) {
    public := [=];
    private := [=];
    genericClosureFunctions(public=public, private=private, name=name,
			    type='MeqRequest');

    private.init := function () {
	wider private;
	return T;
    }

#===========================================================================
# Public interface:
#===========================================================================

#===========================================================================
# Private functions:
#===========================================================================

# Summary of the contents of this object: 
# NB: Called by genericClosureFunction .summary()

    private.summary := function () {
	ss := "";
	ss := paste(ss,'\n-','<specific part>');
	return ss;
    }

# Format part of the contents of this object:
# NB: Called by genericClosureFunction .show()

    private.show := function (what=F, aux=F) {
	ss := "";
	ss := paste(ss,'\n-','<specific part>');
	return ss;
    } 

#===========================================================================
# Finished:
#===========================================================================

    private.init();
    return public;
}




############################################################################
# Glish counterpart of C++ MeqRequestSequence object.
############################################################################

MeqRequestSequence := function (name=F) {
    public := [=];
    private := [=];
    genericClosureFunctions(public=public, private=private, name=name,
			    type='MeqRequestSequence');

    private.init := function () {
	wider private;
	return T;
    }

#===========================================================================
# Public interface:
#===========================================================================

#===========================================================================
# Private functions:
#===========================================================================

# Summary of the contents of this object: 
# NB: Called by genericClosureFunction .summary()

    private.summary := function () {
	ss := "";
	ss := paste(ss,'\n-','<specific part>');
	return ss;
    }

# Format part of the contents of this object:
# NB: Called by genericClosureFunction .show()

    private.show := function (what=F, aux=F) {
	ss := "";
	ss := paste(ss,'\n-','<specific part>');
	return ss;
    } 

#===========================================================================
# Finished:
#===========================================================================

    private.init();
    return public;
}










############################################################################
# test-function:
############################################################################

test_MeqResult := function () {
    include 'inspect.g';
    print '\n\n****************************************************';
    print '** test_MeqResult:';

    # Create global object(s):
    print '\n** Create global objects';
    # global mqr, mqq, mqqs, mqd, mqc, mqv, mqp;
    print s := paste("mqr mqq mqqs mqd mqc mqv mqp", sep=',');
    print eval(paste('gobal',s));

    print '\n',mqr := MeqResult('mqr');
    print '\n',mqq := MeqRequest('mqq');
    print '\n',mqqs := MeqRequestSequence('mqqs');
    print '\n',mqd := MeqDomain('mqd');
    print '\n',mqc := MeqCells('mqc');
    print '\n',mqv := MeqVells('mqv');
    print '\n',mqp := MeqPolc('mqp');
    print ' ';

    print '\n',mqp.its();

    # Inspect global object(s):
    # msv.inspect('mqr');

    return T;
}
# Execute the test-function:
test_MeqResult();

############################################################################
