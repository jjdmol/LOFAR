# file: ../MeqTrees/MeqNode.g

#---------------------------------------------------------

# pragma include once
print '\n\n\n\n\n\n=================================================';
print 'include MeqNode.g    d19/h21/d22/d26/d29sep2003';

include 'unset.g';
include 'genericClosureFunctions.g';
# include 'MeqTree.g';                       # also has MeqName()


############################################################################
# Dummy MeqServer application proxy (OMS)
############################################################################


dummyMeqServer := function (name=F) {
    public := [=];
    private := [=];
    genericClosureFunctions(public=public, private=private, name=name,
			    type='dummyMeqServer');

    private.init := function () {
	wider private;
	return T;
    }

#===========================================================================
# Public interface:
#===========================================================================

    public.relay := function () {return ref public.agent}

# Issue a command to a node (or a solver etc):

    public.meq := function (command=F, args=F, wait_reply=F, trace=T) {
	funcname := private._.onentry('.meq()');

	# If no argument record specified, make default one:
	if (!is_record(args)) {
	    if (command=='Create.Node') {
		args := [class='MEQNode', name='test'];
	    } else if (command=='Get.Node.State') {
		args := [name='test'];
		args := [nodeindex=1];
	    } else {
		args := [=];
		public.message(notrec=command);
	    }
	}

	# Always return rr as an event:
	if (command=='Create.Node') {
	    rr := [nodeindex=1, message='node 1 created'];
	    public.agent -> meqserver_out_app_result_create_node(rr);
	} else if (command=='Get.Node.State') {
	    rr := [class='MEQNode', name='test'];
	    public.agent -> meqserver_out_app_result_get_node_state(rr);
	} else {
	    public.message(notrec=command);
	}

	# If wait_reply=T, 'wait for' the reply and return it:
	if (!wait_reply) rr := T;
	return private._.onexit(funcname, rr);
    }


#===========================================================================
# Private functions:
#===========================================================================


#===========================================================================
# Finished:
#===========================================================================

    private.init();
    return public;
}


############################################################################
# MeqNode interface object.
############################################################################

# A MeqNode interface object allows the user to interact with the
# actual MeqNodes that are stored in a C++ node repository. 

# Apart from functions to define (groups of) nodes (names/indices),
# this interface object maintains a 'current' node in the form of
# one of the specific node objects (MeqParm, MeqFunc, etc).
# Command issued to this current node are conveyed to C++.
# (NB: In a later state we might have such a ficilty on all
#      specific Glish node objects, but not yet)

MeqNode := function (name=F, ref server=F) {
    public := [=];
    private := [=];
    genericClosureFunctions(public=public, private=private, name=name,
			    type='MeqNode');

    # Each MeqNode interface object is attached to its own
    # C++ node repository:
    private.state.repository := 'default';


    # Attach the MeqServer application proxy:
    private.mqs := ref server;
    # include 'dummyMeqServer.g';
    private.mqs := dummyMeqServer();
    whenever private.mqs.agent -> * do {
	s := paste('msv.agent ->');
	s := paste(s,private._.format($value,$name));
	public.message(s);
	if ($name=='getState') {
	} else if ($name=='getResult') {
	} else if ($name=='getDefrec') {
	    # do nothing, wait_reply=T
	} else {
	    public.message('msv event',notrec=$name);
	}
    }


    private.init := function () {
	wider private;

	# The interface objects maintains one or more 'current' nodes.
	private.state.current := [=];

	return T;
    }

#===========================================================================
# Public interface:
#===========================================================================


# Return a string vector of existing node-names:
# The pattern can be a vector of patterns.
# If the pattern is integer, it is assumed to be a vector of 
# node indices 

    public.nodenames := function (pattern="*") {
	ss := "";
	if (is_integer(pattern)) {
	    # Assume a vector of node indices:
	    ss := public.index2name(pattern);
	} else if (!is_string(pattern)) {
	    # error
	} else {
	    for (p in pattern) {
		s := '*';
		ss := [ss,s];
	    }
	}
	return ss;
    }
    public.spigots := function (pattern="*") {
    }
    public.parms := function (pattern="*") {
    }

# Get the repository indices of the specified nodes:

    public.name2index := function (nodenames=F) {
    }

# Get the names of the nodes at the specified (vector of) indices:

    public.index2name := function (index=F) {
	ss := private.mqs.meq('index2name',[index=index], wait_reply=T);
	return ss;
    }

#-------------------------------------------------------------------------
# Interaction with the 'current' node in the C++ repository:
#-------------------------------------------------------------------------

# Get an existing node from the C++ repository and convert it to a
# specific Glish node object. Store it as private.state.current:
# NB: All the generic node functions may be accessed via:
#     rr := mqn.current().function()

    public.current := function (pattern=F) {
	wider private;
	if (!is_boolean(pattern)) {
	    # Get the node from the C++ repository:
	    private.mqs.meq('getState',[name=pattern]);
	    if (state.type == 'MeqFunc') {
		node := MeqFunc(state);
	    } else if (state.type == 'MeqParm') {
		node := MeqParm(state);
	    } else {
		s := paste('state.type not recognised:',state.type);
		public.message(s, error=T);
	    }
	    private.state.current := node;    # store   
	} 
	return private.state.current;
    }

# Issue a .getResult() call to a node. If it does not have the desired
# MeqResult in its cache, it will take the necessary steps.
# The MeqResult is returned via an event (see above).

    public.getResult := function(request_id) {
	r := private.mqs.meq ('getResult',[request_id=request_id], 
				  wait_reply=F, trace=trace);
	return r;
    }

# Get the definition record of a given MeqNode class:

    public.defrec := function(classname='classname', child=F, trace=T) {
	rr := private.mqs.meq ('getDefrec', wait_reply=T, trace=trace);
	if (is_string(child)) {
	    # Vector of child names given:
	    for (c in child) {
	    }
	} else if (is_record(child)) {
	    # Vector of child records (defrecs) given(??):
	    for (c in child) {
	    }
	} 
	return rr;
    }

# Create a node in the C++ repository:

    public.create := function (name='classname.a.b.c', defrec=F, trace=T) {
	index := private.mqs.meq ('create', [defrec=defrec], 
				      wait_reply=T, trace=trace);
	return index;
    }

#===========================================================================
# Private functions:
#===========================================================================



#===========================================================================
# Finished:
#===========================================================================

    private.init();
    return public;
}







############################################################################
# Specific MeqNode object: MeqFunc
############################################################################



MeqFunc := function (name=F) {
    public := [=];
    private := [=];
    genericMeqNodeFunctions(public=public, private=private, name=name,
			    type='MeqFunc', descr='MeqNode math expression object');

#===========================================================================
# Specific public interface:
#===========================================================================

#===========================================================================
#  Specific private functions:
#===========================================================================

#===========================================================================
# Finished:
#===========================================================================

    return public;
}





############################################################################
# Specific MeqNode object: MeqParm
############################################################################

MeqParm := function (name=F) {
    public := [=];
    private := [=];
    genericMeqNodeFunctions(public=public, private=private, name=name,
			    type='MeqParm', descr='MeqNode parameter interface object');

#===========================================================================
# Specific public interface:
#===========================================================================

    public.plot := function () {
	s := private.its.type;
	print s,': specific .plot() not implemented';
	return T;
    }

#===========================================================================
#  Specific private functions:
#===========================================================================

#===========================================================================
# Finished:
#===========================================================================

    return public;
}





############################################################################
# Generic functions for all types of node-objects:
# (MeqFunc, MeqParm, MeqSpigot etc)
############################################################################



genericMeqNodeFunctions := function (ref public=F, ref private=F, 
				     type='MeqNode', name=F, descr=F) {
    genericClosureFunctions(public=public, private=private, name=name,
			    type=type, descr=descr);

    private.state.name := '<node_name>';
    private.state.index := '<node_index>';
    public.label();

#===========================================================================
# Public interface:
#=========================================================================== 
# 
    public.name := function () {return private.state.name}
    public.index := function () {return private.state.index}

# Calls to the associated C++ nodes:

    public.getResult := function (request_id) {
	funcname := private._.onentry('.getResult()');
	rr := 'MeqResult';
	return private._.onexit(funcname, rr);
    }

    public.getState := function () {
	funcname := private._.onentry('.getState()');
	return private._.onexit(funcname, private.state);
    }

# The following functions may be overwritten by specific ones
# for more specific node objects like MeqParm etc:

    public.plot := function () {
	s := private.its.type;
	print s,': .plot(): to be implemented by overwriting this function';
	return T;
    }

#===========================================================================
# Private functions:
#===========================================================================


#===========================================================================
# Finished:
#===========================================================================

    return T;
}







############################################################################
# test-function:
############################################################################

test_MeqNode := function () {
    include 'inspect.g';
    print '\n\n****************************************************';
    print '** test_MeqNode:';

    # Create global object(s):
    print '\n** Create global objects';
    global mqs, mqn, mqe, mqs, mqp, mqt;
    if (T) {
	print '\n dummy mqs=\n',mqs := dummyMeqServer('mqs');
    } else {
	print '\n OMS mqs=\n',mqs := meqserver(verbose=4);
	print '\n** mqs.init() ->', mqs.init([=],[=],[=],wait=T);
    }
    print ' ';
    # mqs.inspect();

    # Implemented commands:
    print '\n', index := mqs.meq('Create.Node', [class='MEQNode', name='test'], wait_reply=T);
    print '\n', state := mqs.meq('Get.Node.State', [name='test'], wait_reply=T);
    print '\n', state := mqs.meq('Get.Node.State', [nodeindex=1], wait_reply=T);

    # Not yet implemented:
    # print '\n', mqs.meq('Set.Node.State', state, wait_reply=T);
    # print '\n', mqs.meq('Get.Node.Result', [name='test', request_id=1], wait_reply=T);

    # print '\n',mqn := MeqNode('mqn', server=mqs);

    # Inspect global object(s):
    # mqn.inspect();
    # mqp.inspect();
    # mqe.inspect();

    # mqn.agent -> test_event(mqn.type());
    # mqs.agent -> test_event(mqs.type());
    # print 'mqs.meq() ->',mqs.meq('<command>');

    # print 'mqn.defrec() ->',mqn.defrec();

    return T;
}
# Execute the test-function:
test_MeqNode();

############################################################################
